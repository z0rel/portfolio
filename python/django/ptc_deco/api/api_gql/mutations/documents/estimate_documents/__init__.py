from graphene import Mutation, ID, String

from .mutate_generate_appendix_docx import mutate_generate_appendix_docx
from ....utils.auth.decorators import login_or_permissions_required


class GenerateAppendixDocx(Mutation):
    class Arguments:
        # appendix_code = String(description='Код приложения'),
        appendix_id = ID(description='Идентификатор приложения')

    content = String(description='base64-закодированное protobuf-сообщение с данными')

    @login_or_permissions_required(login_required=False, permissions=('api.generate_appendix_docx', ))
    def mutate(root, info, **input):
        return GenerateAppendixDocx(content=mutate_generate_appendix_docx(root, info, **input))
