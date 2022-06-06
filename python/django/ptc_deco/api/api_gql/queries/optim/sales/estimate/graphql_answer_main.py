import django_filters
import graphene
from django.db import models
from graphene import Node
from graphene_django import DjangoObjectType
from rest_framework import serializers as rest_serializers
from rest_framework.renderers import JSONRenderer

from .graphql_answer_entities import (
    EstimateStaticAdditionalOthers,
    EstimateAdditionalCostsRts,
    EstimateAdditionalOthers,
    EstimateItogsNonRtsNoSides,
    EstimateReservationsNonRts,
    EstimateNoSidesNonRts,
)
from .graphql_resolvers import (
    resolve_reservations,
    resolve_additional_static_itogs,
    resolve_additional_costs_rts,
    resolve_additional_rts_itogs,
    resolve_itogs_additional_nonrts_nosides,
    resolve_additional_nonrts_reservations,
    resolve_estimate_no_sides_nonrts,
    resolve_address_programm,
)
from .......api.models.utils import named_meta
from .......api import models as m

from .db_query import get_estimate_obj_by_filter

class SingleStringModel(models.Model):
    Meta = named_meta('Таблица длинных строк', 'SingleStringModel', managed=False)

    content = models.TextField(help_text='длинная строка', null=True)


def optim_linear_dataset(node_cls, src_generator):
    # get_dataset = src_generator(node_cls)

    class ReservationSerializer(rest_serializers.ModelSerializer):
        # category_name = serializers.RelatedField(source='category', read_only=True)

        class Meta:
            model = m.ReservationCalculated
            fields = '__all__'
            depth = 1

    renderer = JSONRenderer()

    def f(self, info):
        serializer = ReservationSerializer(self.estimate_obj.reservations_rts, many=True)
        item = node_cls()
        item.content = renderer.render(serializer.data)
        # item.content = base64.b64encode(dataset.encode()).decode('ascii')
        item.pk = 1
        return [item]

    return f


def connect_model_node(model_cls, description):
    modelname = model_cls.__name__
    cls_node = type(
        f'V{modelname}Node',
        (DjangoObjectType,),
        {
            'Meta': type(
                f'V{modelname}Node.Meta',
                (),
                {
                    'model': model_cls,
                    'interfaces': (Node,),
                    'description': description,
                },
            )
        },
    )
    cls_connection = type(
        f'V{modelname}Connection',
        (graphene.Connection,),
        {
            'Meta': type(
                f'{modelname}Connection.Meta',
                (),
                {
                    'node': cls_node,
                },
            )
        },
    )
    return cls_node, graphene.relay.ConnectionField(cls_connection, description=description)


class EstimateCalculatedItogsQuerySet:
    def filter(self, **kwargs):
        estimate_obj = get_estimate_obj_by_filter(**kwargs)
        if estimate_obj is None:
            return []
        itog = estimate_obj.itog
        result = VEstimateCalculatedItogsApiNode()  # определен внизу файла
        estimate_obj.assign_to_dst_slots(itog, result, {'estimatecalculateditogsapi'})
        result.estimate_obj = estimate_obj
        return [result]


class EstimateCalculatedItogsManager(models.Manager):
    def get_queryset(self):
        return EstimateCalculatedItogsQuerySet()



