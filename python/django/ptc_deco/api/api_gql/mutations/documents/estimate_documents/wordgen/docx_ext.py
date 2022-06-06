from docx.parts.story import BaseStoryPart
from docx.oxml.xmlchemy import BaseOxmlElement, OneAndOnlyOne

from docx.oxml import OxmlElement
from docx.oxml import ns
from docx.oxml.ns import qn
from docx.oxml.ns import nsdecls
from docx.oxml import parse_xml
from docx.opc.constants import RELATIONSHIP_TYPE
import os


ns.nsmap['w14'] = 'http://schemas.microsoft.com/office/word/2010/wordml'
with open(os.path.join(os.path.dirname(__file__), 'xml', 'anchor.xml'), 'r', encoding='utf-8') as f:
    XML_ANCHOR_IMAGE = f.read()


def set_cell_border(cell, **kwargs):
    """
    Set cell`s border
    Usage:

    set_cell_border(
        cell,
        top={'sz': 12, 'val': 'single', 'color': '#FF0000', 'space': '0'},
        bottom={'sz': 12, 'color': '#00FF00', 'val': 'single'},
        start={'sz': 24, 'val': 'dashed', 'shadow': 'true'},
        end={'sz': 12, 'val': 'dashed'},
    )
    """
    tc = cell._tc
    tc_pr = tc.get_or_add_tcPr()

    # check for tag existence, if none found, then create one
    tc_borders = tc_pr.first_child_found_in('w:tcBorders')
    if tc_borders is None:
        tc_borders = OxmlElement('w:tcBorders')
        tc_pr.append(tc_borders)

    # list over all available tags
    for edge in ('start', 'top', 'end', 'bottom', 'insideH', 'insideV'):
        edge_data = kwargs.get(edge)
        if edge_data:
            tag = 'w:{}'.format(edge)

            # check for tag existence, if none found, then create one
            element = tc_borders.find(qn(tag))
            if element is None:
                element = OxmlElement(tag)
                tc_borders.append(element)

            # looks like order of attributes is important
            for key in ['sz', 'val', 'color', 'space', 'shadow']:
                if key in edge_data:
                    element.set(qn('w:{}'.format(key)), str(edge_data[key]))


def set_repeat_table_header(row):
    """set repeat table row on every new page"""
    tr = row._tr
    tr_pr = tr.get_or_add_trPr()
    tbl_header = OxmlElement('w:tblHeader')
    tbl_header.set(qn('w:val'), 'true')
    tr_pr.append(tbl_header)
    return row


def lining_elem():
    element = OxmlElement('w14:numForm')
    element.set(qn('w14:val'), 'lining')
    return element


def shading_elem(cell_color):
    return parse_xml(r'<w:shd {0} w:fill="{1}"/>'.format(nsdecls('w'), cell_color))


def add_hyperlink(paragraph, url, text, style):
    """
    :param paragraph: The paragraph we are adding the hyperlink to.
    :param url: A string containing the required url
    :param text: The text displayed for the url
    :param style стиль добавляемой ссылки
    :return: The hyperlink object
    """

    # This gets access to the document.xml.rels file and gets a new relation id value
    part = paragraph.part
    r_id = part.relate_to(url, RELATIONSHIP_TYPE.HYPERLINK, is_external=True)

    # Create the w:hyperlink tag and add needed values
    hyperlink = OxmlElement('w:hyperlink')
    hyperlink.set(
        qn('r:id'),
        r_id,
    )

    # Create a w:r element
    new_run = OxmlElement('w:r')

    # Create a new w:rPr element
    r_pr = OxmlElement('w:rPr')

    # Join all the xml elements together add add the required text to the w:r element
    new_run.append(r_pr)
    new_run.text = text
    new_run.style = style
    hyperlink.append(new_run)

    paragraph._p.append(hyperlink)

    return hyperlink


class CTAnchor(BaseOxmlElement):
    """
    ``<w:anchor>`` element, container for an anchor shape.
    """

    extent = OneAndOnlyOne('wp:extent')
    docPr = OneAndOnlyOne('wp:docPr')
    graphic = OneAndOnlyOne('a:graphic')

    @classmethod
    def new(cls, cx, cy, shape_id, pic, posOffsetH, posOffsetV):
        """
        Return a new ``<wp:inline>`` element populated with the values passed
        as parameters.
        """
        anchor = parse_xml(cls._inline_xml(posOffsetH, posOffsetV))
        anchor.extent.cx = cx
        anchor.extent.cy = cy
        anchor.docPr.id = shape_id
        anchor.docPr.name = 'Picture %d' % shape_id
        anchor.graphic.graphicData.uri = 'http://schemas.openxmlformats.org/drawingml/2006/picture'
        anchor.graphic.graphicData._insert_pic(pic)
        return anchor

    @classmethod
    def new_pic_inline(cls, shape_id, rId, filename, cx, cy, pos_offset_h, pos_offset_v):
        """
        Return a new `wp:inline` element containing the `pic:pic` element
        specified by the argument values.
        """
        pic_id = 0  # Word doesn't seem to use this, but does not omit it
        anchor = parse_xml(
            cls._inline_xml(
                pos_offset_h,
                pos_offset_v,
                cx,
                cy,
                shape_id,
                'Picture %d' % shape_id,
                pic_id,
                filename,
                rId,
            )
        )

        # anchor = cls.new(cx, cy, shape_id, pic, posOffsetH, posOffsetV, pic_id, filename, rId)
        # anchor.graphic.graphicData._insert_pic(pic)
        return anchor

    @classmethod
    def _inline_xml(cls, pos_offset_h, pos_offset_v, cx, cy, shape_id, name, pic_id, fname, rId):
        return XML_ANCHOR_IMAGE.format(
            ns=nsdecls('wp', 'a', 'pic', 'r', 'wp14'),
            posOffsetH=pos_offset_h,
            posOffsetV=pos_offset_v,
            cx=cx,
            cy=cy,
            shape_id=shape_id,
            name=name,
            pic_id=pic_id,
            fname=fname,
            rId=rId,
        )


def new_pic_inline_anchor(part: BaseStoryPart, image_descriptor, width, height, posOffsetH, posOffsetV):
    """Return a newly-created `w:inline` element.

    The element contains the image specified by *image_descriptor* and is scaled
    based on the values of *width* and *height*.
    """
    r_id, image = part.get_or_add_image(image_descriptor)
    cx, cy = image.scaled_dimensions(width, height)
    shape_id, filename = part.next_id, image.filename

    return (
        CTAnchor.new_pic_inline(shape_id, r_id, filename, cx, cy, posOffsetH, posOffsetV),
        cx,
        cy,
    )


def add_picture_anchor(run, image_path_or_stream, pos_offset_h, pos_offset_v, width=None, height=None):
    """
    Return a newly appended ``CT_Drawing`` (``<w:drawing>``) child
    element having *inline_or_anchor* as its child.
    """

    inline, cx, cy = new_pic_inline_anchor(run.part, image_path_or_stream, width, height, pos_offset_h, pos_offset_v)

    run._r.add_drawing(inline)
    return cx, cy


def fix_broken_image_20_ids(document):
    """Исправить ошибку, из за которой более 20 элементов изображений и линий, добавленных в документ
    - ломают его структуру"""

    doc_element = document._part._element
    doc_prs = doc_element.findall('.//' + qn('wp:docPr'))
    for doc_pr in doc_prs:
        doc_pr.set('id', str(int(doc_pr.get('id')) + 100000))


def create_element(name):
    return OxmlElement(name)


def create_attribute(element, name, value):
    element.set(ns.qn(name), value)
