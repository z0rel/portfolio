#!/usr/bin/env python3
# coding=utf8

""" The scrapers collection """

# to create whl: python setup.py bdist_wheel
# to install: pip install scrap.whl
# if console show "invalid command 'bdist_wheel'"
# try "pip install wheel"

from setuptools import setup

setup(
    name='scraperlib',
    version='0.0.2',
    description='The scrapers collection',
    packages=['scraperlib'],  # same as name
    install_requires=['scrapy', 'mysql-connector', 'mysql', 'pandas', 'openpyxl'],
)
