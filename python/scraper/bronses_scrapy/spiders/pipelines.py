# -*- coding: utf-8 -*-


from collections import OrderedDict
from pandas import DataFrame
import json
from .toscrape_css import pbar

class QuotesbotPipeline(object):
    def process_item(self, item, spider):
        return item

class JsonWriterPipeline(object):
    def open_spider(self, spider):
        self.file = open('items.jl', 'w')
    def close_spider(self, spider):
        self.file.close()

    def process_item(self, item, spider):
        line = json.dumps(dict(item)) + "\n"
        self.file.write(line)
        return item

class ExcelWriterPipeline(object):
    def open_spider(self, spider):
        self.data = OrderedDict()

    def close_spider(self, spider):
        df = DataFrame(data=self.data)
        pbar.close()
        df.to_excel('chuguni.xlsx')

    def process_item(self, item, spider):
        for x in item:
            if x in self.data:
                self.data[x].append(item[x])
            else:
                self.data[x] = [item[x]]
