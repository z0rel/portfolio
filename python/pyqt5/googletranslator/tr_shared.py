
import sys
import argparse
import threading
import subprocess
import re
import time
import io
import logging
import clipboard
from queue import Queue, Empty


class WorkflowConfig:
    def __init__(self):
        argp = argparse.ArgumentParser(description="Переводчик")
        argp.add_argument('--translate', action='store', help='Перевести аргумент')
        argp.add_argument('--from_lang', action='store', help='язык источник')
        argp.add_argument('--strip_paragraph', action='store_true', help='удалять абзацы')
        self.args = argp.parse_args()

    def needTranslation(self):
        if self.args.translate and isinstance(self.args.translate, str) and len(self.args.translate.strip()):
            self.args.translate = self.args.translate.strip()
            return True


def enqueue_output(out, queue):
    for line in iter(out.readline, b''):
        queue.put(line)
    out.close()


def thread_scanner(obj):
    obj.run()


class ThreadScanner:
    def __init__(self, ui, translate_and_redraw = None):
        super(ThreadScanner, self).__init__()
        ON_POSIX = 'posix' in sys.builtin_module_names
        # self.scan_proc = subprocess.Popen(["/usr/bin/python2.7", "gtkselection.py"],
        #                                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=1, close_fds=ON_POSIX)
        print("process opened")
        self.ui = ui
        # self.end_tok = re.compile(b"^vvvvvvvv$")
        self.translate_and_redraw = translate_and_redraw
        # self.q = Queue()
        self.is_alive = True
        self.exited = False
        # self.t = threading.Thread(target=enqueue_output, args=(self.scan_proc.stdout, self.q))
        # self.t.start()

    def cleanup(self):
        pass
        # self.scan_proc.kill()
        # print(self.scan_proc.wait(5))
        # self.scan_proc = None

    def __del__(self):
        pass
        # if self.scan_proc is not None:
        #     self.cleanup()

    def run(self):
        prevline = ""
        while True:
            with self.ui.mgr.lock:
                if self.ui.mgr.exited or not self.is_alive:
                    self.exited = True
                    return

            # self.scan_proc.poll()
            # if self.scan_proc.returncode:
            #     print('retcode', self.scan_proc.returncode)
            #     return

            try:
                 base_term = clipboard.paste()
                 if prevline == base_term:
                     continue
                 else:
                     prevline = base_term
            except Empty:
                continue

            # if not self.end_tok.match(line):
            #     base_term_list.append(line)
            # else:
            #     base_term = b"\n".join(base_term_list).decode()
            #     base_term_list = []
            if True:
                if self.translate_and_redraw:
                    self.translate_and_redraw(base_term)

            time.sleep(0.1)


class Manager:
    def __init__(self):
        self.stop_waiting = False
        self.exited = False
        self.prev_term = None
        self.lock = threading.Lock()


class Logger(object):
    def __init__(self):
        pass
        # self.init_common()

    def init_common(self):
        self.exited = False
        self.fname = "/tmp/vim-translate.log"
        self.print_to_stdout = False
        self.cb = None
        self.html_fmt = False
        self.term_suffix = None
        self.stream = io.StringIO()
        self.stream_pos = self.stream.tell()

        logging.basicConfig(level=logging.INFO, stream=self.stream,
                            # filename='/tmp/vim-translate.log', filemode='w',
                            format='%(asctime)s %(message)s',
                            datefmt='%m-%d %H:%M')
        self.logger = logging.getLogger('tr_shared')
        self.lock = threading.Lock()

    def init(self):
        if 'exited' not in self.__dict__:
            self.init_common()

    def set_term_suffix(self, s):
        self.term_suffix = s

    def info(self, s):
        if not self.html_fmt:
            self.logger.info(s)
        else:
            self.logger.info('</span><span id="logitem">' + s + '</span>')

    def start_threading_fwrite(self):
        self.thread = threading.Thread(target=self.threading_fwrite, daemon=True)
        self.thread.start()

    def start_threading_callback(self, cb):
        self.cb = cb
        self.thread = threading.Thread(target=self.threading_callback, daemon=True)
        self.thread.start()

    def read_new_lines(self):
        stream_pos = self.stream.tell()
        if self.stream_pos < stream_pos:
            self.stream.seek(self.stream_pos)
            self.stream_pos = stream_pos
            return self.stream.readlines()
        return []

    def threading_callback(self):
        self.cb(['========================= new session debug =========================\n'])
        while True:
            with self.lock:
                if self.exited:
                    return
                if self.term_suffix:
                    self.cb(['<b>' + self.term_suffix + '</b>'])
                    self.term_suffix = None
                newlines = self.read_new_lines()
                if len(newlines) > 0:
                    self.cb(newlines)

    def threading_fwrite(self):
        with open(self.fname, "w") as f:
            f.write('========================= new session debug =========================\n')
            while True:
                with self.lock:
                    if self.exited:
                        return
                    newlines = self.read_new_lines()
                    if len(newlines) > 0:
                        s = "\n".join(newlines)
                        f.write(s + "\n")
                        f.flush()
                        if self.print_to_stdout:
                            print(s)


def prepareTerm(base_term, strip_paragraph):
    base_term = base_term.strip()
    dst = []
    for x in base_term.split("\n"):
        x = x.strip().strip(' */')
        dst.append(x)

    if strip_paragraph:
        base_term = " ".join(dst)

    letter = "[a-zA-ZА-Яа-я]"
    base_term = re.sub("({0})-[ ]+ ({0})".format(letter), r"\1\2", base_term)
    return base_term


logger = Logger()
