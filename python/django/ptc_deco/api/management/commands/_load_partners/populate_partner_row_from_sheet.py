from dataclasses import dataclass
from typing import Optional


@dataclass
class PartnerStrRow:
    row_idx: int
    bin_number: str
    title: str
    partner_type: str
    client_type: str
    bank_recipient: str
    checking_account: str  # расчетный счет
    kbe: str
    linked_partner: str  # связанный контрагент (1 шт)
    brand: str
    postcode_title: str
    city_title: str
    address: str
    working_sector: str


def populate_partner_row_from_sheet(sheet_value, row_idx):
    sheet_value.set_row_idx(row_idx)

    return PartnerStrRow(
        row_idx=row_idx,
        bin_number=sheet_value.get('БИН'),
        title=sheet_value.get('Наименование контрагента'),
        partner_type=sheet_value.get('Тип контрагента'),
        client_type=sheet_value.get('Тип клиента'),
        bank_recipient=sheet_value.get('Банк'),
        checking_account=sheet_value.get('Расчетный счет'),
        kbe=sheet_value.get('Кбе'),
        linked_partner=prepare_linked_contragents(sheet_value.get('Связанные контрагенты')),
        brand=sheet_value.get('Бренд'),
        postcode_title=sheet_value.get('Почтовый индекс'),
        city_title=sheet_value.get_city('Город'),
        address=sheet_value.get('Адрес'),
        working_sector=sheet_value.get('Сектор деятельности'),
    )


LINKED_CONTRAGENTS_REPLACEMETNS = {
    'TOO Outcom': 'ТОО Outcom',
    'TOO Owl Media (ТОО Овл Медиа)': 'ТОО Owl Media (ТОО Овл Медиа)',
    'TOO ROI Group': 'ТОО ROI Group',
    'TOO Arena S': 'ТОО Arena S',
    'ТОО ХАМЛЕ КОМПАНИ ЛТД': 'ТОО Иностранное предприятие ХАМЛЕ КОМПАНИ ЛТД',
    'AO Банк ХоумКредит': 'АО Банк ХоумКредит',
    'TOO Capvia': 'ТОО Capvia',
    'TOO Magnum cash&carry': 'ТОО Magnum cash&carry',
    'ООО Форекс Клуб': 'ТОО Форекс Клуб',
    'АО КОКА КОЛА АЛМАТЫ БОТТЛЕРС': 'ТОО СП КОКА-КОЛА АЛМАТЫ БОТТЛЕРС',
}


def prepare_linked_contragents(name: Optional[str]):
    return LINKED_CONTRAGENTS_REPLACEMETNS.get(name, name)
