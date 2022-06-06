#!/usr/bin/env python3
# coding=utf8

""" Pipeline module for upload items to excel xlsx """

import os
import pandas as pd  # require openpyxl


class WritePhonesToXlsx:
    """ Pipeline for upload items to excel """

    def __init__(self):
        """ Pipeline constructor """

        self.writers = {"phones": None, "comments": None, "vendors": None}
        self.counters = {"phones": 0, "comments": 0, "vendors": 0}
        self.prev_name = 'output.xlsx'
        self.next_name = 'output1.xlsx'
        self.saving_interval = 100000
        self.excel_writer = None

    def open_spider(self, spider):
        """ Initialize pipeline by open spider from spider settings """

        self.prev_name = spider.settings.get('XLSX_OUT_NAME', 'output') + '.xlsx'
        self.next_name = spider.settings.get('XLSX_OUT_NAME', 'output') + '1.xlsx'
        self.saving_interval = spider.settings.get('XLSX_OUT_CACHE_INTERVAL', 100000)
        self.init_excel_writer()

    def close_spider(self, spider):  # pylint: disable=unused-argument
        """ Save data after spider closing """

        self.save_to_excel()

    def init_excel_writer(self):
        """ Initialize excel writer """

        if os.path.exists(self.prev_name):
            os.remove(self.prev_name)
        self.excel_writer = pd.ExcelWriter(self.prev_name)

    def save_to_excel(self):
        """ Save excel sheets to excel workbook """

        for key in self.writers:
            if self.writers[key] is not None:
                self.writers[key].to_excel(self.excel_writer, key)
        self.excel_writer.save()

    def add_to_dataframe(self, writer, fields):
        """ Add new item row to pandas dataframe """

        if self.writers[writer] is None:
            self.writers[writer] = pd.DataFrame(columns=[key for (key, val) in fields])
        dataframe = self.writers[writer]
        vals = [val for (key, val) in fields]
        dataframe.loc[len(dataframe)] = vals

        self.counters[writer] += 1
        if not self.counters[writer] % self.saving_interval:
            self.save_to_excel()
            self.excel_writer.save()
            self.prev_name, self.next_name = self.next_name, self.prev_name
            self.init_excel_writer()

    def process_item(self, item, spider):  # pylint: disable=unused-argument
        """ Pipeline method: route item data to excel sheets """

        if "phone_info" in item:
            self.add_to_dataframe("phones", item["phone_info"])
        elif "user_post" in item:
            self.add_to_dataframe("comments", item["user_post"])
        elif "scrap_vendor_status" in item:
            self.add_to_dataframe("scrap_vendor_status", item["scrap_vendor_status"])
        elif "scrap_comments_status" in item:
            self.add_to_dataframe("scrap_comments_status", item["scrap_comments_status"])
        elif "vendors_info" in item:
            for vendor in item["vendors_info"]:
                self.add_to_dataframe("vendors", vendor)

        return item
