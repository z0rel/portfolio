#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import pygtk
pygtk.require('2.0')

import gtk
import gobject
import sys
import re


class GetSelection(object):
    # Signal handler called when the selections owner returns the data
    unicode_literal = re.compile(r".*(\\u[0-9]{2,3}[fdcb0-9]){1,2}.*", re.DOTALL)
    unicode_literal1 = re.compile(r".*(\\u[0-9]{2,3}[0-9]){1,2}.*", re.DOTALL)

    def selection_received(self, widget, selection_data, data):
        # Make sure we got the data in the expected form
        if str(selection_data.type) == '':
            return False

        if str(selection_data.type) == "STRING":
            # Print out the string we received

            # text = selection_data.get_text()
            text = selection_data.data

            if self.savedText != text:
                self.savedText = text
                enc_text = text

                if GetSelection.unicode_literal.match(text):
                    enc_text = text.decode('unicode_escape')

                while True:
                    m = GetSelection.unicode_literal1.match(enc_text)
                    if m:
                        enc_text = enc_text[0:m.start(1)] + m.group(1).decode('unicode_escape').encode('utf-8') + enc_text[m.end(1):-1]
                    else:
                        break

                print(enc_text)

                print("vvvvvvvv")
                sys.stdout.flush()
                sys.stdout.flush()

        return False

    def timeoutCallback(self):
        self.inv.selection_convert("PRIMARY", "STRING")
        return True

    def __init__(self):
        self.savedText = ""

        # Create the toplevel window
        inv = gtk.Invisible()
        inv.connect("destroy", lambda w: gtk.main_quit())
        inv.connect("selection_received", self.selection_received)
        self.inv = inv

        gobject.timeout_add(1000, self.timeoutCallback)
        inv.show()
        return


def main():
    GetSelection()
    gtk.main()
    return 0


if __name__ == "__main__":
    main()
