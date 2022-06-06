#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import sys
import re
import chardet
from PyQt5 import QtWidgets, QtCore  # , QtGui
from PyQt5.QtCore import pyqtSlot, pyqtSignal  # , QObject
from ui_tr_window import Ui_Form
from tr_shared import WorkflowConfig, ThreadScanner, Manager, prepareTerm, logger, thread_scanner
import translate_web
import mergetr
import threading


# try:
#     import cld2
# except ImportError:
#     import pycld2 as cld2

# <body style=" font-family:'Noto Sans'; font-size:10pt; font-weight:400; font-style:normal;">

# div#source { }
# div#source { }
#        span#prmth { color: rgb(0,139,139); }

tmplt_start_fmt = """<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html><head><meta name="qrichtext" content="1" />
    <style type="text/css">
        span#prmth {
            color: rgb(86,13,13);
            font-weight: bold;
        }
        div#source {
            margin-top: 5px;
            background: rgb(234,239,244);
        }
        span#stranslator_pre {
            background: rgb(175,238,238);
            margin-right: 30px;
            text-indent: 20px;
        }
        div#translator {
            margin-top: 3px;
            text-indent: 10px;
        }
        div#lognode {
            font-family: "Consolas", "Consolas for Powerline", "Ubuntu Mono", "Courier New", Courier, monospace;
        }
        span#logitem {
            color: rgb(86,13,13);
        }
        body {
           font-size:<<fontsize>>pt;
           font-weight:400;
           font-style:normal;
        }
        p, li { white-space: pre-wrap; }
    </style>
</head>
<body>
<p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
"""

tmplt_end = "</body></html>"


def make_div_translator(text):
    return '<div id="translator"><span id="stranslator_pre"> </span> {0}</div>'.format(text)


def make_div_trblock(text):
    text = "".join(text)
    return '<div id="trblock">' + text + '</div>'


def translate_base_term(term_to_translate, from_lang, to_lang, lang_auto_detection, strip_paragraph):
    out = []

    term_to_translate = prepareTerm(term_to_translate, strip_paragraph)
    out += '<div id="source">    ' + term_to_translate + "</div>"

    if not lang_auto_detection:
        tr_spec = (from_lang, to_lang)
    else:
        try:
            # lang = cld2.detect(term_to_translate, isPlainText=True)[2][0][1]
            print(lang)
            if lang == 'ru':
                tr_spec = ('ru', 'en')
            else:  # cld2 тупой и определяет всё что угодно, только не английский
                tr_spec = ('en', 'ru')

        except cld2.error:
            tr_spec = ('en', 'ru')

    tr, is_sentence = translate_web.getTranslations(term_to_translate, *tr_spec)
    if not tr:
        return out

    lines = []

    def transform_to_print(tx):
        return re.sub(r"<h>(.*?)</h>", r'<span id="prmth">\1</span>', tx)

    for head, translation in tr:
        if not translation:
            continue
        translation = [i for i in translation if i]
        if translation and len("".join(translation)) > 0:
            out.append(make_div_translator(head))

            text = '<div id="trres">' + '</div>\n<div id="trres">'.join(translation).strip() + '</div>'

            out.extend(transform_to_print(text))

            text = translation[0]
            if head == "MyMemory":
                text = text[2:]
            lines.append(text)

    if is_sentence:
        mergedCases = mergetr.merge_main(lines)
        out.append(make_div_translator('MERGED'))
        out.append('<div id="trres">' + mergedCases[0] + '</div>')
        out.append(make_div_translator('MERGED'))
        out.append('<div id="trres">' + mergedCases[1] + '</div>')

    return out


class TranslateForm(QtWidgets.QWidget):
    append_text_signal = pyqtSignal(str)
    append_log_signal = pyqtSignal('PyQt_PyObject')

    def __init__(self):
        super(TranslateForm, self).__init__()
        # self.ui = uic.loadUi("tr_window.ui")
        self.ui = Ui_Form()
        self.ui.setupUi(self)
        self.ui.retranslateUi(self)
        self.cfg = WorkflowConfig()
        self.mgr = Manager()
        self.t1 = self.self_thread_scanner()
        self.append_text_signal.connect(self.tr_slot)
        self.append_log_signal.connect(self.add_log_node)
        self.ui.spinBox_textsize.valueChanged.connect(self.change_text_size)
        self.ui.comboBox_dir.currentIndexChanged[str].connect(self.direction_changed)
        self.ui.checkBox_scanning.stateChanged.connect(self.change_scanning_state)
        self.log_nodes = []
        self.current_term = ""
        self.prev_current_term = ""

        self.lang_auto_detection = False 
        self.from_lang = 'en'
        self.to_lang = 'ru'
        self.fontsize_pat = re.compile("(<<fontsize>>)")
        self.translation_history = []

        self.change_text_size(self.ui.spinBox_textsize.value())

        self.ui.textEdit_tr.setHtml(self.tmplt_start + tmplt_end)

        logger.init()
        logger.html_fmt = True
        logger.start_threading_callback(self.add_log_node_wrapper)

        # self.translate_and_redraw("lp inference optimization")
        # self.translate_and_redraw("going")
        self.t1_thr = self.start_thread_scanner()

        self.langpairs = {
                'en→ru': ('en', 'ru'),
                'ru→en': ('ru', 'en')
            }

    def start_thread_scanner(self):
        t = threading.Thread(target=thread_scanner, args=(self.t1,))
        t.start()
        return t


    def self_thread_scanner(self):
        return ThreadScanner(self, self.translate_and_redraw)

    def add_log_node_wrapper(self, term):
        self.append_log_signal.emit(term)

    def translate_and_redraw(self, node):
        self.append_text_signal.emit(node)

    def render_log(self):
        text = self.tmplt_start + "".join(self.log_nodes) + tmplt_end
        self.ui.textEdit_log.setHtml(text)

    def render_tr(self):
        if len(self.translation_history):
            new_text = self.translation_history[0]
        else:
            new_text = ""
        # text = "".join(self.translation_history)
        text = new_text + "\n".join(self.ui.textEdit_tr.toHtml().split("\n")[4:])
        # self.ui.textEdit_tr.setHtml(self.tmplt_start + text + tmplt_end)
        self.ui.textEdit_tr.setHtml(self.tmplt_start + text)

    @pyqtSlot(int)
    def change_scanning_state(self, state):
        with self.mgr.lock:
            stop_waiting = (state != int(QtCore.Qt.Checked))

            if stop_waiting:
                self.t1.is_alive = False
            else:
                self.t1 = self.self_thread_scanner()
                self.t1_thr = self.start_thread_scanner()

        if stop_waiting:
            self.t1.cleanup()
            while True:
                if not self.t1_thr.is_alive():
                    break

            del self.t1
            del self.t1_thr

            # if not self.mgr.stop_waiting and self.mgr.prev_term:
            #     self.tr_slot(self.mgr.prev_term)

    @pyqtSlot('PyQt_PyObject')
    def add_log_node(self, node):
        if self.prev_current_term != self.current_term:
            self.log_nodes.append('<div id="lognode"><b>CURRENT TERM:</b> ' + self.current_term + '</div>')
            self.prev_current_term = self.current_term
        for n in node:
            self.log_nodes.append('<div id="lognode"><span id="logtime">' + n + '</div>')
        self.render_log()

    @pyqtSlot(int)
    def change_text_size(self, size):
        self.tmplt_start = self.fontsize_pat.sub(str(size), tmplt_start_fmt)
        self.render_tr()
        self.render_log()

    @pyqtSlot(str)
    def direction_changed(self, direction):
        if direction == 'auto':
            self.lang_auto_detection = False 
            self.from_lang = 'en'
            self.to_lang = 'ru'
        else:
            langpair = self.langpairs[direction]
            self.lang_auto_detection = False
            self.from_lang = langpair[0]
            self.to_lang = langpair[1]

    @pyqtSlot(str)
    def tr_slot(self, term_to_translate):
        if self.current_term == term_to_translate:
            return

        w = make_div_trblock(translate_base_term(
            term_to_translate=term_to_translate,
            from_lang=self.from_lang,
            to_lang=self.to_lang,
            lang_auto_detection=self.lang_auto_detection,
            strip_paragraph=self.ui.checkBox_strip.isChecked()
        ))
        self.translation_history.insert(0, w)
        self.render_tr()

    def closeEvent(self, e):
        self.mgr.exited = True
        self.change_scanning_state(False)
        e.accept()


def main():
    app = QtWidgets.QApplication(sys.argv)
    wnd = TranslateForm()
    wnd.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
