import graphene
from graphene.relay import Node
from graphql import GraphQLError

from django.contrib.auth.models import Group, Permission
from django.contrib.auth.password_validation import validate_password
from django.core.exceptions import ValidationError

from ...models.users import EmployeePosition
from .auth import AuthMutation
from ptc_deco.api.api_gql.mutations.utils.user_utils import get_user_password_err_message

from ....api import models as m
from ....api.models.users import user_password_is_used
from .utils import create_custom_mutation_class, CudListNode

from .estimate_crud import EditNonRtsItems, DeleteEstimateItem, AddEstimateItemToAppendix
from .create_reservation import CreateOrUpdateReservation
from .mountings_crud import CreateMountingTasksForProject
from .documents.estimate_documents import GenerateAppendixDocx
from .download_packages_info import DownloadPackagesInfo
from .package_batch_cud.package_batch_add_construction_sides import BatchAddConstructionSidesToPackage
from .package_reservation import CreateReservationPackage, UpdatePackageReservationType
from .upload_packages import UploadPackages
from .download_mountings import DownloadMountingsInfo
from .download_estimate_report import DownloadEstimateReportInfo
from .download_advertising_sides_and_appendices import DownloadAdvertisingSidesAndAppendicesInfo
from .download_projects import DownloadProjectsInfo
from .download_contracts import DownloadConstractsInfo
from .download_brands import DownloadBrandsInfo
from .download_constructions import DownloadConstructionsInfo
from .download_partners import DownloadPartnersInfo
from .download_appendices import DownloadAppendicesInfo
from .download_advertising_sides import DownloadAdvertisingSidesInfo
from .download_reservations import DownloadReservationsInfo
from .download_m_projects_info import DownloadMProjectsInfo


def field(classname, description):
    return classname.Field(description=description)


def create_custom_mutation_class_by_crud_list():
    # TODO: есть еще DjangoFilterUpdateMutation и DjangoFilterDeleteMutation

    _ = None
    c = CudListNode
    attrs_list = [
        # users
        c(
            'user',
            m.CustomUser,
            permissions=('api.view_customuser',),
            create_permissions=('api.add_customuser',),
            update_permissions=('api.change_customuser',),
            delete_permissions=('api.delete_customuser',),
        ),
        c(
            'user_group',
            Group,
            permissions=('auth.view_group',),
            create_permissions=('auth.add_group',),
            update_permissions=('auth.change_group',),
            delete_permissions=('auth.delete_group',),
        ),
        c(
            'user_permission',
            Permission,
            permissions=('auth.view_permission',),
            create_permissions=('auth.add_permission',),
            update_permissions=('auth.change_permission',),
            delete_permissions=('auth.delete_permission',),
        ),
        c(
            'employee_position',
            EmployeePosition,
            permissions=('api.view_employeeposition',),
            create_permissions=('api.add_employeeposition',),
            update_permissions=('api.change_employeeposition',),
            delete_permissions=('api.delete_employeeposition',),
        ),
        # construction
        c(
            'format',
            m.Format,
            permissions=('api.view_format',),
            create_permissions=('api.add_format',),
            update_permissions=('api.change_format',),
            delete_permissions=('api.delete_format',),
        ),
        c(
            'model_construction',
            m.ModelConstruction,
            permissions=('api.view_modelconstruction',),
            create_permissions=('api.add_modelconstruction',),
            update_permissions=('api.change_modelconstruction',),
            delete_permissions=('api.delete_modelconstruction',),
        ),
        c(
            'under_family_construction',
            m.UnderFamilyConstruction,
            permissions=('api.view_underfamilyconstruction',),
            create_permissions=('api.add_underfamilyconstruction',),
            update_permissions=('api.change_underfamilyconstruction',),
            delete_permissions=('api.delete_underfamilyconstruction',),
        ),
        c(
            'family_construction',
            m.FamilyConstruction,
            permissions=('api.view_familyconstruction',),
            create_permissions=('api.add_familyconstruction',),
            update_permissions=('api.change_familyconstruction',),
            delete_permissions=('api.delete_familyconstruction',),
        ),
        c(
            'side',
            m.Side,
            permissions=('api.view_side',),
            create_permissions=('api.add_side',),
            update_permissions=('api.change_side',),
            delete_permissions=('api.delete_side',),
        ),
        c(
            'advertising_side',
            m.AdvertisingSide,
            permissions=('api.view_advertisingside',),
            create_permissions=('api.add_advertisingside',),
            update_permissions=('api.change_advertisingside',),
            delete_permissions=('api.delete_advertisingside',),
        ),
        c(
            'purpose_side',
            m.PurposeSide,
            permissions=('api.view_purposeside',),
            create_permissions=('api.add_purposeside',),
            update_permissions=('api.change_purposeside',),
            delete_permissions=('api.delete_purposeside',),
        ),
        c(
            'loc_address',
            m.Addresses,
            permissions=('api.view_addresses',),
            create_permissions=('api.add_addresses',),
            update_permissions=('api.change_addresses',),
            delete_permissions=('api.delete_addresses',),
        ),
        c(
            'obstruction',
            m.construction.Obstruction,
            permissions=('api.view_obstruction',),
            create_permissions=('api.add_obstruction',),
            update_permissions=('api.change_obstruction',),
            delete_permissions=('api.delete_obstruction',),
        ),
        c(
            'tech_problem',
            m.TechProblems,
            to_accs_all=True,
            permissions=('api.view_techproblems',),
            create_permissions=('api.add_techproblems',),
            update_permissions=('api.change_techproblems',),
            delete_permissions=('api.delete_techproblems',),
        ),
    ]

    def construction_before_mutate(cls, root, info, input, *args):
        if not isinstance(input, str) and getattr(input, 'keys'):  # is not delete mutation
            if 'create_marketing_address' in input.keys():
                creating_marketing_address = input.get('create_marketing_address', None)
                if input.get('marketing_address') is None and creating_marketing_address:
                    postcode_id = input.get('postcode', None)
                    if postcode_id:
                        postcode_id = Node.from_global_id(postcode_id)[1]
                    fnd_obj = m.Addresses.objects.filter(address=creating_marketing_address, postcode_id=postcode_id)
                    if fnd_obj:
                        input['marketing_address'] = Node.to_global_id('VAddressesNode', fnd_obj[0].id)
                    else:
                        obj = m.Addresses.objects.create(postcode_id=postcode_id, address=creating_marketing_address)
                        input['marketing_address'] = Node.to_global_id('VAddressesNode', obj.id)
            if 'create_obstruction' in input:
                creating_obstruction = input.get('create_obstruction', None)
                if input.get('obstruction') is None and creating_obstruction:
                    fnd_obj = m.Obstruction.objects.filter(title=creating_obstruction)
                    if fnd_obj:
                        input['obstruction'] = Node.to_global_id('VObstructionNode', fnd_obj[0].id)
                    else:
                        obj = m.Obstruction.objects.create(title=creating_obstruction)
                        input['obstruction'] = Node.to_global_id('VObstructionNode', obj.id)
            if 'create_tech_problem' in input:
                creating_tech_problem = input.get('create_tech_problem', None)
                if input.get('tech_problem') is None and creating_tech_problem:
                    tech_problem_comment = input.get('tech_problem_comment')
                    fnd_obj = m.TechProblems.objects.filter(title=creating_tech_problem).first()
                    construction_id = None
                    for argument in info.field_asts[0].arguments:
                        if argument.name.value == 'id':
                            construction_id = Node.from_global_id(argument.value.value)[1]
                            break
                    construction_obj = (
                        m.Construction.objects.filter(id=construction_id).first() if construction_id is not None else None
                    )
                    if fnd_obj:
                        if tech_problem_comment and fnd_obj.comment != tech_problem_comment:
                            fnd_obj.comment = tech_problem_comment
                            fnd_obj.save()
                        if construction_obj:
                            construction_obj.tech_problem.add(fnd_obj)
                        input['tech_problem'] = fnd_obj.id
                    else:
                        obj = m.TechProblems.objects.create(title=creating_tech_problem, comment=tech_problem_comment)
                        construction_obj.tech_problem.add(obj)
                        input['tech_problem'] = obj.id
        return input

    def user_validate(cls, root, info, input, *args):
        if isinstance(input, dict):
            password = input.get('password', None)
        else:
            password = None
        if password:
            try:
                validate_password(password)
            except ValidationError as error:
                raise GraphQLError(str(error))

    def user_before_mutate(cls, root, info, input, *args):
        user_id_user_pass = {}
        bad_user_password = {}

        if args and isinstance(input, dict):
            user_id = args[0]
            raw_password = input.get('password', None)
            if raw_password:
                user_id_user_pass[user_id] = raw_password
        elif isinstance(input, list) and len(input) > 0 and isinstance(input[0], dict):
            for param_set in input:
                user_id, raw_password = param_set.get('id', None), param_set.get('password', None)
                if user_id and raw_password:
                    user_id_user_pass[user_id] = raw_password

        for user_id, raw_password in user_id_user_pass.items():
            _type, _user_id = Node.from_global_id(user_id)
            user = m.CustomUser.objects.filter(id=_user_id).first()
            password_is_used, limit = user_password_is_used(user, raw_password)
            if password_is_used:
                bad_user_password[user_id] = get_user_password_err_message(limit, user_id)
        if bad_user_password:
            raise GraphQLError(str(bad_user_password.values()))

    attrs_list.extend(
        [
            c(
                'user',
                m.CustomUser,
                before_mutate=user_before_mutate,
                validate=user_validate,
            ),
            c(
                'construction',
                m.Construction,
                file_fields=['photo'],
                fk_fields=[],
                m2m_fields=[('tech_problem', 'TechProblems')],
                custom_fields={
                    'create_marketing_address': graphene.String(
                        description='Создать новый маркетинговый адрес с заданным (или незаданным) почтовым кодом postcode: ID'
                    ),
                    'create_obstruction': graphene.String(
                        description='Создать новую помеху с заданным названием: String'
                    ),
                    'create_tech_problem': graphene.String(
                        description='Создать новую техническую проблему с заданным названием и комментарием: String'
                    ),
                    'tech_problem_comment': graphene.String(description='Комментарий к технической проблеме: String'),
                },
                before_mutate=construction_before_mutate,
                permissions=('api.view_construction', ),
                create_permissions=('api.add_construction', ),
                update_permissions=('api.change_construction', ),
                delete_permissions=('api.delete_construction', ),
            ),
            c(
                'construction_side',
                m.ConstructionSide,
                permissions=('api.view_constructionside',),
                create_permissions=('api.add_constructionside',),
                update_permissions=('api.change_constructionside',),
                delete_permissions=('api.delete_constructionside',),
            ),
            # contract
            c(
                'contract_type',
                m.ContractType,
                permissions=('api.view_contracttype',),
                create_permissions=('api.add_contracttype',),
                update_permissions=('api.change_contracttype',),
                delete_permissions=('api.delete_contracttype',),
            ),
            c(
                'contract',
                m.Contract,
                file_fields=['contract_pdf'],
                fk_fields=[],
                m2m_fields=[],
                m2o_fields=[
                    ('contract_appendices', 'Appendix'),
                    ('invoices', 'Invoice'),
                ],
                permissions=('api.view_contract',),
                create_permissions=('api.add_contract',),
                update_permissions=('api.change_contract',),
                delete_permissions=('api.delete_contract',),
            ),
            c(
                'appendix',
                m.Appendix,
                file_fields=['additionally_agreement'],
                fk_fields=[],
                m2m_fields=[],
                permissions=('api.view_appendix',),
                create_permissions=('api.add_appendix',),
                update_permissions=('api.change_appendix',),
                delete_permissions=('api.delete_appendix',),
            ),
            # contragents
            c(
                'agency_commission',
                m.AgencyCommission,
                permissions=('api.view_agencycommission',),
                create_permissions=('api.add_agencycommission',),
                update_permissions=('api.change_agencycommission',),
                delete_permissions=('api.delete_agencycommission',),
            ),
            c(
                'working_sector',
                m.WorkingSector,
                permissions=('api.view_workingsector',),
                create_permissions=('api.add_workingsector',),
                update_permissions=('api.change_workingsector',),
                delete_permissions=('api.delete_workingsector',),
            ),
            c(
                'partner_type',
                m.PartnerType,
                permissions=('api.view_partnertype',),
                create_permissions=('api.add_partnertype',),
                update_permissions=('api.change_partnertype',),
                delete_permissions=('api.delete_partnertype',),
            ),
            c(
                'client_type',
                m.ClientType,
                permissions=('api.view_clienttype',),
                create_permissions=('api.add_clienttype',),
                update_permissions=('api.change_clienttype',),
                delete_permissions=('api.delete_clienttype',),
            ),
            c(
                'partner',
                m.Partner,
                file_fields=[],
                fk_fields=[
                    ('agency_commission', 'AgencyCommission'),
                ],
                m2m_fields=[
                    ('projects', 'Project'),
                    ('advertisers', 'Partner'),
                    ('brands', 'Brand'),
                    ('working_sectors', 'WorkingSector'),
                ],
                permissions=('api.view_partner',),
                create_permissions=('api.add_partner',),
                update_permissions=('api.change_partner',),
                delete_permissions=('api.delete_partner',),
            ),
            c(
                'contact_person',
                m.ContactPerson,
                permissions=('api.view_contactperson',),
                create_permissions=('api.add_contactperson',),
                update_permissions=('api.change_contactperson',),
                delete_permissions=('api.delete_contactperson',),
            ),
            # crews
            c(
                'crew',
                m.Crew,
                permissions=('api.view_crew',),
                create_permissions=('api.add_crew',),
                update_permissions=('api.change_crew',),
                delete_permissions=('api.delete_crew',),
            ),
            c(
                'mounting_task',
                m.MountingTask,
                batch=True,
                to_accs_all=True,
                permissions=('api.view_mountingtask',),
                create_permissions=('api.add_mountingtask',),
                update_permissions=('api.change_mountingtask',),
                delete_permissions=('api.delete_mountingtask',),
            ),
            c(
                'mounting',
                m.Mounting,
                file_fields=[],
                batch=True,
                permissions=('api.view_mounting',),
                create_permissions=('api.add_mounting',),
                update_permissions=('api.change_mounting',),
                delete_permissions=('api.delete_mounting',),
            ),
            c(
                'mounting_photo',
                m.MountingPhoto,
                file_fields=['photo'],
                permissions=('api.view_mountingphoto',),
                create_permissions=('api.add_mountingphoto',),
                update_permissions=('api.change_mountingphoto',),
                delete_permissions=('api.delete_mountingphoto',),
            ),
            # geolocation
            c(
                'city',
                m.City,
                permissions=('api.view_city',),
                create_permissions=('api.add_city',),
                update_permissions=('api.change_city',),
                delete_permissions=('api.delete_city',),
            ),
            c(
                'district',
                m.District,
                permissions=('api.view_district',),
                create_permissions=('api.add_district',),
                update_permissions=('api.change_district',),
                delete_permissions=('api.delete_district',),
            ),
            c(
                'postcode',
                m.Postcode,
                permissions=('api.view_postcode',),
                create_permissions=('api.add_postcode',),
                update_permissions=('api.change_postcode',),
                delete_permissions=('api.delete_postcode',),
            ),
            c(
                'country',
                m.Country,
                permissions=('api.view_country',),
                create_permissions=('api.add_country',),
                update_permissions=('api.change_country',),
                delete_permissions=('api.delete_country',),
            ),
            c(
                'location',
                m.Location,
                file_fields=['document'],
                fk_fields=[],
                m2m_fields=[],
                m2o_fields=[('constructions', 'Construction')],
                permissions=('api.view_location',),
                create_permissions=('api.add_location',),
                update_permissions=('api.change_location',),
                delete_permissions=('api.delete_location',),
            ),
            # project
            c(
                'brand',
                m.Brand,
                permissions=('api.view_brand',),
                create_permissions=('api.add_brand',),
                update_permissions=('api.change_brand',),
                delete_permissions=('api.delete_brand',),
            ),
            c(
                'project',
                m.Project,
                file_fields=[],
                fk_fields=[('agency_commission', 'AgencyCommission')],
                permissions=('api.view_project',),
                create_permissions=('api.add_project',),
                update_permissions=('api.change_project',),
                delete_permissions=('api.delete_project',),

            ),
            c(
                'reservation_type',
                m.ReservationType,
                permissions=('api.view_reservationtype',),
                create_permissions=('api.add_reservationtype',),
                update_permissions=('api.change_reservationtype',),
                delete_permissions=('api.delete_reservationtype',),
            ),
            c(
                'reservation',
                m.Reservation,
                permissions=('api.view_reservation',),
                create_permissions=('api.add_reservation',),
                update_permissions=('api.change_reservation',),
                delete_permissions=('api.delete_reservation',),
            ),
            # ['reservation_package', m.ReservationPackage],
            c(
                'design',
                m.Design,
                file_fields=['img'],
                permissions=('api.view_design',),
                create_permissions=('api.add_design',),
                update_permissions=('api.change_design',),
                delete_permissions=('api.delete_design',),
            ),
            c(
                'brand_image',
                m.BrandImage,
                file_fields=['img'],
                permissions=('api.view_brandimage',),
                create_permissions=('api.add_brandimage',),
                update_permissions=('api.change_brandimage',),
                delete_permissions=('api.delete_brandimage',),
            ),
            # sales
            c(
                'sales_additional_cost',
                m.AdditionalCosts,
                file_fields=[],
                fk_fields=[('agency_commission', 'AgencyCommission')],
                permissions=('api.view_additionalcosts',),
                create_permissions=('api.add_additionalcosts',),
                update_permissions=('api.change_additionalcosts',),
                delete_permissions=('api.delete_additionalcosts',),
            ),
            c(
                'sales_nonrts',
                m.EstimateNonRts,
                file_fields=[],
                fk_fields=[('agency_commission', 'AgencyCommission')],
                permissions=('api.view_estimatenonrts',),
                create_permissions=('api.add_estimatenonrts',),
                update_permissions=('api.change_estimatenonrts',),
                delete_permissions=('api.delete_estimatenonrts',),
            ),
            c(
                'sales_invoice',
                m.Invoice,
                file_fields=[],
                fk_fields=[],
                permissions=('api.view_invoice',),
                create_permissions=('api.add_invoice',),
                update_permissions=('api.change_invoice',),
                delete_permissions=('api.delete_invoice',),
            ),
            c(
                'sales_placement_price',
                m.PlacementPrice,
                permissions=('api.view_placementprice',),
                create_permissions=('api.add_placementpric',),
                update_permissions=('api.change_placementpric',),
                delete_permissions=('api.delete_placementpric',),
            ),
            c(
                'advert_promo_company',
                m.AdvertPromoCompany,
                permissions=('api.view_advertpromocompany',),
                create_permissions=('api.add_advertpromocompany',),
                update_permissions=('api.change_advertpromocompany',),
                delete_permissions=('api.delete_advertpromocompany',),
            ),
            c(
                'notification',
                m.Notification,
                permissions=('api.view_notification',),
                create_permissions=('api.add_notification',),
                update_permissions=('api.change_notification',),
                delete_permissions=('api.delete_notification',),
            ),
            c(
                'static_additional_costs',
                m.StaticAdditionalCosts,
                permissions=('api.view_staticadditionalcosts',),
                create_permissions=('api.add_staticadditionalcosts',),
                update_permissions=('api.change_staticadditionalcosts',),
                delete_permissions=('api.delete_staticadditionalcosts',),
            ),
            c(
                'construction_notification',
                m.ConstructionNotification,
                permissions=('api.view_constructionnotification',),
                create_permissions=('api.add_constructionnotification',),
                update_permissions=('api.change_constructionnotification',),
                delete_permissions=('api.delete_constructionnotification',),
            ),
        ]
    )

    attrs = create_custom_mutation_class(attrs_list)

    attrs['create_mounting_tasks_for_project'] = field(
        CreateMountingTasksForProject, 'Создать монтажные задачи для всего проекта'
    )
    attrs['edit_estimate_item'] = field(EditNonRtsItems, 'Изменить значения полей строк сметы')
    attrs['delete_estimate_item'] = field(DeleteEstimateItem, 'Удалить элемент сметы проекта или приложения')
    attrs['add_estimate_item_to_appendix'] = field(
        AddEstimateItemToAppendix, 'Добавить строки из сметы проекта в смету приложения'
    )

    attrs['create_or_update_reservation'] = field(
        CreateOrUpdateReservation, 'Добавить или обновить список бронирований'
    )
    attrs['generate_appendix_docx'] = field(GenerateAppendixDocx, 'Сгенерировать docx приложения к Договору')
    attrs['download_packages_info'] = field(DownloadPackagesInfo, 'Загрузить информацию по пакетам')
    attrs['batch_add_construction_sides_to_package'] = field(
        BatchAddConstructionSidesToPackage, 'Добавить стороны конструкций в пакет'
    )
    attrs['create_package_reservation'] = field(CreateReservationPackage, 'Забронировать пакет')
    attrs['update_package_reservation_type'] = field(UpdatePackageReservationType, 'Изменить тип брони пакета')
    attrs['upload_packages'] = field(UploadPackages, 'Выгрузить информацию о пакетном размещении')
    attrs['download_mountings_info'] = field(DownloadMountingsInfo, 'Выгрузить информацию о монтажах')
    attrs['download_estimate_report_info'] = field(DownloadEstimateReportInfo, 'Загрузить смету')
    attrs['download_advertising_sides_and_appendices'] = field(
        DownloadAdvertisingSidesAndAppendicesInfo,
        'Выгрузить информацию о рекламных сторонах и приложениях',
    )
    attrs['download_projects'] = field(DownloadProjectsInfo, 'Загрузить информацию о проектах')
    attrs['download_contracts'] = field(DownloadConstractsInfo, 'Загрузить информацию о договорах')
    attrs['download_brands'] = field(DownloadBrandsInfo, 'Загрузить информацию о брендах')
    attrs['download_constructions'] = field(DownloadConstructionsInfo, 'Загрузить информацию о конструкциях')
    attrs['download_partners'] = field(DownloadPartnersInfo, 'Загрузить информацию о контрагентах')
    attrs['download_advertising_sides'] = field(
        DownloadAdvertisingSidesInfo,
        'Загрузить информацию о рекламных сторонах'
    )
    attrs['download_appendices'] = field(DownloadAppendicesInfo, 'Загрузить информацию о приложениях')
    attrs['download_reservations'] = field(DownloadReservationsInfo, 'Загрузить информацию о забронированных сторонах')
    attrs['download_m_projects_info'] = field(DownloadMProjectsInfo, 'Загрузить информацию о подаче разнарядки')

    return type('CustomMutation', (graphene.ObjectType,), attrs)


CustomMutation = create_custom_mutation_class_by_crud_list()
