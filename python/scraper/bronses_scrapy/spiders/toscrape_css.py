# -*- coding: utf-8 -*-
import scrapy
from scrapy.selector import Selector
from pyquery import PyQuery as pq
from collections import OrderedDict
import re
from tqdm import tqdm
import traceback
import sys


START_URLS = OrderedDict([
        ('http://www.splav-kharkov.com/choose_type_class.php?type_id=3',  {"cat": "Сталь конструкционная"    , "tp": None}),
        ('http://www.splav-kharkov.com/choose_type_class.php?type_id=4',  {"cat": "Сталь инструментальная"   , "tp": None}),
        ('http://www.splav-kharkov.com/choose_type_class.php?type_id=6',  {"cat": "Сталь коррозионно-стойкая", "tp": None}),
        ('http://www.splav-kharkov.com/choose_type_class.php?type_id=8',  {"cat": "Сталь специальная"        , "tp": None}),
        ('http://www.splav-kharkov.com/choose_type_class.php?type_id=7',  {"cat": "Чугун"                    , "tp": None}),
        ("http://www.splav-kharkov.com/choose_type_class.php?type_id=9",  {"cat": "Бронза"                   , "tp": None}),
        ("http://www.splav-kharkov.com/choose_type_class.php?type_id=10", {"cat": "Латунь"                   , "tp": None}),
        ("http://splav-kharkov.com/mat_start.php?name_id=1132", {"cat": "Бронза", "tp": 1}), # БрА11Ж6Н6
        ("http://splav-kharkov.com/mat_start.php?name_id=1034", {"cat": "Бронза", "tp": 1}), # БрO10С10
        ("http://www.splav-kharkov.com/mat_start.php?name_id=1515", {"cat": "Бронза", "tp": 1}), 
        ('http://www.splav-kharkov.com/mat_start.php?name_id=1529', {"cat": "Латунь", "tp": 1}), 
])
exclude_url = set(["http://www.splav-kharkov.com/choose_type_class.php?type_id={0}#top".format(i) for i in range(0,20)] + 
[
    "http://www.splav-kharkov.com/about_program.php",
    ""
])

pbar = tqdm(total=len(START_URLS))
pbar_totals = len(START_URLS)

class ToScrapeCSSSpider(scrapy.Spider):
    name = "toscrape-css"
    start_urls = list(START_URLS.keys())

    def parse(self, response):
        try:
            global pbar_totals
            metadata = START_URLS[response.request.url]
            tp = metadata["tp"]
            cathegory = metadata["cat"]

            if tp:
                next_request = scrapy.Request(response.request.url, self.parse_int)
                next_request.meta['request_cathegory'] = cathegory
                yield next_request
                return

            pbar.update(1)
            pbar.refresh()

            data = response.css('center a::attr(href)').extract()
            pbar.total += len(data)
            pbar.refresh()
            for x in data:
                nexturl = response.urljoin(x)
                if nexturl in exclude_url:
                   pbar.update(1)
                   continue
                next_request = scrapy.Request(nexturl, self.parse_int)
                next_request.meta['request_cathegory'] = cathegory
                yield next_request

        except Exception as exc:
            print(exc, response.url)
            exc_info = sys.exc_info()
            traceback.print_exception(*exc_info)
    

    def parse_int(self, response):
        try:
            pbar.update(1)
            pbar.refresh()

            cathegory = response.meta['request_cathegory']

            name = pq(response.css('body > center > table  tr:nth-child(1) td:nth-child(2)').extract_first()).text()
            classify = pq(response.css('body > center > table  tr:nth-child(2) td:nth-child(2)').extract_first()).text()
            usage = pq(response.css('body > center > table  tr:nth-child(3) td:nth-child(2)').extract_first()).text()
            analogues = pq(response.css('body > center > table  tr:nth-child(4) td:nth-child(2)').extract_first()).text()
            x = response.css('body > center > table  tr:nth-child(5) td:nth-child(2)')
            dop = pq(x.extract_first()).text() if x else ""

            tren = response.xpath('//tr[contains(., "Коэффициент трения")]').getall()
            additions = OrderedDict([
                ("Коэффициент трения со смазкой :", None),
                ("Коэффициент трения без смазки :", None),
                ("Коэффициент трения в морской воде :", None)
            ])
            for x in tren:
                key = pq(x)('td').eq(0).text()
                if key not in additions:
                    print(key)
                additions[key] = pq(x)('td').eq(1).text()
            
            base = OrderedDict([
                ('cathegory', cathegory),
                ('mark', name), 
                ('name', None),
                ('value', None),
                ('classify', classify), 
                ('usage', usage), 
                ('analogues', analogues),
                ('dop', dop), 
                ('url', response.url)
            ])
            physical0 = response.xpath('//b[contains(., "Физические свойства материала")]/parent::*/parent::*/following-sibling::*')
            head = OrderedDict([
                ("C Дж/(кг·град)", "Удельная теплоемкость материала (диапазон 20o - T ), C [Дж/(кг·град)]"),
                ("T Град", "Температура, при которой получены данные свойства , T [Град]"),
                ("E 10- 5 МПа", "Модуль упругости первого рода , E 10-5 [МПа]"),
                ("R 10 9 Ом·м", "Удельное электросопротивление, R 10 9 Ом·м"),
                ("r кг/м3", "Плотность материала, rho [кг/м3]"),
                ("a\n10 6 1/Град", "Коэффициент температурного (линейного) расширения (диапазон 20o - T ) , alpha 10+6 [1/Град]"),
                ("l Вт/(м·град)", "Коэффициент теплопроводности (теплоемкость материала) , lambda [Вт/(м·град)]"),
            ])
            def make_properties(head, physical0):
                physical = []
                if not physical0:
                    physical = [{head[key]: '' for key in head}]
                    return physical

                if physical0:
                    table = physical0.extract_first()
                    vals = pq(table)('tr')
                    physical1 = pq(vals[1])('td')
                    physical0 = pq(vals[0])('td')
                    rows = OrderedDict([(x,[]) for x in head])

                    for i, td in enumerate(physical0):
                        name = pq(td).text().strip() + " " + physical1.eq(i).text().strip()
                        if name not in head or not head[name]:
                            print(name, response.url)
                            continue
                        for x in vals[2:]:
                            item = pq(x)('td').eq(i)
                            rows[name].append(item.text())

                    try:
                        physical = [{head[key]: rows[key][idx] for key in head} for idx in range(0, len(vals) - 2)]
                    except Exception as e:
                            print(e)
                return physical

            if response.url == "http://www.splav-kharkov.com/mat_start.php?name_id=1515":
                i = 111
            physical = make_properties(head, physical0)
            mechan_head = OrderedDict([
                ("Сортамент -", "Сортамент"),
                ("Размер мм", "Размер мм"),
                ("KCU кДж / м2", "Ударная вязкость), KCU [кДж/м2]"),
                ("s\nT МПа", "Предел пропорциональности (предел текучести для остаточной деформации), sigma T [МПа]"),
                ("s\nв МПа", "Предел кратковременной прочности), sigma в [МПа]"), 
                ("d\n5 %", "Относительное удлинение при разрыве , delta 5 [%]"),
                ("Напр. -", "Напр. -"),
                ("y %", "Относительное сужение , psi [%] "),
                ("Термообр. -", "Термообр. -")
            ])
            mech_fnd = OrderedDict([(mechan_head[key],None) for key in mechan_head])
            mechanical0 = response.xpath('//b[contains(., "Механические свойства при")]/parent::*/parent::*/following-sibling::*')
            mechan = make_properties(mechan_head, mechanical0)
            
            liteyn0 = response.xpath('//b[contains(., "Литейно-технологические свойства материала")]/parent::*/parent::*/following-sibling::*')
            liteyn_head = OrderedDict([
                ("Температура плавления :", None),
                ("Температура литья :", None),
                ("Линейная усадка :", None),
                ("Температура горячей обработки :", None),
                ("Температура отжига :", None),
            ])
            if liteyn0:
                liteyn_row = pq(liteyn0.extract()[0])('tr')
                for row in liteyn_row:
                    row = pq(row)('td')
                    lname = row.eq(0).text().strip()
                    if lname not in liteyn_head:
                        print(lname)
                    liteyn_head[lname] = row.eq(1).text().strip()

            chem_cond = response.xpath('//b[contains(., "Химический состав в")]/parent::*/parent::*/following-sibling::*/table').extract_first()
            components_set = ["Fe", "Ni", "Al", "Cu", "Pb", "Zn", "Sn", "Sb", "P", "As", "Si", "Mn", "Bi", "Примесей", "Ti", "Be", "O", "Ag", "Mg", "Cd", "-", "Zr", "Cr", "S", "Co", "Te", "Nb", "B", "C",
                              "Mo", "Se", "N", "W", "V", "Ta", "Ce", "Ca", "РЗМ"]
            components = OrderedDict([(x,None) for x in components_set])
            try:
                comp_table = pq(chem_cond)('tr')
                td_comp = pq(comp_table.eq(0))('td')
                if "Раньше данный сайт располагался" not in td_comp.text():
                    for i, x in enumerate(td_comp):
                        comp_head = pq(x).text()
                        comp_val = pq(pq(comp_table.eq(1))('td').eq(i)).text()
                        if comp_head not in components:
                            print(comp_head, comp_val)

                        components[comp_head] = comp_val

            except Exception as exc:
                print(exc)

            def gen_res(a, b, phys = {}, mechan = {}, liteyn_head = {}):
                res = OrderedDict()
                res.update(base)
                res.update({'name': a, 'твердость': b})
                res.update(phys)
                res.update(mechan)
                res.update(additions)
                res.update(liteyn_head)
                res.update(components)
                return res

            data = response.xpath('//tr[contains(., "Твердость")]').getall()
            founded_sortaments = set()
            for x in data:
                if "Твердость по Бринеллю" in x:
                    continue
                if x:
                    a = pq(x)('td').eq(0).text()
                    b = re.sub("HB 10 -1 = ", "", pq(x)('td').eq(1).text())
                else:
                    a = None
                    b = None

                for i, v in enumerate(mechan):
                    sortament = v['Сортамент']
                    if sortament in a:
                        mech_fnd = v
                        founded_sortaments.add(i)
                        break

                if not physical:
                    print("ERROR - PHYSICAL not defined")

                for x in physical:
                    yield gen_res(None,b,x,mech_fnd,liteyn_head)

            for i, mech in enumerate(mechan):
                if i not in founded_sortaments:
                    if not physical:
                        print("ERROR - PHYSICAL not defined")
                    for x in physical:
                        yield gen_res(None,None,x,mech,liteyn_head)

            if not data:
                if mechan:
                    print('mechan', response.url)
                if not physical:
                    print("ERROR - PHYSICAL not defined")
                for x in physical:
                    yield gen_res(None,None,x,mech_fnd,liteyn_head)

        except Exception as exc:
            print(exc, response.request.url)
            exc_info = sys.exc_info()
            traceback.print_exception(*exc_info)




        # for quote in response.css("div.quote"):
        #     yield {
        #         'text': quote.css("span.text::text").extract_first(),
        #         'author': quote.css("small.author::text").extract_first(),
        #         'tags': quote.css("div.tags > a.tag::text").extract()
        #     }

        # next_page_url = response.css("li.next > a::attr(href)").extract_first()
        # if next_page_url is not None:
        #     yield scrapy.Request(response.urljoin(next_page_url))
