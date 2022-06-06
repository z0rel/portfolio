# import libraries
import cv2
import datetime

from django.db.models import F
from pyzbar import pyzbar
from django.utils.timezone import make_aware

from django.core.management.base import BaseCommand, CommandError
from commercial_scan import models as m


def read_barcodes(frame):
    barcodes = pyzbar.decode(frame)
    for barcode in barcodes:
        x, y, w, h = barcode.rect
        # 1
        barcode_info = barcode.data.decode("utf-8")
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

        # 2
        font = cv2.FONT_HERSHEY_DUPLEX
        cv2.putText(frame, barcode_info, (x + 6, y - 6), font, 2.0, (255, 255, 255), 1)
        # 3
        obj = m.CheckData.objects.filter(qr_data=barcode_info).annotate(inn=F('seller_inn'))
        dt = make_aware(datetime.datetime(year=2021, month=5, day=20))
        if obj:
            obj_val: m.CheckData = obj[0]
            print(
                "already created at: ",
                obj_val.created_date,
                barcode_info,
                "CHECK DATA:",
                obj_val.fns_json[:20].replace("\n", " ") if obj_val.fns_json is not None else None,
            )
            if obj_val.inn == '3666080389' and obj_val.check_date >= dt:
                print('=========== IS ENKOR OK ====================')

            if obj_val.fns_json is None:
                print("  WARNING: CHECK_FNS_DATA IS EMPTY")
        else:
            m.CheckData.objects.create(
                qr_data=barcode_info, created_date=make_aware(datetime.datetime.now())
            )
            print(datetime.datetime.now(), "OKAY: Recognized Barcode:" + barcode_info)

    return frame


class Command(BaseCommand):
    help = "Распознавать QR-коды и сохранять их в базу"

    def handle(self, *args, **options):
        camera = cv2.VideoCapture(0)
        print(camera)
        ret, frame = camera.read()
        while ret:
            ret, frame = camera.read()
            frame = read_barcodes(frame)
            cv2.imshow("Сканер чеков", frame)
            if cv2.waitKey(1) & 0xFF == 27:
                break
        # 3
        camera.release()
        cv2.destroyAllWindows()
