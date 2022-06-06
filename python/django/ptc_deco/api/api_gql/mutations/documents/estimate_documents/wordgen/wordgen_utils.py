import os
import re
from docx.oxml import ns
from docx.oxml.ns import nsdecls
from docx.enum.table import WD_CELL_VERTICAL_ALIGNMENT as WD_ALIGN_VERTICAL
import locale


CELL_COLOR = 'F9F9F9'
CELL_COLOR_DARK = 'F3F3F3'
BORDER_COLOR = '#AEAEAE'
RZP_AZ = re.compile('[A-Za-z]')
RU_CAP = re.compile('[А-Я-./0-9]')
RE_09 = re.compile('^[0-9] .*')


VTOP = WD_ALIGN_VERTICAL.TOP
VCENTER = WD_ALIGN_VERTICAL.CENTER

ns.nsmap['wp14'] = 'http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing'

XML_PARAM_FOOTER_LINE_LENGTH = '6160000'


def tr_descr(val):
    return val


with open(os.path.join(os.path.dirname(__file__), 'xml', 'word_line.xml'), 'r', encoding='utf-8') as f:
    XML_FOOTER_LINE_TEXT = f.read().format(
        w=(nsdecls('wp14') + ' ' + nsdecls('wp') + ' ' + nsdecls('w')),
        l=XML_PARAM_FOOTER_LINE_LENGTH,
    )


with open(os.path.join(os.path.dirname(__file__), 'xml', 'word_line_sect2.xml'), 'r', encoding='utf-8') as f:
    XML_FOOTER2_LINE_TEXT = f.read().format(
        w=(nsdecls('wp14') + ' ' + nsdecls('wp') + ' ' + nsdecls('w')),
        l=XML_PARAM_FOOTER_LINE_LENGTH,
    )


class WordGeneratorContext:
    def __init__(self, valut):
        self.VALUT = valut

    def tr_price(self, val):
        if not val:
            return '-'
        return f"{locale.format_string('%.2f', round(val, 2), grouping=True) if val is not None else ''} {self.VALUT}"


def round_to_min_zeros(val):
    v1000 = round(val * 1000)
    v0 = round(val)
    if v1000 / 1000 == v0:
        return '{0}'.format(v0)
    elif v1000 / 100 == round(val * 10):
        return '{0:.1f}'.format(val)
    elif v1000 / 10 == round(val * 100):
        return '{0:.2f}'.format(val)
    else:
        return '{0:.3f}'.format(val)


def round_to_min_zeros2(val):
    return locale.format_string('%.2f', round(val, 2), grouping=True) if val is not None else ''


def format_itog_price_with_triple_spaces(x):
    """ Отформатровать цену с пробельными разделителями """
    x = int(round(x))
    return '{0:,}'.format(x).replace(',', ' ')


def unquote(text):
    text = text.strip()
    if len(text) and text[0] == '"':
        text = text[1:]
    if len(text) and text[-1] == '"':
        text = text[:-1]
    return text


def put_text_with_end_dot(val):
    res = str(val)
    if res and res[-1] != '.':
        res = res + '.'
    return res


if os.name == 'nt':
    import win32com.client as client
    import pythoncom

    pythoncom.CoInitialize()


def convert_to_pdf_win(filepath: str):
    """Save a pdf of a docx file."""
    word = None
    try:
        word = client.DispatchEx('Word.Application')
        target_path = filepath.replace('.docx', r'.pdf')
        word_doc = word.Documents.Open(filepath)
        word_doc.SaveAs(target_path, FileFormat=17)
        word_doc.Close()
    except Exception as e:
        print(e)
        # raise e
    finally:
        if word:
            word.Quit()
