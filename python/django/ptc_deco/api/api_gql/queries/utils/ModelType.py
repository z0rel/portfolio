from typing import Union, TypeVar, Generic, Iterator
from django.db.models import QuerySet

T = TypeVar('T')


class ModelType(Generic[T]):
    def __iter__(self) -> Iterator[Union[T, QuerySet]]:
        pass
