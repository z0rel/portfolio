from typing import Union

from django.db.models import F

from .......api import models as m
from ......estimate import EstimateCalc


class AnnotationAppendixQuery:
    project_client_bik: type(m.Partner.bik)
    project_client_bin_number: type(m.Partner.bin_number)
    contract_code: type(m.Contract.code)
    contract_registration_date: type(m.Contract.registration_date)
    contract_serial_number: type(m.Contract.serial_number)
    contract_payment_date: type(m.Contract.payment_date)
    client_id: type(m.Project.client_id)


def appendix_query(**filter_args) -> Union[m.Appendix, AnnotationAppendixQuery]:
    return (
        m.Appendix.objects.filter(**filter_args)
        .select_related(
            'project',
            'project__client',
            'project__agency_commission',
            'project__client__agency_commission'
        )
        .only(
            'id',
            'code',
            'project__code',
            'project__client__title',
            'project__client',
            'project__client_id',
            'project__agency_commission',
            'project__client__agency_commission',
            'project__discount_nalog_percent',
            'project__discount_client_percent',
            'project__discount_price_percent',
            'project__client__discount_nalog_percent',
            'project__client__discount_client_percent',
            'project__client__discount_price_percent',
            'project__client__bik',
            'project__client__bin_number',
            'project__brand__title',
            'contract__code',
            'contract__registration_date',
            'contract__payment_date',
            'contract__signatory_position',
            'contract__signatory_two',
            'contract__serial_number',
            'contract__based_on_document',
            'signatory_position',
            'payment_date',
            'contract__payment_date',
            'created_date'
        )
        .annotate(
            project_client_bik=F('project__client__bik'),
            project_client_bin_number=F('project__client__bin_number'),
            contract_code=F('contract__code'),
            contract_registration_date=F('contract__registration_date'),
            contract_serial_number=F('contract__serial_number'),
            contract_payment_date=F('contract__payment_date'),
            client_id=F('project__client_id'),
            client_title=F('project__client__title'),
            contract_signatory_position=F('contract__signatory_position'),
            contract_signatory_two=F('contract__signatory_two'),
            contract_based_on_document=F('contract__based_on_document'),
            project_brand_title=F('project__brand__title'),
            project_code=F('project__code'),
        )
    )


def project_query(**filter_args):
    return (
        m.Project.objects.filter(**filter_args)
        .select_related('client', 'agency_commission', 'client__agency_commission')
        .only(
            'id',
            'code',
            'client',
            'agency_commission',
            'client__agency_commission',
            'discount_nalog_percent',
            'discount_client_percent',
            'discount_price_percent',
            'client__discount_nalog_percent',
            'client__discount_client_percent',
            'client__discount_price_percent',
        )
    )


def get_estimate_obj_by_filter_args(**kwargs):
    project_code = kwargs.pop('project__code__exact') if 'project__code__exact' in kwargs else None
    appendix_code = kwargs.pop('appendix__code__exact') if 'appendix__code__exact' in kwargs else None
    project_id = kwargs.pop('project__id__exact') if 'project__id__exact' in kwargs else None
    appendix_id = kwargs.pop('appendix__id__exact') if 'appendix__id__exact' in kwargs else None

    filter_arg_appendix = {}
    if appendix_id is not None:
        filter_arg_appendix['id'] = appendix_id
    if appendix_code is not None:
        filter_arg_appendix['code'] = appendix_code

    if filter_arg_appendix:
        q = appendix_query(**filter_arg_appendix)
        if not q:
            return None
        appendix_obj = q[0]
        project_obj = appendix_obj.project
        return (
            {'project_obj': project_obj, 'appendix_obj': appendix_obj},
            {'appendix': appendix_obj}
        )
    else:
        filter_arg_project = {}
        if project_code is not None:
            filter_arg_project['code'] = project_code
        if project_id is not None:
            filter_arg_project['id'] = project_id

        if not filter_arg_project:
            return None

        q = project_query(**filter_arg_project)
        if not q:
            return None
        project_obj = q[0]

        return (
            {'project_obj': project_obj, 'appendix_obj': None},
            {'project__id': project_obj.id}
        )


def get_estimate_obj_by_filter(**kwargs):
    constructor_arg, calc_arg = get_estimate_obj_by_filter_args(**kwargs)
    return EstimateCalc(**constructor_arg).full_calc(**calc_arg)



