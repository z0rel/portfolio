#!/usr/bin/env python3
# coding=utf8

import os
import locale
from gettext import gettext as _
import gettext

ENCODING = None

# python pygettext.py -d base -o lang/base.pot behrens_converter/transform.py

def __translate_standard_messages():
    """Эта функция нужна только для того, чтобы Poedit при сканировании
       добавлял соответствующие строки в .po-файл для их перевода.
       Строки скопированы из стандартного модуля Python argparse.py.
    """
    # argparse
    _('%(prog)s: error: %(message)s\n')
    _('expected one argument')
    _('invalid choice: %(value)r (choose from %(choices)s)')
    _('not allowed with argument %s')
    _('optional arguments')
    _('positional arguments')
    _('show this help message and exit')
    _('usage: ')


def locale_set_up(enc=None):
    global ENCODING
    langdir = os.path.join(os.path.dirname(__file__), 'lang')

    _locale, _encoding = locale.getdefaultlocale()  # Значения по умолчанию
    ENCODING = _encoding
    if enc:
        ENCODING = enc
    # Это заставит все стандартные модули Python использовать наш домен
    # и путь к файлам с переводом
    os.environ['LANGUAGE'] = _locale
    gettext.textdomain("behrens_converter")
    gettext.bindtextdomain("behrens_converter", langdir)
    # gettext.bind_textdomain_codeset("behrens_converter", ENCODING)
