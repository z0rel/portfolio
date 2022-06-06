#!/usr/bin/env python3


"""
Usage:

python exec(open("/home/artem/devs/linthreads/misc/scheduler/scripts/gdbhelper/schedtracer.py").read())
c

or

source /home/artem/devs/linthreads/misc/scheduler/scripts/gdbhelper/schedtracer.py
c
"""

import gdb
import shelve
import datetime
import skiplist_node


class Logger:
    def __init__(self, fname='/tmp/linschedtrace.log'):
        self.fname = fname
        with open(self.fname, "w"):
            pass

    def info(self, *text):
        with open(self.fname, "ab") as f:
            # f.write(str(datetime.datetime.now()) + ": ")
            if len(text) == 1 and isinstance(text[0], str):
                f.write(text[0])
            else:
                f.write(str(text))
            f.write("\n")

filternames = { "lc", "None" }
filteraddr = { "0x7", "0x3", "0x1", "0x2", "0x0" }


skipnextprev = {"next", "prev", "key"}


def get_strnames(value):
    return sorted((str(f.name) for f in value.type.fields()))


def dump_skiplist_content(value, container, is_head=False):
    val = value["value"]
    if str(val) != "0x0":
        val = val.dereference()["tid"]
    else:
        val = None

    node = skiplist_node.skiplist_node(
        keyval=str(value["key"]),
        address=str(value.address),
        level=str(value["level"]),
        value=str(val),
        prev_addrs=[i for i in [str(value["prev"][i]) for i in range(0, 8)] if i != "0x0"],
        next_addrs=[i for i in [str(value["next"][i]) for i in range(0, 8)] if i != "0x0"],
        container=container,
        is_head=is_head
    )
    if node.address in container:
        return
    container[node.address] = node
    for name in ["prev", "next"]:
        for i in range(0, 8):
            ref = value[name][i]
            addr = int(str(ref), 16)
            if not addr:
                continue
            elif addr < 10:
                logger.info("bad address:{0} i:{1} ptr:{2} {3}".format(addr, i, name, str(node)))
            else:
                dump_skiplist_content(ref.dereference(), container)


def dump_skiplist(value):
    nodes = {}
    dump_skiplist_content(value, nodes, True)
    return nodes


def pyvalue(value, name, handled_objects, frame, parents):
    valtype = value.type
    strvaltype = str(valtype)
    saddr = str(value.address)

    def retdef():
        return skiplist_node.py_value(name, strvaltype, saddr, str(value), attributes={})

    if saddr in filteraddr or strvaltype == "void *":
        return retdef()

    if strvaltype == "struct sched_skiplist_node":
        return skiplist_node.py_value(name, strvaltype, saddr, value=dump_skiplist(value), attributes={})

    if saddr in handled_objects:
        return retdef()

    handled_objects.add(saddr)

    try:
        if valtype.code == gdb.TYPE_CODE_PTR:
            svaladdr = str(value)
            if svaladdr in filteraddr:
                return retdef()
            value = value.dereference()
            valtype = value.type

    except gdb.error as err:
        print(str(err))
        print("  ", name, strvaltype, saddr, parents, frame)
        logger.info("Exception: {0}".format(str(err)))
        logger.info("    {0} {1} {2} {3} {4}".format(name, strvaltype, saddr, parents, frame))
        return skiplist_node.py_value(name, strvaltype, saddr, value=hex(value), attributes={})

    if valtype.code == gdb.TYPE_CODE_STRUCT:
        strnames = get_strnames(value)
        ch_parents = parents + [name]
        names = {f: pyvalue(value[f], f, handled_objects, frame, ch_parents)
                 for f in strnames if f not in filternames}
        return skiplist_node.py_value(name, strvaltype, saddr, value=None, attributes=names)
    else:
        return skiplist_node.py_value(name, strvaltype, saddr, value=str(value), attributes={})


def tracer():
    frames = []
    frame = gdb.selected_frame()
    wt_num = None

    global logger
    logger = Logger(fname="/tmp/linschedtrace/wt{0}.log".format(str(1)))  # wt_num)))

    i = 0
    while frame:
        i += 1
        if not wt_num:
            try:
                blocks = frame.block()
            except RuntimeError:
                break
            for symbol in blocks:
                if symbol.print_name == 'self':
                    wt_num = int(symbol.value(frame)['cpu_num'])
        frames.append(frame)
        frame = frame.older()

    out_frames = []
    for frame in reversed(frames):
        symtab = frame.find_sal()
        if symtab.symtab is None:
            continue

        frame_items = {}
        funname = str(frame.function())
        for symbol in frame.block():
            value = symbol.value(frame)
            if symbol.print_name not in filternames:
                handled_objects = set()
                name = str(symbol.print_name)
                frame_items[name] = pyvalue(value, name, handled_objects, funname, [name])

        frame_head = skiplist_node.stack_head(str(symtab.symtab.filename),
                                              str(symtab.line), funname, frame_items)
        out_frames.append(frame_head)

    appended_data = (datetime.datetime.now(), out_frames)
    with shelve.open('/tmp/linschedtrace.db') as debugdb:
        try:
            debugdb["frames"].append(appended_data)
        except KeyError:
            debugdb["frames"] = [appended_data]


tracer()
