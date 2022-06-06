#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json
import requests
from urllib.parse import quote
from subprocess import check_output
import json
import re
# https://github.com/matheuss/google-translate-token

unneed_headers = {
    "authority": "translate.google.com",
    "accept": "*/*",
    "accept-encoding": "gzip, deflate, sdch, br",
    "accept-language": "ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4",
    "user-agent": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.100 Safari/537.36",
    "x-client-data": "CI22yQEIorbJAQihisoBCKmdygE="
}


def tr_google_fast(base_term, from_lang, to_lang, session_google):
    # base_url = 'https://translate.google.com/translate_a/single?client=t'
    base_url = 'https://translate.google.com/translate_a/single?client=webapp'
    langs = '&sl={sl}&tl={tl}&hl={hl}'.format(sl=from_lang, tl=to_lang, hl=to_lang)
    # mixin = '&dt=at&dt=bd&dt=ex&dt=ld&dt=md&dt=qca&dt=rw&dt=rm&dt=ss&dt=t&ie=UTF-8&oe=UTF-8&otf=1&pc=1&ssel=6&tsel=3&kc=5'

    escaped_base_term = base_term.replace("'", "\\'")
    jsfun = 'function(value) { console.log(value.value); }'
    jsfun = '() => console.log(value.value)'
    workdir = os.path.dirname(os.path.realpath(__file__))

    args = [
        "C:\\Program Files\\nodejs\\node.exe",
        "-e",
        f'''require("@iamtraction/google-translate")("{escaped_base_term}", {{ from: "{from_lang}", to: "{
            to_lang}"}}).then(x => console.log(JSON.stringify(x))).catch(x => console.log(JSON.stringify(x)))''']


    output = check_output(args, cwd=workdir).decode().strip()
    obj = json.loads(output)
    return obj["text"], True

    # mixin = "&dt=at&dt=bd&dt=ex&dt=ld&dt=md&dt=qca&dt=rw&dt=rm&dt=ss&dt=t&otf=2&ssel=0&tsel=0&kc=3&tk={tk}&q={q}"
    # print(workdir)
    # print(args)
    # mixin = mixin.format(tk=check_output(args, cwd=workdir).decode().strip(), q=quote(base_term, safe=''))

    # req_uri = base_url + langs + mixin
    # cookie = requests.cookies.cookiejar_from_dict(
    #     {
    #         'NID': '189=hyn8SCgMxfatpjS6oPL79jJKWWPsqPFQ8IHY6buD-WLae22p-Pz9ob2dNZBjKwm-bPlyEDPk1P3l5AOsaQp74vgDTxydn62KLX_CuMJsJtmoDlz5XPKLvyozavWAVihewZIAVCv014jfMIIV94cGDYHFh8macFQ9ub0Ov-e0-tuzlzZ76e2CVgn9RH3IdBDSE6dX',
    #         '_ga': 'GA1.3.934827190.1550692951'}
    #     )

    # req = requests.Request('GET', req_uri, cookies=cookie, headers=unneed_headers)
    # prp_req = req.prepare()
    # resp = session_google.send(prp_req, verify=True, timeout=4)
    # resp = requests.get(req_uri, verify=True, timeout=4, headers=unneed_headers, cookies=cookie)


    result = resp.text
    result = re.sub(",,+", ",", result)
    result = re.sub("\\[,", "[", result)
    result = re.sub(",]", "]", result)
    obj = json.loads(result)
    translations = []

    def filter_join_list(tr):
        if isinstance(tr, int):
            assert(0)
        tr = [x for x in tr if x[:2] != "e1"]
        return ", ".join(tr)


    def check_append_if_unique(tr, dst):
        if not isinstance(tr, str):
            assert(0)
            return False
        if tr[:2] == "e1":
            return False
        tr = tr.strip()
        return (tr not in dst)

    def append_if_unique_onespace(tr, dst):
        if check_append_if_unique(tr, dst):
            dst.append("<br/>")
            dst.append(tr)

    def append_if_unique(tr, dst):
        if check_append_if_unique(tr, dst):
            dst.append(tr)

    def append_if_unique_list(tr_list, dst):
        for tr in tr_list:
            append_if_unique(tr, dst)

    def parse_0_sentence(item):
        assert(isinstance(item, list))
        dst = []
        if len(item) > 1:
            item = item[:-1]
        for l in item:
            append_if_unique(l[0], dst)
        append_if_unique(" ".join(dst), translations)

    def parse_mid_sentence(item):
        dst_all = []
        for item_0 in item:
            if isinstance(item_0, str) or (isinstance(item_0, list) and len(item_0) < 3) or not item_0:
                continue

            if isinstance(item_0[0], str):
                if len(item_0) == 1:
                    append_if_unique_onespace(item_0[0], translations)
                else:

                    if isinstance(item_0[2], list) and isinstance(item_0[2][0], list):
                        dst_item = []
                        for tr in item_0[2]:
                            append_if_unique(tr[0], dst_item)
                        if len(dst_item) > 0:
                            dst_all.append(dst_item)
                    else:
                        assert(0)

        lens = [len(x) for x in dst_all]
        if len(lens) > 0:
            l_max = max(lens)
            for l in dst_all:
                if len(l) < l_max:
                    l.extend([l[0]] * (l_max - len(l)))
            for tr_item in zip(*dst_all):
                tr = " ".join(tr_item)
                append_if_unique_onespace(tr, translations)

    def parse_0_word(item, item_5):
        tr = [item[0][0]]
        for alternative in item_5[0][2]:
            append_if_unique(alternative[0], tr)
        if len(item[1]) > 3:
            tr_pron = item[1][3]
            translations.append(", ".join(tr))
            translations.append("<h>произношение:</h> {0}".format(tr_pron))
        else:
            translations.append(", ".join(tr))

    def parse_1_word(item_list, item_m1, item_m2):
        for item in item_list:
            part_of_speech = item[0]
            tr = item[1]
            translations.append("<h>{0}:</h> {1}".format(part_of_speech, ", ".join(tr)))
            variants = item[2]
            for var in variants:
                tr = var[0]
                eng_vars = var[1]
                translations.append("{0} -> {1}".format(tr, ", ".join(eng_vars)))

        def check_str1(item1):
            return (isinstance(item1, list) and len(item1) > 0 and isinstance(item1[0], list) and
                    len(item1[0]) > 0 and isinstance(item1[0][0], str))

        if check_str1(item_m1):
            translations.append("<h>Словосочетания:</h> {0}".format(", ".join(item_m1[0])))

        if check_str1(item_m2):
            translations.append("<h>Варианты использования</h>:")
            for tr in item_m2:
                translations.append(tr[0])

    is_sentence = len(obj) > 1 and not isinstance(obj[1], list)

    # lang_str = None
    if is_sentence: # это предложение
        parse_0_sentence(obj[0])
        if len(obj) > 1:
            # lang_str = obj[-1]
            obj = obj[1:-1]
        else:
            obj = []

        for i, item in enumerate(obj):
            if not isinstance(item, list) or item[0] == from_lang:
                continue

            parse_mid_sentence(item)
    else:       # это перевод слова
        parse_0_word(obj[0], obj[5])
        m1 = obj[-1]
        m2 = obj[-2]
        if m2 is not None:
            m2 = m2[0]
        parse_1_word(obj[1], m1, m2)

    return translations, is_sentence


def test():
    s = requests.session()
    print(tr_google_fast("avs ee sss rr vv", 'en', 'ru', s))
    print(tr_google_fast("Optimizations of the LP-inference method", 'en', 'ru', s))
    print(tr_google_fast("Задача будет снята с планирования", 'ru', 'en', s))
    print(tr_google_fast("Hello ; '''W'", 'en', 'ru', s))


if __name__ == '__main__':
    test()
