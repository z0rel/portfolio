from django.db.models import F

ORDERING_FIELDS = {
    'id': (['id'], 'По идентификатору стороны'),
    'advertising_side_code': (['advertising_side_code'], 'По коду рекламной стороны'),
    'side_code': (['side_code'], 'По коду стороны'),
    'format_code': (['format_code'], 'По коду формата конструкции'),
    'format_title': (['format_title'], 'По наименованию формата конструкции'),
    'marketing_address': (['marketing_address'], 'По маркетинговому адресу'),
    'status_connection': (['status_connection'], 'По статусу подключения - горит / не горит'),
    'num_in_district': (['num_in_district'], 'По номеру конструкции в районе'),
    'postcode_title': (['postcode_title'], 'По почтовому индексу'),
    'city_title': (['city_title'], 'По названию города'),
    'package_title': (['package_title'], 'По названию пакета'),
    'package_id': (['package_id'], 'По идентификатору пакета'),
    'district_title': (['district_title'], 'По названию района'),
    'advertising_side_title': (['advertising_side_title'], 'По названию рекламной стороны'),
    'side_size': (['side_size'], 'По размеру стороны'),
    'side_title': (['side_title'], 'По наименованию стороны'),
    'nonrts_owner_title': (['nonrts_owner_title'], 'По наименованию владельца'),
    'is_nonrts': (['is_nonrts'], 'По типу владельца - РТС либо НОН РТС'),
}
