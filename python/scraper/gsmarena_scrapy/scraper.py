#!/usr/bin/env python3
# coding=utf8

import logging
import scrapy
import csv
import random
import urllib
import re

import datetime 
from scrapy.crawler import CrawlerProcess
from scrapy.utils.log import configure_logging
from pipeline_sql import get_handled_sources

class StaticData:
    def __init__(self):
        self.handled_fields = set([
            'released-hl', 'body-hl', 'os-hl', 'storage-hl', 'modelname',
            'displaysize-hl', 'displayres-hl', 'camerapixels-hl', 'videopixels-hl',
            'ramsize-hl', 'chipset-hl', 'batsize-hl', 'battype-hl', 'comment',
            'nettech', 'edge', 'gprstext', 'net2g', 'net3g', 'net4g', 'nfc', 'speed', 'year',
            'status', 'dimensions', 'weight', 'sim', 'bodyother', 'build', 'displaytype',
            'displaysize', 'displayresolution', 'displayprotection', 'displayother', 'os', 'chipset', 'cpu', 'gpu',
            'memoryslot', 'internalmemory', 'memoryother', 'cam1modules', 'cam1features',
            'cam1video', 'cam2modules', 'cam2features', 'cam2video', 'optionalother', 'wlan',
            'bluetooth', 'gps', 'radio', 'usb', 'sensors', 'featuresother', 'batdescription1',
            'battalktime1', "batstandby1", 'colors', 'price', "batlife", "batmusicplayback1", "tbench", 'sar-eu', 'sar-us'
        ])
        
        self.handled_test_fields = set([
            'Performance', 'Display', 'Camera', 'Loudspeaker', 'Audio quality', 'Battery life'
        ])

        self.integer_re = re.compile(r'\d+')
        self.hours_ago = re.compile(r"(\d+) days? ago")
        self.hours_ago = re.compile(r"(\d+) hours? ago")
        self.minutes_ago = re.compile(r"(\d+) minute?s? ago")
        self.seconds_ago = re.compile(r"(\d+) seconds? ago")


class ScraperGsmarenaMixin(object):
    metadata = StaticData()
    
    def make_scrapy_request(self, url, callback, *args, **kwargs):
        parsed_url = urllib.parse.urlparse(url)
        request = scrapy.Request(url, callback, *args, **kwargs)
        proxy = None
        if parsed_url.scheme == 'https' and 'USER_HTTPS_PROXIES' in self.settings:
           proxy = self.settings['USER_HTTPS_PROXIES']
        elif parsed_url.scheme == 'http' and 'USER_HTTP_PROXIES' in self.settings:
           proxy = self.settings['USER_HTTP_PROXIES']

        if proxy and isinstance(proxy, list) and len(proxy) > 0: 
            request.meta['proxy'] = random.choice(proxy)
        return request

    def start_requests(self):
        request = self.make_scrapy_request(url='https://www.gsmarena.com/makers.php3', callback=self.parse_main)
        yield request

    def make_phone_group_request(self, response, vendor, group_url):
        next_page = response.urljoin(group_url)
        request_phone_page = self.make_scrapy_request(url=next_page, callback=self.parse_phones_page)
        request_phone_page.meta['vendor'] = vendor
        return request_phone_page

    def set_single_phone_metadata(self, request, vendor, phone_brief, phone_title, phone_url):
        request.meta['vendor'] = vendor
        request.meta['phone_brief'] = phone_brief
        request.meta['phone_title'] = phone_title
        request.meta['phone_url'] = phone_url 

    def parse_main(self, response):
        phones_group = response.css("div#body div.st-text td a")

        vendors = []
        
        for group in phones_group:
            vendor = group.css("::text").extract_first()
            group_url = group.css("::attr(href)").extract_first()
            devices_count_text = group.css("span::text").extract_first()
            cnt_list = self.metadata.integer_re.findall(devices_count_text)
            devices_count = int(cnt_list[0]) if len(cnt_list) > 0 else 0
            vendors.append([
                ('vendor_name', vendor),
                ('devices_cnt', devices_count),
                ('group_url', response.urljoin(group_url)) 
            ])
        yield {"vendors_info": vendors}

        handled_vendors = self.settings['SKIP_HANDLED_SOURCES'].get("vendors", {})

        for vendor in vendors:
            vendor_name = vendor[0][1]
            vendor_url = vendor[2][1] 
            next_url = response.urljoin(vendor_url)
            if next_url not in handled_vendors:
                yield self.make_phone_group_request(response, vendor_name, vendor_url)
            

    def parse_phones_page(self, response):
        vendor = response.meta['vendor']
        paginators = response.css("div#body div.main")
        phones_list = paginators.css("div#review-body div.makers > ul > li")
        handled_phones = self.settings['SKIP_HANDLED_SOURCES'].get("phones", {})

        for phone in phones_list:
            phone_url = response.urljoin(phone.css("a::attr(href)").extract_first())
            if phone_url not in handled_phones:
                phone_img = phone.css("a img")
                phone_brief = phone_img.css("::attr(title)").extract_first()
                phone_title = phone.css("a strong span::text").extract_first()
                request_phone_page = self.make_scrapy_request(url=phone_url, callback=self.parse_phone_page)
                self.set_single_phone_metadata(request_phone_page, vendor, phone_brief, phone_title, phone_url)
                yield request_phone_page

        next_page = paginators.css("a.pages-next")
        nextpage_buttouns_group = next_page.css("::attr(class)").extract_first()

        if nextpage_buttouns_group:
            nextpage_class_list = nextpage_buttouns_group.split(" ")
            if 'disabled' not in nextpage_class_list:
                next_page_url = next_page.css("::attr(href)").extract_first()
                yield self.make_phone_group_request(response, vendor, next_page_url)
        else:
            service_data = { "scrap_vendor_status": [ ("vendor_name", vendor), ("scrapped", True) ]}
            yield service_data

    def parse_phone_page(self, response):
        vendor = response.meta['vendor']
        phone_brief = response.meta['phone_brief'] 
        phone_title = response.meta['phone_title'] 

        body = response.css('div.main.main-review')
        handled_comments = self.settings['SKIP_HANDLED_SOURCES'].get("comments", {})

        def cssvalue(selector):
            return body.css('*[data-spec="' + selector + '"]::text').extract_first()

        def cssvalue_join(selector):
            return " <br/> ".join([(s.replace("\n", " ").replace("\r", " ") if s else '') for s in body.css('*[data-spec="' + selector + '"]::text').extract()])

        def cssvalue_sib(selector):
            sib1 = body.css('span[data-spec="' + selector + '"]::text').extract_first()
            sib2 = body.css('span[data-spec="' + selector + '"] + span::text').extract_first()
            return (str(sib1) if sib1 else '') + (str(sib2) if sib2 else '')

        tests = body.xpath(".//tr/th[contains(./text(), 'Tests')]/parent::*/parent::*")
        test_fields = set(tests.css('td.ttl a::text').extract())

        unhandled_fields = list(sorted(set(response.css("*[data-spec]::attr(data-spec)").extract()) - self.metadata.handled_fields))
        unhandled_test_fields = list(sorted(test_fields - self.metadata.handled_test_fields))
        main_img_url = body.css('div.specs-photo-main a::attr(href)').extract_first()
        hits_val = body.css('li.help-popularity span::text').extract_first()
        hits_val = int(self.metadata.integer_re.findall(hits_val.replace(',', ''))[0]) if hits_val else None 

        data = { "phone_info": [
            ("vendor_name", vendor),  
            ("phone_title", phone_title),
            ("phone_brief", phone_brief),
            ("phone_url", response.url),
            ("fullname", body.css('h1.specs-phone-name-title::text').extract_first()),
            ("main_img_url", (response.urljoin(main_img_url) if main_img_url else None)),
            ("become_a_fan", body.css('a.specs-fans strong::text').extract_first()),
            ("help_popularity", hits_val),
            ("released_date", cssvalue("released-hl")),
            ("body_hl", cssvalue('body-hl')),
            ("os_hl", cssvalue('os-hl')),
            ("storage", cssvalue('storage-hl')),
            ("modelname", cssvalue('modelname')),
            ("displaysize", cssvalue('displaysize-hl')),
            ("display_res", cssvalue('displayres-hl')),
            ("camerapixels", cssvalue_sib('camerapixels-hl')),
            ("videopixels", cssvalue('videopixels-hl')),
            ("ramsize_gb", cssvalue_sib('ramsize-hl')),
            ("chipset", cssvalue('chipset-hl')),
            ("batsize", cssvalue_sib('batsize-hl')),
            ("battype", cssvalue('battype-hl')),
            ("comment", cssvalue('comment')),
            ("net_tech", cssvalue('nettech')),
            ("net_edge", cssvalue('edge')),
            ("net_gprs", cssvalue('gprstext')),
            ("net_2g", cssvalue('net2g')),
            ("net_3g", cssvalue('net3g')),
            ("net_4g", cssvalue('net4g')),
            ("net_speed", cssvalue('speed')),
            ("net_nfc", cssvalue('nfc')),
            ("announced", cssvalue('year')),
            ("phone_status", cssvalue('status')),
            ("body_dimensions", cssvalue('dimensions')),
            ("body_weight", cssvalue('weight')),
            ("body_sim", cssvalue('sim')),
            ("body_other", cssvalue('bodyother')),
            ("body_build", cssvalue('build')),
            ("display_type", cssvalue('displaytype')),
            ("display_size", cssvalue('displaysize')),
            ("display_resolution", cssvalue('displayresolution')),
            ("display_protection", cssvalue('displayprotection')),
            ("display_other", cssvalue_join('displayother')),
            ("platform_os", cssvalue('os')),
            ("platform_chipset", cssvalue('chipset')),
            ("platform_cpu", cssvalue('cpu')),
            ("platform_gpu", cssvalue('gpu')),
            ("memory_card_slot", cssvalue('memoryslot')),
            ("memory_internal", cssvalue('internalmemory')),
            ("memory_other", cssvalue('memoryother')),
            ("main_camera_single", cssvalue('cam1modules')),
            ("main_camera_features", cssvalue('cam1features')),
            ("main_camera_video", cssvalue('cam1video')),
            ("selfie_camera_single", cssvalue('cam2modules')),
            ("selfie_camera_features", cssvalue('cam2features')),
            ("selfie_camera_video", cssvalue('cam2video')),
            ("sound_loudspeaker", body.css('a[href="glossary.php3?term=loudspeaker"]').xpath('./parent::*/parent::*').css('td.nfo::text').extract_first()),
            ("sound_35_jack", body.css('a[href="glossary.php3?term=audio-jack"]').xpath('./parent::*/parent::*').css('td.nfo::text').extract_first()),
            ("optional_other", cssvalue('optionalother')),
            ("wlan", cssvalue('wlan')),
            ("bluetooth", cssvalue('bluetooth')),
            ("gps", cssvalue('gps')),
            ("radio", cssvalue('radio')),
            ("usb", cssvalue('usb')),
            ("sensors", cssvalue('sensors')), 
            ("features_other", cssvalue_join('featuresother')),
            ("battery_descr", cssvalue('batdescription1')),
            ("battery_talk_time", cssvalue('battalktime1')),
            ("battery_standby", cssvalue('batstandby1')),
            ("battary_music_play", cssvalue("batmusicplayback1")),
            ("misc_colors", cssvalue('colors')),
            ("misc_sar_eu", cssvalue_join('sar-eu')),
            ("misc_sar_us", cssvalue_join('sar-us')),
            ("price", cssvalue('price')),
            ("test_performance", " <br/> ".join(body.css('*[data-spec="tbench"] > a::text').extract())),
            ("test_battary_life", body.css('*[data-spec="batlife"] > div > a::text').extract_first()),
            ("test_display", ", ".join(tests.xpath(".//td/a[contains(./text(), 'Display')]/parent::*/parent::*").css('td + td > a::text').extract())),
            ("test_camera", ", ".join(tests.xpath(".//td/a[contains(./text(), 'Camera')]/parent::*/parent::*").css('td + td > a::text').extract())),
            ("test_loudspeaker", ", ".join(tests.xpath(".//td/a[contains(./text(), 'Loudspeaker')]/parent::*/parent::*").css('td + td > a::text').extract())),
            ("test_audio_quality", ", ".join(tests.xpath(".//td/a[contains(./text(), 'Audio quality')]/parent::*/parent::*").css('td + td > a::text').extract()))
        ]}

        if len(unhandled_fields) > 0: 
            with(open("unhandled_fields", "a")) as f:
               f.write(response.url + " " + ", ".join(unhandled_fields) + "\n")

        if len(unhandled_test_fields) > 0: 
            with(open("unhandled_test_fields", "a")) as f:
               f.write(response.url + " " + ", ".join(unhandled_test_fields) + "\n")

        yield data

        if response.url not in handled_comments:
            comments_url = response.urljoin(body.css('div.sub-footer > div.button-links > ul > li > a::attr(href)').extract_first())
            comments_phone_request = self.make_scrapy_request(url=comments_url, callback=self.parse_comments_page)
            self.set_single_phone_metadata(comments_phone_request, vendor, phone_brief, phone_title, response.url)
            yield comments_phone_request

    def parse_comments_page(self, response):
        vendor = response.meta['vendor']
        phone_brief = response.meta['phone_brief'] 
        phone_title = response.meta['phone_title'] 
        phone_url = response.meta['phone_url'] 

        body = response.css('div#user-comments')
        user_threads = body.css('div.user-thread')

        def convert_to_date(s):
            if not s:
                return None
            d_m = self.metadata.hours_ago.match(s)
            h_m = self.metadata.hours_ago.match(s)
            m_m = self.metadata.minutes_ago.match(s)
            s_m = self.metadata.seconds_ago.match(s)
            if d_m:
                return (datetime.datetime.now() - datetime.timedelta(days=int(h_m.group(1)))).date()
            if h_m:
                return (datetime.datetime.now() - datetime.timedelta(hours=int(h_m.group(1)))).date()
            if m_m:
                return (datetime.datetime.now() - datetime.timedelta(minutes=int(m_m.group(1)))).date()
            if s_m:
                return (datetime.datetime.now() - datetime.timedelta(seconds=int(m_m.group(1)))).date()
            return datetime.datetime.strptime(s, "%d %b %Y").date() 

        for user_thread in user_threads:
            reply_text = user_thread.css('span.uinreply-msg::text').extract_first()
            if reply_text:
                reply_text = reply_text.replace("\r", "").replace("\n", " <br/> ")
            user_post_text = user_thread.css('p.uopin::text').extract_first()
            if user_post_text:
                user_post_text = user_post_text.replace("\r", "").replace("\n", " <br/> ")

            reply_msg_id = user_thread.css('span.uinreply-msg a::attr(href)').extract_first()
            reply_msg_id = reply_msg_id.replace("#", '') if reply_msg_id else None

            user_reply_name_date = user_thread.css('span.uinreply::text').extract_first()
            user_reply_name = None
            user_reply_date = None
            if user_reply_name_date:
                user_reply_name, user_reply_date =  user_reply_name_date.split(', ')

            user_post_count = user_thread.css('span.uavatar-stat.post-count::text').extract_first()
            user_upvote_count = user_thread.css('span.uavatar-stat.upvote-count::text').extract_first()
            user_post_date = user_thread.css('li.upost time::text').extract_first()
            user_post_rating = user_thread.css('span.thumbs-score::text').extract_first()
            post_id = user_thread.css('::attr(id)').extract_first()
            data = {"user_post": [
                ("post_id", int(post_id) if post_id else None),
                ("vendor_name", vendor),
                ("phone_title", phone_title),
                ("phone_url", phone_url),
                ("user_name", user_thread.css('li.uname2::text').extract_first()),
                ("user_location", user_thread.css('li.ulocation span::text').extract_first()),
                ("user_location_type", user_thread.css('li.ulocation span::attr(title)').extract_first()),
                ("user_post_date", convert_to_date(user_post_date)),
                ("user_post_rating", int(user_post_rating) if user_post_rating else None),
                ("user_post_text", user_post_text),
                ("user_reply_name", user_reply_name),
                ("user_reply_date", convert_to_date(user_reply_date)),
                ("user_reply_msg", reply_text),
                ("user_reply_msg_id", int(reply_msg_id) if reply_msg_id else None),
                ("user_post_count", int(user_post_count) if user_post_count else None),
                ("user_upvote_count", int(user_upvote_count) if user_upvote_count else None),
            ]}
            yield data

        nextpage_url = response.css('div#user-pages a[title="Next page"]::attr(href)').extract_first()
        if nextpage_url:
           nextpage_request = self.make_scrapy_request(url=response.urljoin(nextpage_url), callback=self.parse_comments_page)
           self.set_single_phone_metadata(nextpage_request, vendor, phone_brief, phone_title, phone_url)
           yield nextpage_request
        else:
            yield { "scrap_comments_status": [
                ("vendor_name", vendor),
                ("phone_title", phone_title), 
                ("phone_url", phone_url),
                ("scrapped", True),
            ]}


        
def crawl_gsmarena(
        loglevel = 'INFO',
        proxy = {"http":[], "https": []},
        config_download_delay = 0.25,
        user_agent =  'Mozilla/5.0 (X11; Linux x86_64…) Gecko/20100101 Firefox/63.0',
        xlsx_out = { 'enabled': True },
        mysql_out = { 'enabled': True }
    ):

    xlsx_out_default = { 
        'enable': True,
        'cache_interval': 10000,
        'filename': 'output',
        'pipeline': "gsmarena.pipeline_excel.WritePhonesToXlsx" 
    }

    mysql_out_defalut = { 
        'pipeline': "gsmarena.pipeline_sql.WritePhonesToDB",
        'enable': True,
        'user': 'gsmarena_user',
        'password': '12345678',
        'host': '127.0.0.1',
        'port': 3306,
        'database': 'gsmarena',
        'autocommit': 1
    }

    compose_lambda = lambda d, dflt: { k: (dflt[k] if k not in d else d[k]) for k in dflt} if d else None
    xlsx_out = compose_lambda(xlsx_out, xlsx_out_default)
    mysql_out = compose_lambda(mysql_out, mysql_out_defalut)

    configure_logging({"LOG_ENABLED": True, "LOG_LEVEL": loglevel})

    class ScraperGsmarena(ScraperGsmarenaMixin, scrapy.Spider):
        name = "gsmarena"
    
        custom_settings = {
            "LOG_LEVEL": loglevel
        }
        download_delay = config_download_delay

    settings = {
        'USER_AGENT': user_agent,
        'ITEM_PIPELINES': {},
        'SKIP_HANDLED_SOURCES': {}
    }

    if xlsx_out and xlsx_out.get('enable') == True:
        settings['ITEM_PIPELINES'][ xlsx_out['pipeline'] ] = 300
        settings['XLSX_OUT_NAME'] = xlsx_out['filename']
        settings['XLSX_OUT_CACHE_INTERVAL'] = xlsx_out['cache_interval']

    if mysql_out and mysql_out.get('enable') == True:
        settings['ITEM_PIPELINES'][ mysql_out['pipeline'] ] = 400
        settings['MYSQL_USERNAME'] = mysql_out['user']
        settings['MYSQL_PORT'] = mysql_out['port']
        settings['MYSQL_HOST'] = mysql_out['host']
        settings['MYSQL_PASSWORD'] = mysql_out['password']
        settings['MYSQL_DATABASE'] = mysql_out['database']
        settings['MYSQL_AUTOCOMMIT'] = mysql_out['autocommit']
        settings['SKIP_HANDLED_SOURCES'] = get_handled_sources(
            mysql_out['user'], mysql_out['password'], mysql_out['host'], mysql_out['port'], mysql_out['database'])

    if proxy is not None and isinstance(proxy, dict) and len(proxy) > 0:
        settings['DOWNLOADER_MIDDLEWARES'] = { 'scrapy.downloadermiddlewares.httpproxy.HttpProxyMiddleware': 750 }
        settings['USER_HTTP_PROXIES'] = proxy["http"]
        settings['USER_HTTPS_PROXIES'] = proxy["https"]

    process = CrawlerProcess(settings)
    
    process.crawl(ScraperGsmarena)
    process.start() # the script will block here until the crawling is finished



if __name__ == '__main__':
    crawl_gsmarena(
        loglevel = "INFO",
        xlsx_out = { "enable": False, 'pipeline': "pipeline_excel.WritePhonesToXlsx" },
        mysql_out = { 'pipeline': "pipeline_sql.WritePhonesToDB" },
        config_download_delay = 0.1,
        proxy = {
        "http":  ["http://46.38.35.62:32540",
         "http://212.33.239.50:46961",
         "http://188.170.38.66:55623",
         "http://195.88.16.145:35256",
         "http://185.26.219.34:40981",
         "http://185.17.203.153:52132",
         "http://77.244.16.5:45973"],
         
        "https": [
            # "https://82.114.92.122:49968", # slow
            "https://178.150.223.2:41258",
            "https://139.59.129.174:8080",
            "https://138.197.182.151:8080"

            ]
    })





