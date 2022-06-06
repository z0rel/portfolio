#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import time
import mergetr
import os
import traceback
import translate_web
from tr_shared import WorkflowConfig, ThreadScanner, Manager, prepareTerm, logger
import urwid.curses_display
import urwid.raw_display
import urwid.web_display
import urwid


try:
    import cld2
except ImportError:
    import pycld2 as cld2


if urwid.web_display.is_web_request():
    Screen = urwid.web_display.Screen
else:
    # if len(sys.argv) > 1 and sys.argv[1][:1] == "r":
    #     Screen = urwid.raw_display.Screen
    # else:
    # Screen = urwid.raw_display.Screen
    Screen = urwid.curses_display.Screen


logger.init()
logger.print_to_stdout = True
logger.start_threading_fwrite()


_COLOR = {'green': "\x1b[32;01m",
          'red': "\x1b[31;01m",
          'yellow': "\x1b[33;01m",
          'light yellow': "\x1b[93m",
          'blue': "\x1b[34;01m",
          'light blue': "\x1b[94;01m",
          'light cyan': "\x1b[96;01m",
          'cyan': "\x1b[36m",
          'magenta': "\x1b[35;01m",
          'bold': "\x1b[1m",
          'reset': "\x1b[0m"
          }


def red_str(text):
    return _COLOR['red'] + text + _COLOR['reset']


def green_str(text):
    return _COLOR['green'] + text + _COLOR['reset']


def magenta_str(text):
    return _COLOR['magenta'] + text + _COLOR['reset']


def light_yellow_str(text):
    return _COLOR['light yellow'] + text + _COLOR['reset']


def yellow_str(text):
    return _COLOR['yellow'] + text + _COLOR['reset']


def blue_str(text):
    return _COLOR['blue'] + text + _COLOR['reset']


def blue_str_urwid(text):
    return (urwid.AttrSpec('dark blue', 'default'), text)


def light_blue_str(text):
    return _COLOR['light blue'] + text + _COLOR['reset']


def light_cyan_str(text):
    return _COLOR['cyan'] + text + _COLOR['reset']


def cyan_str(text):
    return _COLOR['cyan'] + text + _COLOR['reset']


def cyan_str_urwid(text):
    return (urwid.AttrSpec('dark cyan', 'default'), text)


def bold_str(text):
    return _COLOR['bold'] + text + _COLOR['reset']


def bold_str_urwid(text):
    return (urwid.AttrSpec('bold', 'default'), text)


def split_by_tokens(start, end, text):
    tx = text.split(start)
    for i, t in enumerate(tx):
        tx[i] = t.split(end)
    res = []
    for t in tx:
        if isinstance(t, list):
            res.extend(t)
        else:
            res.append(t)
    tx = res
    res = []
    for i, t in enumerate(tx):
        if i % 2:
            res.append(start + t + end)
        else:
            res.append(t)
    return res


def split_by_tokens_l(start, end, text):
    res = []
    for t in text:
        r = split_by_tokens(start, end, t)
        res.extend(r)
    return res


def change_items(l, match_conf):
    res = []
    for i in l:
        for m, r in match_conf:
            mr = m.match(i)
            if mr:
                t = mr.group(1)
                res.append(r(t))
                break
        else:
            res.append(i)
    return res


def translate_base_term(base_term, wrk):
    try:
        rows, columns = os.popen('stty size', 'r').read().split()
        columns = int(columns)
    except ValueError:
        columns = 80
    lf = "\n"

    out = []
    out.append("█" * columns + lf)

    base_term = prepareTerm(base_term, wrk.args.strip_paragraph)
    out += base_term + lf

    if not wrk.args.from_lang or not len(wrk.args.from_lang):
        try:
            lang = cld2.detect(base_term, isPlainText=True)[2][0][1]
        except cld2.error:
            lang = 'en'
    else:
        lang = wrk.args.from_lang.strip()

    if lang == 'ru':
        trSpec = ('ru', 'en')
    else:
        trSpec = ('en', 'ru')

    tr, is_sentence = translate_web.getTranslations(base_term, *trSpec)
    if not tr:
        return out

    lines = []

    def transform_to_print(tx):
        res1 = split_by_tokens_l("<h>", "</h>", split_by_tokens("<b>", "</b>", tx))
        res1 = change_items(res1, [(re.compile(r"<b>(.*?)</b>"), bold_str_urwid), (re.compile(r"<h>(.*?)</h>"), cyan_str_urwid)])
        return res1

    for head, translation in tr:
        if not translation:
            continue
        translation = [i for i in translation if i]
        if translation and len("".join(translation)) > 0:
            out.append(blue_str_urwid("  {0}".format(head)))
            out.append(lf)
            text = "\n".join(translation).strip()

            out.extend(transform_to_print(text))
            out.append(lf)
            text = translation[0]
            if head == "MyMemory":
                text = text[2:]
            lines.append(text)

    if is_sentence:
        mergedCases = mergetr.merge_main(lines)
        out.append("  == MERGED == " + lf)
        out.append(mergedCases[0] + lf)
        out.append("  == MERGED == " + lf)
        out.append(mergedCases[1] + lf)

    return out


class TrCui:
    def __init__(self, wrk):
        self.wrk = wrk
        self.header_fmt = "Translate sentences. Press Q to exit. Buffer scanning: {0}."

        self.screen = Screen()
        self.screen.set_input_timeouts(0.1)
        self.header = urwid.Text(self.header_fmt.format("ENABLED"))

        self.header_wrapped = urwid.AttrWrap(self.header, 'header')
        self.lw = urwid.SimpleListWalker([])
        self.listbox = urwid.ListBox(self.lw)
        self.listbox = urwid.AttrWrap(self.listbox, 'listbox')
        self.top = urwid.Frame(self.listbox, self.header_wrapped)
        self.mgr = Manager()

        self.loop = urwid.MainLoop(self.top, [
                ('header', 'black', 'dark cyan', 'standout'),
                ('key', 'yellow', 'dark blue', 'bold'),
                ('listbox', 'light gray', 'black'),
            ], self.screen, input_filter=self.input_filter  # , event_loop=urwid.SelectEventLoop()
        )

    def redraw_screen(self):
        self.loop.draw_screen()

    def input_filter(self, keys, raw):
        if 'q' in keys or 'Q' in keys:
            with self.mgr.lock:
                self.mgr.exited = True
            raise urwid.ExitMainLoop

        for c in ['s', 'ы']:
            if c in keys:
                with self.mgr.lock:
                    self.mgr.stop_waiting = not self.mgr.stop_waiting
                self.header.set_text(self.header_fmt.format("ENABLED" if not self.mgr.stop_waiting else "DISABLED"))
                if not self.mgr.stop_waiting and self.mgr.prev_term:
                    self.translate_and_redraw(self.mgr.prev_term)

        return keys

    def translate_and_redraw(self, base_term):
        try:
            text = translate_base_term(base_term, self.wrk)
        except Exception:
            text = traceback.format_exc()

        rawt = urwid.Text(text)
        self.lw.insert(0, rawt)
        self.listbox.set_focus_valign('top')
        time.sleep(0.1)
        if self.mgr.exited:
            return
        self.redraw_screen()


def main():
    wrk = WorkflowConfig()

    if wrk.needTranslation():
        translate_base_term(wrk.args.translate, wrk)
    else:
        try:
            tr_cui = TrCui(wrk)
            try:
                old = tr_cui.screen.tty_signal_keys('undefined', 'undefined', 'undefined', 'undefined', 'undefined')
                t1 = ThreadScanner(tr_cui)
                tr_cui.loop.run()
            finally:
                tr_cui.screen.tty_signal_keys(*old)
                t1.cleanup()
        except KeyboardInterrupt:
            pass


if __name__ == '__main__':
    main()
