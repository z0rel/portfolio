from .docx_ext import create_element, create_attribute


def add_page_number(run):
    fld_char1 = create_element('w:fldChar')
    create_attribute(fld_char1, 'w:fldCharType', 'begin')

    instr_text = create_element('w:instrText')
    create_attribute(instr_text, 'xml:space', 'preserve')
    instr_text.text = 'PAGE'

    fld_char2 = create_element('w:fldChar')
    create_attribute(fld_char2, 'w:fldCharType', 'end')

    run._r.append(fld_char1)
    run._r.append(instr_text)
    run._r.append(fld_char2)
