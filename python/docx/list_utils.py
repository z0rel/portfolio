import re
from docx.enum.text import WD_BREAK

SUBRE = re.compile('^[0-9]+\\.\\s*')
SIGNS = {'.', ':', ';'}
RZP_AZ = re.compile('[A-Za-z]')
RU_CAP = re.compile('[А-Я-./0-9]')


def add_signs_dot(text_item):
    if text_item and text_item[-1] not in SIGNS:
        return text_item + '.'
    return text_item


def add_signs_semicolon(text_item):
    if text_item and text_item[-1] not in SIGNS:
        return text_item + ':'
    return text_item


def add_signs_dotcomma(text_item):
    if text_item and text_item[-1] not in SIGNS:
        return text_item + ';'
    return text_item


def update_text_item_with_semicolon(text_item, next_item):
    if not next_item:
        return add_signs_dot(text_item), False
    if next_item[:2] != '-\t':
        return add_signs_dot(text_item), True
    else:
        return add_signs_dotcomma(text_item), False


ISNUM = re.compile(r'^([0-9]+).*')


def make_text_item_with_tire(
    start_maker,
    text_item,
    cnt,
    i,
    arr_it,
    max_i,
    style_pt1,
    style_pt2,
    style_pt1_last,
    style_pt2_last,
):
    pt2_cnt1 = 11

    style = style_pt1 if cnt < pt2_cnt1 else style_pt2
    text_item, islast = update_text_item_with_semicolon(text_item, arr_it[i + 1] if i < max_i else None)
    ch = text_item[2]
    ch2 = text_item[3] if len(text_item) > 3 else False
    if not RZP_AZ.match(ch) and (not ch2 or not RU_CAP.match(ch2)):
        text_item = start_maker + ch.lower() + text_item[3:]
    text_item = '\t' + text_item

    if islast:
        style = style_pt1_last if cnt < pt2_cnt1 else style_pt2_last
    return style, text_item


def _add_run_with_styled_semicolon(paragraph, text, style_tire):
    arr = text.split(':')
    if arr:
        r = paragraph.add_run(arr[0])
        for x in arr[1:]:
            r = paragraph.add_run(':')
            r.style = style_tire
            r = paragraph.add_run(x)
    else:
        r = paragraph.add_run(text)
    return r


def add_run_with_styled_tire(paragraph, text, style_tire='Title_tkp_tire'):
    arr = text.split('-')
    if arr:
        _add_run_with_styled_semicolon(paragraph, arr[0], style_tire)
        for x in arr[1:]:
            r = paragraph.add_run('-')
            r.style = style_tire
            _add_run_with_styled_semicolon(paragraph, x, style_tire)
    else:
        _add_run_with_styled_semicolon(paragraph, text, style_tire)


def add_paragraph_breaked_line(paragraph, text, style_tire):
    arr = text.split('<BR>')
    r = _add_run_with_styled_semicolon(paragraph, arr[0], style_tire)
    for item in arr[1:]:
        r.add_break(WD_BREAK.LINE)
        r = _add_run_with_styled_semicolon(paragraph, item, style_tire)
    return r


def add_paragraph_with_styled_tire(document, text, style, style_tire='Title_tkp_tire'):
    arr = text.split('-')
    if arr:
        p = document.add_paragraph(style=style)
        add_paragraph_breaked_line(p, arr[0], style_tire)
        for x in arr[1:]:
            r = p.add_run('-')
            r.style = style_tire
            add_paragraph_breaked_line(p, x, style_tire)
    else:
        p = document.add_paragraph(style=style)
        add_paragraph_breaked_line(p, text, style_tire)
    return p


def get_level_indices(isnum, level2_idx, level3_idx):
    num = 0
    if isnum:
        num = int(isnum.group(1))
    if num == 0:
        level2_idx += 1
        level3_idx = 0

    if num == 1:
        level3_idx += 1
    return num, level2_idx, level3_idx, '' if isnum else '\t'
