import sys
import os
from itertools import chain
from typing import List

from django.db import transaction, connection
from django.core.management.base import BaseCommand
from os import listdir
from os.path import isfile, isdir, join


class Command(BaseCommand):
    help = 'Создать элементы ddl-определений'

    @transaction.atomic
    def handle(self, *args, **options):
        basepath = join(os.path.dirname(__file__), '..', '..')

        functions_files = []

        def add_files_to_execute_array(_root: str, _functions_files: List[str]):
            for file in sorted(listdir(_root)):
                fullpath_of_item = join(_root, file)
                if isfile(fullpath_of_item):
                    if len(fullpath_of_item) and fullpath_of_item[0] != '_':
                        _functions_files.append(fullpath_of_item)
                elif isdir(fullpath_of_item):
                    add_files_to_execute_array(fullpath_of_item, _functions_files)

        add_files_to_execute_array(join(basepath, 'sql', 'functions'), functions_files)

        with connection.cursor() as cursor:
            for script_name in functions_files:
                print(script_name)
                with open(script_name, encoding='utf-8') as f:
                    s = f.read()
                    s = s.strip()
                    if s:
                        cursor.execute(s)
