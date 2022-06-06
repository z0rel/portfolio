import graphene as g
from ....api import models as m


class CountOfMountings(g.ObjectType):
    count_of_mountings = g.Int(description='Количество незавершенных монтажей')
    count_of_common_tasks = g.Int(description='Количество незавершенных общих задач')


def resolve_count_of_mountings(parent, info, **kwargs):
    count_of_mountings = m.Mounting.objects.filter(
        archived=False, mounting_done=False, mounting_task__isnull=True
    ).count()

    count_of_common_tasks = m.Mounting.objects.filter(
        archived=False, mounting_done=False, mounting_task__isnull=False
    ).count()

    return CountOfMountings(count_of_mountings=count_of_mountings, count_of_common_tasks=count_of_common_tasks)
