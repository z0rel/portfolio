#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import xmltodict
import traceback
import io
import threading
import random
import simplejson
from inspect import currentframe, getframeinfo
from contextlib import contextmanager
import lxml.html
from lxml import etree
from lxml.html.clean import Cleaner
from tr_shared import logger

# import google_translator
import googletr_fast
# import imp
# imp.reload(googletr_fast)


@contextmanager
def stdout_redirector(stream, logger, term_suffix, frameinfo):
    old_stdout = sys.stdout
    old_stderr = sys.stderr
    sys.stdout = stream
    sys.stderr = stream
    if logger:
        logger.set_term_suffix("{0}:{1}: ".format(frameinfo.filename, frameinfo.lineno) + term_suffix)
    try:
        yield
    finally:
        sys.stdout = old_stdout
        sys.stderr = old_stderr
        if logger:
            logger.set_term_suffix(None)


with stdout_redirector(io.StringIO(), None, None, None):
    import requests

needBing = False

session_mymemory = requests.Session()
session_promt = requests.Session()
if needBing:
    session_bing = requests.Session()
session_google = requests.Session()
session_yandex = requests.Session()


class exceptionDecorate:
    def __init__(self, fun):
        self.fun = fun

    def log_tb_repr(self, tb):
        def log_str(i):
            i = i.rstrip()
            for s in i.split("\n"):
                logger.info(s.rstrip())

        if isinstance(tb, list):
            for i in tb:
                log_str(i)
        elif isinstance(tb, str):
            log_str(tb)
        else:
            logger.info(repr(tb))

    def exceptionInfo(self, text=''):
        try:
            logger.info('EXCEPTION in: {fun}()'.format(fun=self.fun.__name__))
            exc_type, exc_value, exc_traceback = sys.exc_info()
            self.log_tb_repr(traceback.format_exception(exc_type, exc_value, exc_traceback))
        except:
            logger.info('EXCEPTION in: exceptionInfo decorator')
            exc_type, exc_value, exc_traceback = sys.exc_info()
            self.log_tb_repr(traceback.format_exception(exc_type, exc_value, exc_traceback))

    def __call__(self, *pargs, **kargs):
        if __name__ == '__main__':
            return self.fun(*pargs, **kargs)
        else:
            try:
                return self.fun(*pargs, **kargs)
            except Exception as i:
                self.exceptionInfo()
                return None


@exceptionDecorate
def tr_google_fast(trText, sl, tl):
    return googletr_fast.tr_google_fast(trText, sl, tl, session_google)


@exceptionDecorate
def tr_my_memory(baseTerm, fromLang, toLang):
    paramsTr = {'q': baseTerm,
                'langpair': "%s|%s" % (fromLang, toLang),
                # 'key': "rdF4FlQVxta9w",
                'de': "vasya123999@google.com"}

    req = requests.Request('GET', 'http://api.mymemory.translated.net/get', params=paramsTr)
    prp_req = req.prepare()
    try:
        data = session_mymemory.send(prp_req, verify=False, timeout=8).json()
    except simplejson.scanner.JSONDecodeError:
        return None, None

    resTxt = []
    resLen = len(data['matches'])
    if resLen > 0:
        for i, match in enumerate(data['matches']):
            if (match['segment'].strip() == baseTerm.strip()):
                if resLen != 1:
                    txt = "{tr}".format(tr=match['translation'])
                else:
                    txt = "{tr}".format(tr=match['translation'])
            else:
                if resLen != 1:
                    txt = "{i}: {fr} => {tr}".format(
                        i=i,
                        fr=match['segment'],
                        tr=match['translation']
                    )
                else:
                    txt = "{tr}".format(tr=match['translation'])
            resTxt.append(txt)
            break

        # txt = data['matches'][0]['translation'].encode('utf-8').split("\n")
    return resTxt, None


class TransformPromtText:
    findChRech = etree.XPath("//div[@class='ref_cform']/span[@class='ref_psp']")
    findRod = etree.XPath("//div[@id='translations']//span[@class='ref_info']")
    findTranslations = etree.XPath("//div[@id='translations']//span[@class='ref_result']")
    cleaner = Cleaner(style=True, links=True, add_nofollow=True, page_structure=False, safe_attrs_only=False, kill_tags=["img", "a"])

    def getChRech(root):
        res = list(sorted(set([node.text for node in TransformPromtText.findChRech(root) if node.text])))
        if res:
            return ", ".join(res)
        return ""

    def getRod(root):
        res = list(sorted(set([node.text for node in TransformPromtText.findRod(root) if node.text])))
        if res:
            return ", ".join(res)
        return ""

    def getTranslations(root):
        return list(sorted(set([node[0].tail.strip() for node in TransformPromtText.findTranslations(root)])))


@exceptionDecorate
def tr_promt(baseTerm, fromLang, toLang):
    # тематика 'естественные науки':
    # template:'Natural', selectedTpl=Natural
    trDir = {('ru', 'en'): 'ru-en', ('en', 'ru'): 'en-ru'}
    paramsTr = {
        'dirCode': trDir[(fromLang, toLang)],
        'template': 'Computer',
        'text': baseTerm,
        'lang': toLang,
        'limit': '3000',
        'useAutoDetect': 'true',
        'ts': 'MainSite',
        'tid': '',
        'key': '',
        'IsMobile': False
      }

    try:
        req = requests.Request('GET', 'http://www.translate.ru/services/TranslationService.asmx/GetTranslateNew',
                               params=paramsTr, cookies={'selectedTpl': 'Computer'})
        prp_req = req.prepare()
        r = session_promt.send(prp_req, verify=False, timeout=4)

    except requests.exceptions.ReadTimeout as e:
        return ["error " + str(e)], None
    except requests.packages.urllib3.exceptions.ReadTimeoutError as e:
        return ["error " + str(e)], None

    data = xmltodict.parse(r.text)
    result = [data['TranslationResult']['result']]

    try:
        res = TransformPromtText.cleaner.clean_html("<html>" + "\n".join(result) + "</html>")
        root = etree.XML(res)
        wordTrResult = TransformPromtText.getTranslations(root)
    except lxml.etree.XMLSyntaxError:
        return result, None

    if not wordTrResult:
        return result, None

    ps = TransformPromtText.getChRech(root)
    rod = TransformPromtText.getRod(root)
    if len(ps) > 0 and len(rod) > 0:
        h = "<h>" + ps + " / " + rod + ": </h>"
    elif len(ps) == 0 and len(rod) == 0:
        h = ""
    else:
        h = "<h>" + ps + rod + ": </h>"

    return ["{h}{w}".format(w=", ".join(wordTrResult), h=h)], None


@exceptionDecorate
def tr_bing(baseTerm, fromLang, toLang):
    uri = "https://www.bing.com/translator/api/Translate/TranslateArray"
    x = 48035562
    payload = [{"id": -random.randint(x, x+10000), "text": baseTerm}]

    headers = {
        'Accept': 'application/json, text/javascript, */*; q=0.01',
        'Accept-Encoding': 'gzip, deflate, br',
        'Accept-Language': 'ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4',
        'Connection': 'keep-alive',
        'Content-Type': 'application/json; charset=UTF-8',
        'Host': 'www.bing.com',
        'Origin': 'https://www.bing.com',
        'Referer': 'https://www.bing.com/translator',
        'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.68 Safari/537.36',
        'X-Requested-With': 'XMLHttpRequest',
    }

    global bing_mtstkn
    global bing_MUID

    cookies = {
        'mtstkn':    bing_mtstkn,
        'MUID':      bing_MUID,
        'sourceDia': {'ru': 'ru-RU', 'en': 'en-US'}[fromLang],
        'srcLang':   fromLang,
        'destLang':  toLang,
        'smru_list': fromLang,
        'dmru_list': fromLang + '%2Cen',
        'destDia':   {'ru': 'ru-RU', 'en': 'en-US'}[toLang],
    }
    data = str(payload).encode('utf-8')

    req = requests.Request('POST', uri, data=data, headers=headers, params={'from': fromLang, 'to': toLang},
                           cookies=cookies)
    prp_req = req.prepare()

    with stdout_redirector(logger.stream, logger, 'BING: send POST', getframeinfo(currentframe())):
        resp = session_bing.send(prp_req)

    res = resp.json()

    translations = []
    if 'items' in res:
        xtxt = res['items']
        for i in xtxt:
            translations.append(i['text'])
    else:
        with stdout_redirector(logger.stream, logger, 'bing - logger.info(resp.reason)', getframeinfo(currentframe())):
            logger.info(resp.reason)
            logger.info(resp.text)
    return translations, None


def authenticate_bing():
    session = requests.Session()
    tkn_req = requests.Request('GET', 'http://www.bing.com/translator/')
    prep_tkn_req = tkn_req.prepare()
    tkn_resp = session.send(prep_tkn_req)
    try:
        bing_mtstkn = tkn_resp.cookies['mtstkn']
    except KeyError as e:
        logger.info(str(e))
        logger.info('BING AUTHENTICATION ERROR: mtstkn cookie not exists')
        return "", ""

    try:
        bing_MUID = tkn_resp.cookies['MUID']
    except KeyError as e:
        logger.info(str(e))
        logger.info('BING AUTHENTICATION ERROR: MUID cookie not exists')
        return "", ""

    return bing_MUID, bing_mtstkn

if needBing:
    bing_MUID, bing_mtstkn = authenticate_bing()


@exceptionDecorate
def tr_yandex(baseTerm, fromLang, toLang):
    paramsTr = {
        'key': 'trnsl.1.1.20170523T144758Z.bfae2972de6c70a1.659c43f7844c457ef4466745d228c6369ade6381',
        'lang': "%s-%s" % (fromLang, toLang),
        'text': baseTerm,
        'options': '1'
      }

    with stdout_redirector(logger.stream, logger, 'yandex - prepare and send GET', getframeinfo(currentframe())):
        req = requests.Request('GET', 'https://translate.yandex.net/api/v1.5/tr.json/translate', params=paramsTr)
        req_prep = req.prepare()
        data = session_yandex.send(req_prep, verify=False, timeout=2).json()

    s = []
    for i in range(0, len(data['text'])):
        s.append(data['text'][i])  # .encode('utf-8').split("\n")
    return s, None


trList = [
#    ("MyMemory", tr_my_memory),
#    ("Promt", tr_promt),
    ("Google", tr_google_fast),
]

#if needBing:
#    trList.append(("Bing", tr_bing))

#trList.append(("Yandex", tr_yandex))

# Для ускорения запросы запускаются в отдельных потоках
class TranslatorThread(threading.Thread):
    def __init__(self, name, action, baseTerm, fromLang, toLang):
        threading.Thread.__init__(self)
        self.action = action
        self.baseTerm = baseTerm
        self.fromLang = fromLang
        self.toLang = toLang
        self.actionName = name

    def run(self):
        res = self.action(self.baseTerm, self.fromLang, self.toLang)
        if res is None:
            res = [None, None]
        self.translation = res[0]
        self.is_sentence = res[1] is not None and res[1]


def getTranslations(baseTerm, fromLang, toLang):
    l = [TranslatorThread(name, action, baseTerm, fromLang, toLang) for (name, action) in trList]

    for i in l:
        i.start()
    for i in l:
        i.join()
    res = []
    d = {}
    is_sentence = False
    for t in l:
        if not t.translation or t.translation is None:
            continue
        is_sentence = is_sentence or t.is_sentence

        if isinstance(t.translation, str):
            d_res = [t.translation]
        else:
            d_res = t.translation

        try:
            d[t.actionName].extend(d_res)
        except KeyError:
            d[t.actionName] = d_res

    for k, i in sorted(d.items()):
        res.append((k, i))
    return res, is_sentence


def test():
    rl = [
        # getTranslations("метода", 'ru', 'en') +
        # getTranslations("method", 'en', 'ru') #+
        # getTranslations("going", 'en', 'ru') ,
        # getTranslations("expressions", 'en', 'ru') ,
        getTranslations("In general, the task is to optimize certain properties of a system by pertinently " +
                        "choosing the system parameters.", "en", "ru"),
        # getTranslations("Если в очереди есть таймауты - выбрать минимальный таймаут из очереди (очереди чего?). " +
        #                 "Если в очереди ничего нет, уснуть бесконечно.", 'ru', 'en')
        # getTranslations("Оптимизация метода LP-вывода", 'ru', 'en')
        # getTranslations("Optimizations of the LP-inference method", 'en', 'ru')
        # getTranslations("Optimizations of the LP-inference method. "+
        #                  "The perturbation is done for every population vector.", 'en', 'ru') ,
        # getTranslations("The perturbation is done for every population vector. going.", 'en', 'ru')
    ]

    for inst, is_sent in rl:
        for name, translation in inst:
            print("{0}:".format(name))
            print("\n".join(translation))
            sys.stdout.flush()

logger.init()

if __name__ == '__main__':
    test()


# vim: set fileencoding=utf-8 :
