from graphene_django_cud.mutations import (
    DjangoCreateMutation,
    DjangoBatchCreateMutation,
    DjangoBatchUpdateMutation,
    DjangoBatchDeleteMutation,
    DjangoPatchMutation,
    DjangoBatchPatchMutation,
    DjangoUpdateMutation,
    DjangoDeleteMutation,
)

from .meta_new_or_use_mutation import meta_new_or_use_mutation, CudListNode, NameOfMutationFlags


def create_custom_mutation_class(crud_list):
    attrs = {}

    item: CudListNode
    for item in crud_list:
        if not item.nobatch:
            meta_new_or_use_mutation(
                attrs=attrs,
                crud_prefix=NameOfMutationFlags.CREEATE_BATCH,
                django_mutation_class=DjangoBatchCreateMutation,
                settings=item
            )
        meta_new_or_use_mutation(
            attrs=attrs,
            crud_prefix=NameOfMutationFlags.CREATE,
            django_mutation_class=DjangoCreateMutation,
            settings=item
        )
        meta_new_or_use_mutation(
            attrs=attrs,
            crud_prefix=NameOfMutationFlags.UPDATE,
            django_mutation_class=DjangoUpdateMutation,
            settings=item
        )
        if not item.nopatch:
            meta_new_or_use_mutation(
                attrs=attrs,
                crud_prefix=NameOfMutationFlags.UPDATE_PATCH,
                django_mutation_class=DjangoPatchMutation,
                settings=item
            )
        if not item.nobatch:
            meta_new_or_use_mutation(
                attrs=attrs,
                crud_prefix=NameOfMutationFlags.UPDATE_BATCH,
                django_mutation_class=DjangoBatchUpdateMutation,
                settings=item
            )
        if not item.nopatch:
            meta_new_or_use_mutation(
                attrs=attrs,
                crud_prefix=NameOfMutationFlags.UPDATE_BATCH_PATCH,
                django_mutation_class=DjangoBatchPatchMutation,
                settings=item
            )
        meta_new_or_use_mutation(
            attrs=attrs,
            crud_prefix=NameOfMutationFlags.DELETE,
            django_mutation_class=DjangoDeleteMutation,
            settings=item
        )
        meta_new_or_use_mutation(
            attrs=attrs,
            crud_prefix=NameOfMutationFlags.DELETE_BATCH,
            django_mutation_class=DjangoBatchDeleteMutation,
            settings=item
        )
    return attrs
