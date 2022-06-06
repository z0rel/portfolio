#!/usr/bin/env python3
# coding=utf8

""" The gsmarena site scraper """

import datetime
import random
import urllib
import re
from functools import partial

import scrapy
from scrapy.crawler import CrawlerProcess
from scrapy.utils.log import configure_logging

from mysql.connector import connection

from .pipeline_sql import get_handled_sources


class StaticData:  # pylint: disable=too-few-public-methods
    """ Static data used in scraping process """
    handled_fields = set([
        'released-hl', 'body-hl', 'os-hl', 'storage-hl', 'modelname',
        'displaysize-hl', 'displayres-hl', 'camerapixels-hl', 'videopixels-hl',
        'ramsize-hl', 'chipset-hl', 'batsize-hl', 'battype-hl', 'comment',
        'nettech', 'edge', 'gprstext', 'net2g', 'net3g', 'net4g', 'nfc', 'speed', 'year',
        'status', 'dimensions', 'weight', 'sim', 'bodyother', 'build', 'displaytype',
        'displaysize', 'displayresolution', 'displayprotection', 'displayother', 'os',
        'chipset', 'cpu', 'gpu', 'memoryslot', 'internalmemory', 'memoryother',
        'cam1modules', 'cam1features', 'cam1video', 'cam2modules', 'cam2features',
        'cam2video', 'optionalother', 'wlan', 'bluetooth', 'gps', 'radio', 'usb',
        'sensors', 'featuresother', 'batdescription1', 'battalktime1', "batstandby1",
        'colors', 'price', "batlife", "batmusicplayback1", "tbench", 'sar-eu', 'sar-us'
    ])

    handled_test_fields = set([
        'Performance', 'Display', 'Camera', 'Loudspeaker', 'Audio quality', 'Battery life'
    ])

    integer_re = re.compile(r'\d+')
    hours_ago = re.compile(r"(\d+) days? ago")
    hours_ago = re.compile(r"(\d+) hours? ago")
    minutes_ago = re.compile(r"(\d+) minute?s? ago")
    seconds_ago = re.compile(r"(\d+) seconds? ago")
    login_date_rxp = re.compile(r"(, )\d+ [a-zA-Z]+ \d+$")
    comment_page_number = re.compile(r"^(.*)p(\d+).php$")

    @staticmethod
    def login_date_split(string):
        """ split login and date to substrings """
        if not string:
            return None, None
        matches = list(StaticData.login_date_rxp.finditer(string))
        if not matches:
            return string, None
        start = matches[0].start()
        end = matches[0].end()
        return string[:start], string[start+2:end]


def log_unhandled_phone_fields(url, unhandled_fields, unhandled_test_fields):
    """ If exists the unscraped fields - log it to files """
    if unhandled_fields:
        with(open("unhandled_fields", "a")) as log_file:
            log_file.write(url + " " + ", ".join(unhandled_fields) + "\n")

    if unhandled_test_fields:
        with(open("unhandled_test_fields", "a")) as log_file:
            log_file.write(url + " " + ", ".join(unhandled_test_fields) + "\n")


def set_single_phone_metadata(request, vendor, phone_brief, phone_title, phone_url):
    """ Setup metadata of single phone to request """
    request.meta['vendor'] = vendor
    request.meta['phone_brief'] = phone_brief
    request.meta['phone_title'] = phone_title
    request.meta['phone_url'] = phone_url


class ScraperGsmarenaMixin:
    """ The scraper gsmarena site logic """

    def __init__(self):
        self.phones_scraped = 0
        self.phones_total = 0
        self.comments_scraped = 0
        self.comments_total = 0

    metadata = StaticData()

    def make_scrapy_request(self, url, callback, *args, **kwargs):
        """ Wrapper for making scrapy request with random proxy """

        parsed_url = urllib.parse.urlparse(url)
        request = scrapy.Request(url, callback, *args, **kwargs)
        proxy = None
        if parsed_url.scheme == 'https' and 'USER_HTTPS_PROXIES' in self.settings:
            proxy = self.settings['USER_HTTPS_PROXIES']
        elif parsed_url.scheme == 'http' and 'USER_HTTP_PROXIES' in self.settings:
            proxy = self.settings['USER_HTTP_PROXIES']

        if proxy and isinstance(proxy, list):
            request.meta['proxy'] = random.choice(proxy)
        return request

    def start_requests(self):
        """ The scrap site entry point """
        callback = self.settings['CFG_ENTRY_POINT_CALLBACK']

        for url, *args in self.settings['CFG_ENTRY_POINT_URLS']:
            request = self.make_scrapy_request(url=url, callback=partial(callback, self))
            reqv_meta_setup = self.settings['CFG_ENTRY_POINT_REQV_SETUP']
            if reqv_meta_setup:
                reqv_meta_setup(request, *args)
            yield request

    def make_phone_group_request(self, response, vendor, group_url, priority=0):
        """ Wrapper to make request to single phone from vendor phones list  """
        next_page = response.urljoin(group_url)
        request_phone_page = self.make_scrapy_request(url=next_page,
                                                      callback=self.parse_phones_page,
                                                      priority=priority)
        request_phone_page.meta['vendor'] = vendor
        return request_phone_page

    def parse_main(self, response):
        """ Scrap the vendors list """
        phones_group = response.css("div#body div.st-text td a")

        vendors = []
        for group in phones_group:
            vendor = group.css("::text").extract_first()
            group_url = group.css("::attr(href)").extract_first()
            devices_count_text = group.css("span::text").extract_first()
            cnt_list = self.metadata.integer_re.findall(devices_count_text)
            devices_count = int(cnt_list[0]) if cnt_list else 0
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
                yield self.make_phone_group_request(response, vendor_name, vendor_url,
                                                    priority=999999)

    def parse_phones_page(self, response):
        """ Scrap the vendor phones list """

        paginators = response.css("div#body div.main")
        phones_list = paginators.css("div#review-body div.makers > ul > li")

        for phone in phones_list:
            phone_url = response.urljoin(phone.css("a::attr(href)").extract_first())
            if phone_url not in self.settings['SKIP_HANDLED_SOURCES'].get("phones", {}):
                self.phones_total += 1

        for phone in phones_list:
            phone_url = response.urljoin(phone.css("a::attr(href)").extract_first())
            if phone_url not in self.settings['SKIP_HANDLED_SOURCES'].get("phones", {}):
                phone_brief = phone.css("a img::attr(title)").extract_first()
                phone_title = phone.css("a strong span::text").extract_first()
                request_phone_page = self.make_scrapy_request(url=phone_url,
                                                              callback=self.parse_phone_page)
                set_single_phone_metadata(request_phone_page, response.meta['vendor'],
                                          phone_brief, phone_title, phone_url)
                request_phone_page.meta['phone_num'] = self.phones_scraped
                yield request_phone_page

        next_page = paginators.css("a.pages-next")
        nextpage_buttouns_group = next_page.css("::attr(class)").extract_first()

        if nextpage_buttouns_group:
            nextpage_class_list = nextpage_buttouns_group.split(" ")
            if 'disabled' not in nextpage_class_list:
                next_page_url = next_page.css("::attr(href)").extract_first()
                yield self.make_phone_group_request(response, response.meta['vendor'],
                                                    next_page_url,
                                                    priority=999999)
        else:
            service_data = {"scrap_vendor_status": [("vendor_name", response.meta['vendor']),
                                                    ("scrapped", True)]}
            yield service_data

    def if_log_progress(self, value):
        """ Log the scraping progress  """
        if not value % 100:
            self.logger.info("Scraped {0} of {1} comments, {2} of {3} phones".format(
                self.comments_scraped, self.comments_total,
                self.phones_scraped, self.phones_total))

    def parse_phone_page(self, response):
        """ Scrap the single phone page list """
        body = response.css('div.main.main-review')
        self.phones_scraped += 1
        self.if_log_progress(self.phones_scraped)

        class Items:
            """ Phone page items """

            @staticmethod
            def cssvalue(selector):
                """ Get the page value by css selector """
                return body.css('*[data-spec="' + selector + '"]::text').extract_first()

            @staticmethod
            def cssvalue_join(selector):
                """ Get multiline page value by css selector """
                cssspec = '*[data-spec="' + selector + '"]::text'
                return " <br/> ".join([(s.replace("\n", " ").replace("\r", " ") if s else '')
                                       for s in body.css(cssspec).extract()])

            @staticmethod
            def cssvalue_sib(selector):
                """ Get the sibling page values by css selector """
                sib1 = body.css('span[data-spec="' + selector + '"]::text').extract_first()
                sib2 = body.css('span[data-spec="' + selector + '"] + span::text').extract_first()
                return (str(sib1) if sib1 else '') + (str(sib2) if sib2 else '')

            tests = body.xpath(".//tr/th[contains(./text(), 'Tests')]/parent::*/parent::*")

            @staticmethod
            def main_img_url():
                """ Get the url of phone image """
                res = body.css('div.specs-photo-main a::attr(href)').extract_first()
                return response.urljoin(res) if res else None

            @staticmethod
            def hits_val():
                """ Get the hits value """
                hits_val = body.css('li.help-popularity span::text').extract_first()
                hits_val = StaticData.integer_re.findall(hits_val.replace(',', ''))[0]
                hits_val = int(hits_val) if hits_val else None

            handled_fields = set(response.css("*[data-spec]::attr(data-spec)").extract())

            @staticmethod
            def handled_test_fields():
                """ Get test fields names that were scraped """
                return set(Items.tests.css('td.ttl a::text').extract())

            @staticmethod
            def tests_item(sel):
                """ Get the table with data of tests  """
                tests = Items.tests
                res = tests.xpath(".//td/a[contains(./text(), '" + sel + "')]/parent::*/parent::*")
                return ", ".join(res.css('td + td > a::text').extract())

            @staticmethod
            def audio_value(selector):
                """ Get the audio section values by selector """
                res = body.css('a[href="glossary.php3?term=' + selector + '"]')
                return res.xpath('./parent::*/parent::*').css('td.nfo::text').extract_first()

            @staticmethod
            def test_performance():
                """ Get test performance benchmark item """
                return " <br/> ".join(body.css('*[data-spec="tbench"] > a::text').extract())

            @staticmethod
            def test_battary_life():
                """ Get battary life item """
                return body.css('*[data-spec="batlife"] > div > a::text').extract_first()

        log_unhandled_phone_fields(
            response.url,
            unhandled_fields=list(sorted(Items.handled_fields - self.metadata.handled_fields)),
            unhandled_test_fields=list(sorted(Items.handled_test_fields()
                                              - self.metadata.handled_test_fields))
        )

        comments_start_url = body.css('div.sub-footer > div.button-links > ul > li > a::attr(href)')
        comments_start_url = comments_start_url.extract_first()
        comments_start_url = response.urljoin(comments_start_url)

        data = {"phone_info": [
            ("vendor_name", response.meta['vendor']),
            ("phone_title", response.meta['phone_title']),
            ("phone_brief", response.meta['phone_brief']),
            ("phone_url", response.url),
            ("fullname", body.css('h1.specs-phone-name-title::text').extract_first()),
            ("main_img_url", Items.main_img_url()),
            ("become_a_fan", body.css('a.specs-fans strong::text').extract_first()),
            ("help_popularity", Items.hits_val()),
            ("released_date", Items.cssvalue("released-hl")),
            ("body_hl", Items.cssvalue('body-hl')),
            ("os_hl", Items.cssvalue('os-hl')),
            ("storage", Items.cssvalue('storage-hl')),
            ("modelname", Items.cssvalue('modelname')),
            ("displaysize", Items.cssvalue('displaysize-hl')),
            ("display_res", Items.cssvalue('displayres-hl')),
            ("camerapixels", Items.cssvalue_sib('camerapixels-hl')),
            ("videopixels", Items.cssvalue('videopixels-hl')),
            ("ramsize_gb", Items.cssvalue_sib('ramsize-hl')),
            ("chipset", Items.cssvalue('chipset-hl')),
            ("batsize", Items.cssvalue_sib('batsize-hl')),
            ("battype", Items.cssvalue('battype-hl')),
            ("comment", Items.cssvalue('comment')),
            ("net_tech", Items.cssvalue('nettech')),
            ("net_edge", Items.cssvalue('edge')),
            ("net_gprs", Items.cssvalue('gprstext')),
            ("net_2g", Items.cssvalue('net2g')),
            ("net_3g", Items.cssvalue('net3g')),
            ("net_4g", Items.cssvalue('net4g')),
            ("net_speed", Items.cssvalue('speed')),
            ("net_nfc", Items.cssvalue('nfc')),
            ("announced", Items.cssvalue('year')),
            ("phone_status", Items.cssvalue('status')),
            ("body_dimensions", Items.cssvalue('dimensions')),
            ("body_weight", Items.cssvalue('weight')),
            ("body_sim", Items.cssvalue('sim')),
            ("body_other", Items.cssvalue('bodyother')),
            ("body_build", Items.cssvalue('build')),
            ("display_type", Items.cssvalue('displaytype')),
            ("display_size", Items.cssvalue('displaysize')),
            ("display_resolution", Items.cssvalue('displayresolution')),
            ("display_protection", Items.cssvalue('displayprotection')),
            ("display_other", Items.cssvalue_join('displayother')),
            ("platform_os", Items.cssvalue('os')),
            ("platform_chipset", Items.cssvalue('chipset')),
            ("platform_cpu", Items.cssvalue('cpu')),
            ("platform_gpu", Items.cssvalue('gpu')),
            ("memory_card_slot", Items.cssvalue('memoryslot')),
            ("memory_internal", Items.cssvalue('internalmemory')),
            ("memory_other", Items.cssvalue('memoryother')),
            ("main_camera_single", Items.cssvalue('cam1modules')),
            ("main_camera_features", Items.cssvalue('cam1features')),
            ("main_camera_video", Items.cssvalue('cam1video')),
            ("selfie_camera_single", Items.cssvalue('cam2modules')),
            ("selfie_camera_features", Items.cssvalue('cam2features')),
            ("selfie_camera_video", Items.cssvalue('cam2video')),
            ("sound_loudspeaker", Items.audio_value('loudspeaker')),
            ("sound_35_jack", Items.audio_value('audio-jack')),
            ("optional_other", Items.cssvalue('optionalother')),
            ("wlan", Items.cssvalue('wlan')),
            ("bluetooth", Items.cssvalue('bluetooth')),
            ("gps", Items.cssvalue('gps')),
            ("radio", Items.cssvalue('radio')),
            ("usb", Items.cssvalue('usb')),
            ("sensors", Items.cssvalue('sensors')),
            ("features_other", Items.cssvalue_join('featuresother')),
            ("battery_descr", Items.cssvalue('batdescription1')),
            ("battery_talk_time", Items.cssvalue('battalktime1')),
            ("battery_standby", Items.cssvalue('batstandby1')),
            ("battary_music_play", Items.cssvalue("batmusicplayback1")),
            ("misc_colors", Items.cssvalue('colors')),
            ("misc_sar_eu", Items.cssvalue_join('sar-eu')),
            ("misc_sar_us", Items.cssvalue_join('sar-us')),
            ("price", Items.cssvalue('price')),
            ("test_performance", Items.test_performance()),
            ("test_battary_life", Items.test_battary_life()),
            ("test_display", Items.tests_item('Display')),
            ("test_camera", Items.tests_item('Camera')),
            ("test_loudspeaker", Items.tests_item('Loudspeaker')),
            ("test_audio_quality", Items.tests_item('Audio quality')),
            ("comment_start_url", comments_start_url)
        ]}

        yield data

        if 'phone_num' not in response.meta:
            response.meta['phone_num'] = self.phones_total
            self.phones_total += 1

        if response.url not in self.settings['SKIP_HANDLED_SOURCES'].get("comments", {}):
            # first comment page has max priority for count of total
            comments_phone_request = self.make_scrapy_request(url=comments_start_url,
                                                              callback=self.parse_comments_page,
                                                              priority=999999)
            set_single_phone_metadata(comments_phone_request, response.meta['vendor'],
                                      response.meta['phone_brief'], response.meta['phone_title'],
                                      response.url)
            comments_phone_request.meta["comments_page_first"] = True
            comments_phone_request.meta["phone_num"] = response.meta["phone_num"]
            self.comments_total += 1
            yield comments_phone_request

    def parse_comments_page(self, response): # pylint: disable=too-many-statements
        """ Scrap comments for phone """
        body = response.css('div#user-comments')

        def convert_to_date(string):
            """ Convert text date item to date type """
            if not string:
                return None
            d_m = self.metadata.hours_ago.match(string)
            h_m = self.metadata.hours_ago.match(string)
            m_m = self.metadata.minutes_ago.match(string)
            s_m = self.metadata.seconds_ago.match(string)

            def timedelta(key, rxp):
                return datetime.timedelta(**{key: int(rxp.group(1))})

            for key, math_obj in [("days", d_m), ("hours", h_m),
                                  ("minutes", m_m), ("seconds", s_m)]:
                if math_obj:
                    return (datetime.datetime.now() - timedelta(key, math_obj)).date()

            return datetime.datetime.strptime(string, "%d %b %Y").date()

        for post in body.css('div.user-thread'):
            user_reply_name_date = post.css('span.uinreply::text').extract_first()

            class Items:
                """ The comments items from scraped page """
                @staticmethod
                def reply_text(post):
                    """ Get the reply item text """
                    res = post.css('span.uinreply-msg::text').extract_first()
                    return res.replace("\r", "").replace("\n", " <br/> ") if res else None

                @staticmethod
                def user_post_text(post):
                    """ Get the user post text """
                    res = post.css('p.uopin::text').extract_first()
                    return res.replace("\r", "").replace("\n", " <br/> ") if res else None

                @staticmethod
                def reply_msg_id(post):
                    """ Get the reply text for current post """
                    res = post.css('span.uinreply-msg a::attr(href)').extract_first()
                    return int(res.replace("#", '')) if res else None

                @staticmethod
                def post_id(post):
                    """ Get the post id value """
                    res = post.css('::attr(id)').extract_first()
                    return int(res) if res else None

                user_reply_name, user_reply_date = StaticData.login_date_split(user_reply_name_date)

                @staticmethod
                def user_post_count(post):
                    """ Get the counts of posts value """
                    res = post.css('span.uavatar-stat.post-count::text').extract_first()
                    return int(res) if res else None

                @staticmethod
                def user_upvote_count(post):
                    """ Get the upvote value """
                    res = post.css('span.uavatar-stat.upvote-count::text').extract_first()
                    return int(res) if res else None

                user_post_date = post.css('li.upost time::text').extract_first()

                @staticmethod
                def user_post_rating(post):
                    """ Get the post rating value """
                    res = post.css('span.thumbs-score::text').extract_first()
                    return int(res) if res else None

            data = {"user_post": [
                ("post_id", Items.post_id(post)),
                ("vendor_name", response.meta['vendor']),
                ("phone_title", response.meta['phone_title']),
                ("phone_url", response.meta['phone_url']),
                ("user_name", post.css('li.uname2::text').extract_first()),
                ("user_location", post.css('li.ulocation span::text').extract_first()),
                ("user_location_type", post.css('li.ulocation span::attr(title)').extract_first()),
                ("user_post_date", convert_to_date(Items.user_post_date)),
                ("user_post_rating", Items.user_post_rating(post)),
                ("user_post_text", Items.user_post_text(post)),
                ("user_reply_name", Items.user_reply_name),
                ("user_reply_date", convert_to_date(Items.user_reply_date)),
                ("user_reply_msg", Items.reply_text(post)),
                ("user_reply_msg_id", Items.reply_msg_id(post)),
                ("user_post_count", Items.user_post_count(post)),
                ("user_upvote_count", Items.user_upvote_count(post)),
            ]}
            yield data

        self.comments_scraped += 1
        self.if_log_progress(self.comments_scraped)

        def get_nextpage_request(url, enable_fast_logic, i, last_page_num):
            """ Get the request for next comment page """
            # 2 - higer priority for not last comments page
            # 1 - low priority for last comments page
            is_last_page = i == last_page_num
            if is_last_page:
                pass
            priority = response.meta.get('phone_num', 0) + (2 if not is_last_page else 1)
            nextpage_request = self.make_scrapy_request(url=response.urljoin(url),
                                                        callback=self.parse_comments_page,
                                                        priority=priority)
            set_single_phone_metadata(nextpage_request, response.meta['vendor'],
                                      response.meta['phone_brief'], response.meta['phone_title'],
                                      response.meta['phone_url'])

            nextpage_request.meta['is_last_page'] = is_last_page
            nextpage_request.meta["comments_fast_scrap"] = enable_fast_logic
            return nextpage_request

        def comment_next_pages(response):
            """ Scrap the next comment pages """
            nextpage_item = response.css('div#user-pages a[title="Next page"]')
            nextpage_url = nextpage_item.css('::attr(href)')
            nextpage_url = nextpage_url.extract_first()
            pages_requests = None
            if nextpage_url:
                nextpage_url = response.urljoin(nextpage_url)
                if response.meta.get("comments_page_first", False):
                    last_page = nextpage_item.xpath("./preceding-sibling::a")[-1]
                    last_page = last_page.css("a::attr(href)").extract_first()
                    last_page_match = StaticData.comment_page_number.match(last_page)
                    if last_page_match:
                        last_page_num = int(last_page_match.group(2))
                        self.comments_total += last_page_num - 1
                        page_base = last_page_match.group(1)
                        pages_list = [response.urljoin(page_base + "p" + str(i) + ".php")
                                      for i in range(2, last_page_num+1)]
                        pages_requests = (get_nextpage_request(comment_page_url, True,
                                                               i, last_page_num)
                                          for (i, comment_page_url) in enumerate(pages_list))
            return pages_requests, nextpage_url

        pages_requests, nextpage_url = comment_next_pages(response)

        if nextpage_url:
            if pages_requests and response.meta.get("comments_page_first", False):
                for request in pages_requests:
                    yield request

            if not response.meta.get("comments_fast_scrap", False) and not pages_requests:
                request = get_nextpage_request(nextpage_url, False, 0, -1)
                yield request

        if response.meta.get('is_last_page', False):
            yield {"scrap_comments_status": [
                ("vendor_name", response.meta['vendor']),
                ("phone_title", response.meta['phone_title']),
                ("phone_url", response.meta['phone_url']),
                ("scrapped", True),
            ]}


def gsmarena_restore_parents(username, password, host, port, database):  # pylint: disable=too-many-locals
    """ Restore parents for single-row comments """
    cnx = connection.MySQLConnection(user=username, password=password, database=database,
                                     host=host, port=port)
    cnx.autocommit = False
    cur = cnx.cursor()
    cur.execute("SELECT post_id, user_reply_name, user_reply_date, user_reply_msg " +
                " FROM gsmarena_comments " +
                "WHERE user_reply_msg_id is NULL and user_reply_date is not NULL;")
    rows = cur.fetchall()

    print('Replies without parent id:', len(rows))
    cur.execute('SELECT post_id, user_post_text, user_post_date, user_name FROM gsmarena_comments ')
    hashed_row = {}
    for post_id, user_post_text, user_post_date, uname in cur.fetchall():
        try:
            hashed_row[(uname, user_post_date)].append((post_id, user_post_text))
        except KeyError:
            hashed_row[(uname, user_post_date)] = [(post_id, user_post_text)]

    cnt = 0
    for idx, (post_id, user, post_date, message) in enumerate(rows):
        vals = hashed_row.get((user, post_date), None)
        # cur.execute('SELECT post_id, user_post_text FROM gsmarena_comments '
        #             + ' WHERE user_post_date = %s and user_name = %s;', [post_date, user])
        # vals = cur.fetchall()
        if not vals or not message:
            continue
        for q_post_id, q_post_text in vals:
            if q_post_text and q_post_text == message[:len(q_post_text)]:
                cnt += 1
                if not cnt % 10000:
                    cnx.commit()
                print(idx, "source:", q_post_id, 'reply_post_id:', post_id)
                # cmd = ('UPDATE gsmarena_comments SET user_reply_msg_id = ' + str(q_post_id)
                #        + ' WHERE post_id = ' + str(post_id) + ";")
                # print(cmd)
                cur.execute('UPDATE gsmarena_comments SET user_reply_msg_id = %s' +
                            ' WHERE post_id = %s;',
                            [q_post_id, post_id])
    cnx.commit()
    cur.close()
    cnx.close()


def compose(value, default_value, skip_none=True):
    """ Compose default settings and settings from function argument """
    if skip_none and not value:
        return None
    if isinstance(value, bool):
        value = {"enable": value}

    value = {} if value is None else value
    return {k: (default_value[k] if k not in value else value[k])
            for k in default_value}


def get_mysql_config(mysql_out):
    """ Get full mysql configuration """
    mysql_out_defalut = {
        'pipeline': "scraperlib.pipeline_sql.WritePhonesToDB",
        'enable': True,
        'user': 'gsmarena_user',
        'password': '12345678',
        'host': '127.0.0.1',
        'port': 3306,
        'database': 'gsmarena',
        'autocommit': 1
    }
    return compose(mysql_out, mysql_out_defalut, skip_none=True)

def crawl_gsmarena(proxy=False, scrap_config=None, xlsx_out=True, mysql_out=True):
    """ The main function for crawling and scraping gsmarena site """

    scrap_config_default = {
        "loglevel": "INFO",
        "download_delay": 0.25,
        "user_agent": 'Mozilla/5.0 (X11; Linux x86_64…) Gecko/20100101 Firefox/63.0',
        "concurrent_requests": 50,
        "concurrent_requests_per_domain": 40,
        "concurrent_saving_items": 1000,
        'entry_point_urls': [['https://www.gsmarena.com/makers.php3']],
        'entry_point_callback': ScraperGsmarenaMixin.parse_main,
        'entry_point_reqv_setup': None
    }

    xlsx_out_default = {
        'enable': True,
        'cache_interval': 10000,
        'filename': 'output',
        'pipeline': "scraperlib.pipeline_excel.WritePhonesToXlsx"
    }

    xlsx_out = compose(xlsx_out, xlsx_out_default)
    mysql_out = get_mysql_config(mysql_out)
    scrap_config = compose(scrap_config, scrap_config_default, skip_none=False)

    configure_logging({"LOG_ENABLED": True, "LOG_LEVEL": scrap_config["loglevel"]})

    class ScraperGsmarena(ScraperGsmarenaMixin, scrapy.Spider):  # pylint: disable=abstract-method
        """ The gsmarena scraper with customized loglevel and delay settings """
        name = "gsmarena"

        custom_settings = {
            "LOG_LEVEL": scrap_config["loglevel"],
            "CONCURRENT_REQUESTS": scrap_config["concurrent_requests"],
            "CONCURRENT_REQUESTS_PER_DOMAIN": scrap_config["concurrent_requests_per_domain"],
            "CONCURRENT_ITEMS": scrap_config["concurrent_saving_items"],
        }
        download_delay = scrap_config["download_delay"]

    settings = {
        'USER_AGENT': scrap_config["user_agent"],
        'ITEM_PIPELINES': {},
        'SKIP_HANDLED_SOURCES': {},
        'CFG_ENTRY_POINT_URLS': scrap_config['entry_point_urls'],
        'CFG_ENTRY_POINT_CALLBACK': scrap_config['entry_point_callback'],
        'CFG_ENTRY_POINT_REQV_SETUP': scrap_config['entry_point_reqv_setup'],
    }

    if xlsx_out and xlsx_out.get('enable'):
        settings['ITEM_PIPELINES'][xlsx_out['pipeline']] = 300
        settings['XLSX_OUT_NAME'] = xlsx_out['filename']
        settings['XLSX_OUT_CACHE_INTERVAL'] = xlsx_out['cache_interval']

    if mysql_out and mysql_out.get('enable'):
        settings['ITEM_PIPELINES'][mysql_out['pipeline']] = 400
        settings['MYSQL_USERNAME'] = mysql_out['user']
        settings['MYSQL_PORT'] = mysql_out['port']
        settings['MYSQL_HOST'] = mysql_out['host']
        settings['MYSQL_PASSWORD'] = mysql_out['password']
        settings['MYSQL_DATABASE'] = mysql_out['database']
        settings['MYSQL_AUTOCOMMIT'] = mysql_out['autocommit']
        settings['SKIP_HANDLED_SOURCES'] = get_handled_sources(
            mysql_out['user'], mysql_out['password'], mysql_out['host'],
            mysql_out['port'], mysql_out['database'])

    if proxy is not None and isinstance(proxy, dict) and proxy:
        settings['DOWNLOADER_MIDDLEWARES'] = {
            'scrapy.downloadermiddlewares.httpproxy.HttpProxyMiddleware': 750
        }
        settings['USER_HTTP_PROXIES'] = proxy["http"]
        settings['USER_HTTPS_PROXIES'] = proxy["https"]

    process = CrawlerProcess(settings)
    process.crawl(ScraperGsmarena)
    process.start()  # the script will block here until the crawling is finished


def pool_proc_fun(arg):
    """ Multiprocess crawler entry point"""
    print(""" Crawling start """)
    crawl_gsmarena(scrap_config=arg.scrap_config, xlsx_out=arg.xlsx_out,
                   mysql_out=arg.mysql_out, proxy=arg.proxy)

def multiprocess_crawl_comments(process_cnt, chunck_num,  # pylint: disable=too-many-arguments
                                proxy=False, scrap_config=None, xlsx_out=True, mysql_out=True):
    """Crawl comment in multiprocess mode"""

    def chunker_list(seq, size):
        """ Split sequence to size chuncks """
        return [seq[i::size] for i in range(size)]

    if not mysql_out:
        return

    cfg = get_mysql_config(mysql_out)
    cnx = connection.MySQLConnection(user=cfg['user'], password=cfg['password'],
                                     database=cfg['database'],
                                     host=cfg['host'], port=cfg['port'])
    cur = cnx.cursor()
    cur.execute("SELECT phone_url, vendor_name, phone_title, comment_start_url, phone_brief"
                + " FROM gsmarena_phones")
    urls = cur.fetchall()
    cur.close()
    cnx.close()
    handled_sources = get_handled_sources(cfg['user'], cfg['password'], cfg['host'],
                                          cfg['port'], cfg['database']).get("comments", [])

    handled_sources = {x[0] for x in handled_sources}
    urls = [(comment_url, phone_url, vendor, title, phone_brief)
            for (phone_url, vendor, title, comment_url, phone_brief) in urls
            if phone_url not in handled_sources]

    def setup_comments_reqv(reqv, phone_url, vendor, title, phone_brief):
        """ Setup comments request as entry point """
        reqv.meta["comments_page_first"] = True
        reqv.meta["phone_title"] = title
        reqv.meta["phone_url"] = phone_url
        reqv.meta["vendor"] = vendor
        reqv.meta["phone_brief"] = phone_brief

    scrap_config['entry_point_callback'] = ScraperGsmarenaMixin.parse_comments_page
    scrap_config['entry_point_reqv_setup'] = setup_comments_reqv
    scrap_config['entry_point_urls'] = chunker_list(urls, process_cnt)[chunck_num]

    crawl_gsmarena(scrap_config=scrap_config, xlsx_out=xlsx_out,
                   mysql_out=mysql_out, proxy=proxy)
