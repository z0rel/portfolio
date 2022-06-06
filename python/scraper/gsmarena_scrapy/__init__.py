#!/usr/bin/env python3
# coding=utf8

""" The gsmarena scrapping module """

from .scrap_gsmarena import crawl_gsmarena
from .scrap_gsmarena import multiprocess_crawl_comments
from .scrap_gsmarena import gsmarena_restore_parents
from .pipeline_sql import upload_data_to_excel
from .pipeline_sql import get_handled_sources
