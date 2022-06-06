import os
import json
from typing import Optional

import requests
import datetime

from django.db.models import Q
from dotenv import load_dotenv

from django.core.management.base import BaseCommand, CommandError
from django.utils.timezone import make_aware

from commercial_scan import models as m
from commercial_scan.management.commands.qr_parse import read_yaml


class NalogRuPython:
    HOST = "irkkt-mobile.nalog.ru:8888"
    DEVICE_OS = "iOS"
    CLIENT_VERSION = "2.9.0"
    DEVICE_ID = "7C82010F-16CC-446B-8F66-FC4080C66521"
    ACCEPT = "*/*"
    USER_AGENT = "billchecker/2.9.0 (iPhone; iOS 13.6; Scale/2.00)"
    ACCEPT_LANGUAGE = "ru-RU;q=1, en-US;q=0.9"

    def __init__(self):
        load_dotenv()
        self.__session_id = None
        self.set_session_id()

    def set_session_id(self) -> None:
        if os.getenv("CLIENT_SECRET") is None:
            raise ValueError('OS environments not content "CLIENT_SECRET"')
        if os.getenv("INN") is None:
            raise ValueError('OS environments not content "INN"')
        if os.getenv("PASSWORD") is None:
            raise ValueError('OS environments not content "PASSWORD"')

        url = f"https://{self.HOST}/v2/mobile/users/lkfl/auth"
        payload = {
            "inn": os.getenv("INN"),
            "client_secret": os.getenv("CLIENT_SECRET"),
            "password": os.getenv("PASSWORD"),
        }
        headers = {
            "Host": self.HOST,
            "Accept": self.ACCEPT,
            "Device-OS": self.DEVICE_OS,
            "Device-Id": self.DEVICE_ID,
            "clientVersion": self.CLIENT_VERSION,
            "Accept-Language": self.ACCEPT_LANGUAGE,
            "User-Agent": self.USER_AGENT,
        }

        resp = requests.post(url, json=payload, headers=headers)
        self.__session_id = resp.json()["sessionId"]

    def _get_ticket_id(self, qr: str) -> Optional[str]:
        url = f"https://{self.HOST}/v2/ticket"
        payload = {"qr": qr}
        headers = {
            "Host": self.HOST,
            "Accept": self.ACCEPT,
            "Device-OS": self.DEVICE_OS,
            "Device-Id": self.DEVICE_ID,
            "clientVersion": self.CLIENT_VERSION,
            "Accept-Language": self.ACCEPT_LANGUAGE,
            "sessionId": self.__session_id,
            "User-Agent": self.USER_AGENT,
        }
        resp = requests.post(url, json=payload, headers=headers)
        tx = resp.text
        if tx == 'Unknown QR code format':
            print(f'Error for qr: {qr}: {tx}')
            return None
        if resp.text == 'Превышен лимит запросов за сутки':
            print('Превышен лимит запросов за сутки')
            return None
        return resp.json().get("id", None) if resp is not None else None

    def get_ticket(self, qr: str) -> Optional[dict]:
        ticket_id = self._get_ticket_id(qr)
        if ticket_id is None:
            return None
        url = f"https://{self.HOST}/v2/tickets/{ticket_id}"
        headers = {
            "Host": self.HOST,
            "sessionId": self.__session_id,
            "Device-OS": self.DEVICE_OS,
            "clientVersion": self.CLIENT_VERSION,
            "Device-Id": self.DEVICE_ID,
            "Accept": self.ACCEPT,
            "User-Agent": self.USER_AGENT,
            "Accept-Language": self.ACCEPT_LANGUAGE,
        }
        resp = requests.get(url, headers=headers)
        return resp.json()


class Command(BaseCommand):
    help = "Распознавать QR-коды через ФНС API"

    def handle(self, *args, **options):
        client = NalogRuPython()
        yaml_data = read_yaml('itemkeys_to_get_from_fns.yml')
        items_by_keys = set([item['item']['key'] for item in yaml_data if 'is_not_avance' not in item['item']
                             ]) if yaml_data else set()
        # при необходимости удалить ранее вставленные - раскомментировать
        #z = ['t=20210721T1737&s=6581.00&fn=9287440300492385&i=34071&fp=4259612753&n=1']
        #m.CheckData.objects.filter(qr_data__in=z).delete()
        filtered_keys = [x.qr_data for x in m.CheckData.objects.filter(qr_data__in=items_by_keys).only('qr_data')]
        inserted_keys = items_by_keys - set(filtered_keys)

        items_by_keys_is_not_avance = set([item['item']['key'] for item in yaml_data if 'is_not_avance' in item['item']
                                          ]) if yaml_data else set()

        if inserted_keys:
            print(f'inserting {len(inserted_keys)} qr keys...')
        for k in inserted_keys:
            m.CheckData.objects.create(
                qr_data=k, created_date=make_aware(datetime.datetime.now())
            )
        for k in items_by_keys_is_not_avance:
            print(f'inserting is not avance key: {k}...')
            m.CheckData.objects.create(
                qr_data=k, created_date=make_aware(datetime.datetime.now()), is_not_avance=True
            )

        for obj in m.CheckData.objects.filter(Q(is_not_avance=None) | Q(is_not_avance=False), fns_json=None):
            if obj.qr_data[:2] == 't=':
                print(obj.qr_data)
                try:
                    ticket = client.get_ticket(obj.qr_data)
                except Exception as e:
                    print('Error: ', str(e))
                    # m.CheckData.objects.filter(qr_data__in=[obj.qr_data]).delete()
                    continue

                if ticket is not None:
                    obj.fns_json = json.dumps(ticket, indent=4)
                    #print(obj.fns_json)
                    obj.fns_query_date = make_aware(datetime.datetime.now())
                    obj.save(update_fields=['fns_json', 'fns_query_date'])
                else:
                    m.CheckData.objects.filter(qr_data__in=[obj.qr_data]).delete()

