#!/usr/bin/env python3
# coding=utf8

# Если в кадре встретилось T и M25 или M22 - вынести T в предшествующий отдельный кадр

import os
import math

from gettext import gettext as _
from .constants import configure_defaults
from .transform_misc import print_dist_f


from .lang_entities import (
    TransformContext,
    DisCommand,
    InternalRpt,
    InternalErp,
    Program,
    Cadr,
    XYJ, XYJ_SPEC,
    NumVal,
    SCmd
)

from .angles import to_polar


def transform_program(program, fname, print_fun, dstdir, context):
    tr_prg = Program()

    if not program.comment_empty:
        tr_prg.comment = program.comment_load()

    commands = preprocess_calls(program.commands, fname, context)
    if context.D.VERBOSE_DISTANTION:
        print("from:", context.fname, "to:", context.fname_tr)

    for i, cadr in enumerate(commands):
        context.update_cadr(cadr)
        tr_cadres = transform_cadr(context, cadr)
        if context.D.VERBOSE_DISTANTION:
            print_dist_f(cadr, context, print_fun)
        context.f_misc = context.f

        tr_prg.commands.extend(tr_cadres)

    postprocess_dist1(fname, tr_prg, print_fun, context)
    postprocess_dist2(fname, tr_prg, print_fun, context)
    postprocess_dist_M20(fname, tr_prg, print_fun, context)
    postprocess_percent(fname, tr_prg, print_fun, context)

    return tr_prg


def postprocess_percent(fname, tr_prg, print_fun, context):
    i = 0
    percent_cnt = 0
    for cadr in tr_prg.commands:
        if not cadr.has_number:
            continue
        percent = [c for c in cadr.body if c.cat == '%'] if cadr.body else []
        assert len(percent) < 2
        if percent:
            percent_cnt += 1
            i = 0
            cadr.num = None
        else:
            cadr.num = i
            i += 1

    fname_base = os.path.basename(fname)
    if len(tr_prg.commands) < 2:
        context.print(_("INTERNAL TRANSLATION ERROR {}: count of the command < 2").format(fname))

    tr_prg.commands.insert(1, Cadr(None, [SCmd('; (UCG,2,X0X600,Y0Y800)')], None))
    tr_prg.commands.insert(1, Cadr(None, [SCmd('; A' + fname_base.split('.')[0])], None))

    if percent_cnt > 1:
        context.print(_("ERROR: multiple ({}) percent in file {}").format(percent_cnt, fname))

    cmds = tr_prg.commands
    for i in range(1, len(tr_prg.commands)):
        c0 = cmds[-1].m_command
        c1 = cmds[-2].m_command
        c0 = c0[0] if c0 else None
        c1 = c1[0] if c1 else None
        if (c0 and c0.cat == 'M' and c0.val.ivalue == 30
            and c1 and c1.cat == 'M' and c1.val.ivalue == 30):
            del cmds[-1]
        else:
            break


def postprocess_dist1(fname, tr_prg, print_fun, context):
    prev_br_entry = ""
    t_prev = None
    for_del = []
    ACCEPTED_XYIJ = {"X", "Y", "I", "J"}
    ACCEPTED_G9091 = {90, 91}

    g0102_series = []
    g9091_without_xy_prev = []
    for i, cadr in enumerate(tr_prg.commands):
        for_del_items = []
        i_appended = False
        for j, item in enumerate(cadr.body):
            if item.cat == "DIS":
                if prev_br_entry == item.br_entry:
                    i_appended = True
                    for_del.append(i)
                else:
                    prev_br_entry = item.br_entry
                continue
            if item.cat == "T":
                if t_prev == item.val.ivalue:
                    for_del_items.append(j)
                else:
                    t_prev = item.val.ivalue
                continue

            if item.cat in ACCEPTED_XYIJ and g0102_series:
                if len(g0102_series) > 1:
                    g9091_without_xy_prev.append(g0102_series)
                g0102_series = []

            if item.cat == 'G' and item.val.ivalue in ACCEPTED_G9091:
                g0102_series.append((i, j, item))

        for j in reversed(for_del_items):
            del cadr.body[j]
        if not cadr.body and not cadr.m_command and not i_appended:
            for_del.append(i)

    for series in g9091_without_xy_prev:
        for i, j, item in series[:-1]:
            for_del.append(i)

    for i in reversed(sorted(set(for_del))):
        del tr_prg.commands[i]

    last_cadr = tr_prg.commands[-1]
    m30 = [it for it in last_cadr.m_command if it.cat == 'M' and it.val.ivalue == 30] if last_cadr.m_command else None

    if m30:
        if last_cadr.body:
            last_cadr.m_command = [it for it in last_cadr.m_command if it.cat == 'M' and it.val.ivalue != 30]
        tr_prg.commands.append(Cadr(last_cadr.num, [], m30))
    else:
        tr_prg.commands.append(Cadr(last_cadr.num, [], XYJ("M", NumVal(30))))


class FPrev:
    def __init__(self):
        self.f_prev = None
        self.f_prev_cadr = None
        self.f_prev_cadr_i = None
        self.f_prev_cadr_j = None
        self.f_prev_xy = None
        self.handled = False


def postprocess_dist2(fname, tr_prg, print_fun, context):
    for_del = []
    ACCEPTED_IJ = {"I", "J"}
    ACCEPTED_G9091 = {90, 91}
    ACCEPTED_G123 = {1, 2, 3}

    g_series = []
    g_all = []
    g_current = None
    g_current0203 = None
    f_prev_list = []
    for_del = []
    for i, cadr in enumerate(tr_prg.commands):
        for_del_items = []
        for j, item in enumerate(cadr.body):
            if item.cat in ACCEPTED_IJ:
                if g_current0203 != 2 and g_current0203 != 3:
                    if cadr.src_cadr:
                        print_fun(_("ERROR: cadr has I or J but previous G is not 2 or 3. prev G: {}, cadr {}:{} - {}").format(g_current0203, context.fname, cadr.src_cadr.lloc(), cadr.src_cadr.ltext(context.fname)))
                    else:
                        print_fun(_("ERROR: cadr has I or J but previous G is not 2 or 3. prev G: {}, file: {}, position: {}").format(g_current0203, context.fname, i))

            if item.cat == 'G':
                if item.val.ivalue in ACCEPTED_G9091:
                    if item.val.ivalue != g_current:
                        if len(g_series) > 1:
                            new_g_series = []
                            for ii, jj, iitem in g_series[1:]:
                                iicadr = tr_prg.commands[ii]
                                if len(iicadr.body) == 1:
                                    new_g_series.append((ii,jj,iitem))
                                else:
                                    del iicadr.body[jj]
                            if new_g_series:
                                g_all.append(new_g_series)
                        g_series = [(i, j, item)]
                        g_current = item.val.ivalue
                    else:
                        g_series.append((i, j, item))
                elif item.val.ivalue in ACCEPTED_G123:
                    g_current0203 = item.val.ivalue
            elif item.cat == 'F':
                if f_prev_list:
                    for fp in reversed(f_prev_list):
                        if fp.handled:
                            break
                        if fp.f_prev_cadr and len(fp.f_prev_cadr.body) == 1 and not fp.f_prev_xy:
                            for_del.append(fp.f_prev_cadr_i)
                            fp.handled = True
                fprev = FPrev()

                fprev.f_prev_xy = [item for item in cadr.body if item.cat in XY_CAT]
                fprev.f_prev_cadr = cadr
                fprev.f_prev_cadr_i = i
                fprev.f_prev_cadr_j = j
                fprev.f_prev = item
                f_prev_list.append(fprev)


            elif item.cat in XYIJ_CAT:
                f_curr = [item for item in cadr.body if item.cat == 'F']
                if (f_prev_list  # если есть ранее f
                    and len(f_prev_list[-1].f_prev_cadr.body) == 1  # одиночно стоящее в кадре
                    and not f_prev_list[-1].f_prev_xy          # и после него нет X Y I J
                    and not f_curr  # а в текущем кадре нет F, но есть X Y I J
                ):
                    cadr.body.append(f_prev_list[-1].f_prev)
                    for_del.append(f_prev_list[-1].f_prev_cadr_i)
                    f_prev_list[-1].f_prev_cadr = cadr
                    f_prev_list[-1].f_prev_cadr_i = i
                    f_prev_list[-1].f_prev_cadr_j = j
                    f_prev_list[-1].f_prev_xy = item

                if f_curr:
                    continue
                elif f_prev_list:
                    f_prev_list[-1].f_prev_xy = item


    for series in g_all:
        for i, j, item in series:
            for_del.append(i)

    for i in reversed(sorted(set(for_del))):
        del tr_prg.commands[i]

    XYIJ = {"X", "Y", "I", "J"}
    to_del = []
    for i, (cadr1, cadr2) in enumerate(zip(tr_prg.commands, tr_prg.commands[1:])):
        if len(cadr1.body) == 1 and cadr1.body[0].cat == 'G' and cadr1.body[0].val.ivalue in ACCEPTED_G9091:
            xyij = [item for item in cadr2.body if item.cat in XYIJ]
            g = [item for item in cadr2.body if item.cat in ACCEPTED_G9091]
            if not g and xyij:
                to_del.append(i)
                cadr2.body.insert(0, cadr1.body[0])

    for i in reversed(to_del):
        del tr_prg.commands[i]


def postprocess_dist_M20(fname, tr_prg, print_fun, context):
    body_item = []
    positions = []
    for_del_items = []
    for i, cadr in enumerate(tr_prg.commands):
        for j, item in enumerate(cadr.body):
            if item.cat == 'RPT':
                positions.append(i)
            elif item.cat == 'ERP':
                positions.append(i+1)

    positions = list(sorted(set(positions)))

    for_del_items = []
    for items in zip(positions, positions[1:]):
        prev_m = None
        for i in range(*items):
            cadr = tr_prg.commands[i]
            if cadr.m_command:
                for_del_m = []
                if prev_m:
                    for j, m in enumerate(cadr.m_command):
                        if prev_m.val.ivalue == 20 and m.val.ivalue == 20:
                            if not cadr.body and len(cadr.m_command) == 1:
                                for_del_items.append(i)
                            else:
                                if len(cadr.m_command) == 1:
                                    cadr.m_command = None
                                else:
                                    for_del_m.append(j)

                for m in cadr.m_command:
                    ival = m.val.ivalue
                    if ival == 20 or ival == 22 or ival == 25:
                        prev_m = m
                
                if for_del_m:
                    for k in reversed(sorted(set(for_del_m))):
                        del cadr.m_command[k]

    for_del_items = list(sorted(set(for_del_items)))
    for i in reversed(for_del_items):
        del tr_prg.commands[i]

class Subprogram:
    def __init__(self, subprogram):
        self.subprogram = subprogram
        self.last_args = {}


def preprocess_calls(commands, fname, context):
    l_cnt = 0
    percent_cnt = 0
    percent_pos = []

    prev_is_percent = False
    for i, cadr in enumerate(commands):
        for item in cadr.body:
            if item.cat == '%':
                percent_cnt += 1
                percent_pos.append((i, True))
                prev_is_percent = True
            elif item.cat == 'L':
                if percent_cnt < 2:
                    l_cnt += 1
                elif not prev_is_percent:
                    percent_pos.append((i, False))
                    if len(cadr.body) > 1:
                        context.print(_("ERROR: subprogram definition has other subprogram calls: file {}:{} - {}").format(fname, cadr.lloc(), cadr.ltext(fname)))
                prev_is_percent = False
            else:
                prev_is_percent = False

    if not percent_cnt:
        commands = [Cadr(NumVal("0"), [XYJ_SPEC('%', None)], None)] + commands

    if not l_cnt and percent_cnt < 2:
        return commands

    percent_pos.append((len(commands), True))
    splitted_programs = []
    for ((start, start_flag), (end, end_flag)) in zip(percent_pos, percent_pos[1:]):
        splitted_programs.append((start_flag, commands[start:end]))

    if len(splitted_programs) == 1:
        context.print(_("ERROR: file {} has {} subprogram calls, but these subprograms was not defined.").format(fname, l_cnt))
        return commands

    main_program = splitted_programs[0][1]
    subprograms_list = splitted_programs[1:]
    subprograms = {}
    for start_flag, prg in subprograms_list:
        offset_val = 0 if start_flag else 1

        if len(prg) < 2 - offset_val:
            context.print(_("ERROR: file {} has empty unnamed subprogram in {} - {}.").format(fname, prg[0].lloc(), prg[0].ltext(fname)))
            continue
        l_cmd = prg[1 - offset_val].body
        if len(l_cmd) != 1 or l_cmd[0].cat != 'L':
            context.print(_("ERROR: file {} has unknown L subprogram name definition {} - {}.").format(fname, prg[0].lloc(), prg[0].ltext(fname)))
            continue
        l_cmd_num = int(l_cmd[0].val.val)
        subprograms[l_cmd_num] = Subprogram(prg)

        if start_flag:
            del prg[0]  # delete percent
        del prg[0]  # delete lname

        cleanup_prg(prg, fname)

    preprocessed_main_program = []
    calls_cnt = 0
    args_cnt = 0
    for i, cadr in enumerate(main_program):
        l_item = None
        for item in cadr.body:
            if item.cat == 'L':
                l_item = item
                calls_cnt += 1
                break
        else:
            preprocessed_main_program.append(cadr)
            continue
        arguments = [s for s in cadr.body if s.cat == 'R']
        additional = [s for s in cadr.body if s.cat != 'L' and s.cat != 'R']

        comments = [Cadr(cadr.num, [DisCommand(item.comment)], None) for item in cadr.body if not item.comment_empty and item.comment]
        preprocessed_main_program.extend(comments)

        if additional:
            context.print(_("ERROR: file {} has unknown operands in L subprogram call {} - {}.").format(fname, cadr.lloc(), cadr.ltext(fname)))

        l_cmd_s   = l_item.val.val
        l_cmd_num = int(l_cmd_s[:2])
        l_cmd_repeats = l_cmd_s[3:]
        l_cmd_repeats = int(l_cmd_repeats) if l_cmd_repeats else 1
        parsed_arguments = {s.cat + s.val.val[:1]: s.val.val[1:] for s in arguments}
        if parsed_arguments:
            args_cnt += 1
        
        subprogram = subprograms.get(int(l_cmd_num), None)
        if parsed_arguments:
            subprogram.last_args.update(parsed_arguments)

        if not subprogram:
            context.print(_("ERROR: file {} has undefined reference to L{} subprogram in its call: {} - {}.").format(fname, l_item.val.val[:2], cadr.lloc(), cadr.ltext(fname)))
            preprocessed_main_program.append(cadr)
            continue

        subprogram_instance = [scadr.copy() for scadr in subprogram.subprogram]
        for scadr in subprogram_instance:
            for s_item in scadr.body:
                if s_item.cat == 'TR':
                    key = 'R' + s_item.val.val
                    arg = subprogram.last_args.get(key, None)
                    if not arg:
                        context.print(_("ERROR: file {} has undefined reference subprogramm call TR{} argument in L{} call. Call line: {} - {}; Subprogram line:  {} - {}").format(
                            fname, s_item.val.val, l_cmd_s[:2], cadr.lloc(), cadr.ltext(fname), scadr.lloc(), scadr.ltext(fname)))
                        continue
                    s_item.cat = 'T'
                    s_item.val.val = arg

        if l_cmd_repeats == 1:
            preprocessed_main_program.extend(subprogram_instance)
        else:
            preprocessed_main_program.append(Cadr(cadr.num, [InternalRpt(l_cmd_repeats)], None, True))
            preprocessed_main_program.extend(subprogram_instance)
            preprocessed_main_program.append(Cadr(cadr.num, [InternalErp()], None, True))
    if context.D.VERBOSE_LOG:
        context.print(_("Convert file {} with L calls with {} subprogramms/{} calls/{} arguments").format(
            fname, len(subprograms), calls_cnt, args_cnt))

    return preprocessed_main_program


def need_remove_cadr_item(item):
    if item.cat == 'M':
        val = int(item.val.val)
        if val == 17 or val == 30:
            return True
    return False


def cleanup_prg(prg, fname):
    empty_cadres = []

    for j, cadr in enumerate(prg):
        positions = [i for (i, item) in enumerate(cadr.body) if need_remove_cadr_item(item)]
        for i in reversed(positions):
            del cadr.body[i]
        if not len(cadr.body):
            empty_cadres.append(j)

    for i in reversed(empty_cadres):
        del prg[i]


NOT_M25_COMMAND = {22, 25, 17, 30}
ACCEPTED_G123 = {1, 2, 3}
ACCEPTED_G9091 = {90, 91}


def update_xy_defaults(context, g, s, f, m2225):
    context.gobj_prev = (g & ACCEPTED_G123) | context.gobj_prev
    context.gobj9091_prev = (g & ACCEPTED_G9091) | context.gobj9091_prev
    context.sobj_prev = context.sobj_prev | s
    if f:
        context.fobj_prev = f
    if m2225:
        context.m2225_prev = m2225


def preprocess_defaults(context, cadr, m_commands, body, s_cadr):
    x = [(pos, i) for (pos, i) in enumerate(body) if i.cat == 'X']
    y = [(pos, i) for (pos, i) in enumerate(body) if i.cat == 'Y']
    f = [(pos, i) for (pos, i) in enumerate(body) if i.cat == 'F']
    g = set(i.val.ivalue for i in body if i.cat == 'G')
    s = set(i.val.ivalue for i in s_cadr if i.cat == 'S')

    m2225 = [i for i in m_commands if (i.val.ivalue == 22 or i.val.ivalue == 25)]

    if not x and not y:
        update_xy_defaults(context, g, s, f, m2225)
        return

    if not context.yobj_prev and not y:
        y_o = XYJ("Y", NumVal("0"))
        x_pos = x[0][0]
        body.insert(x_pos+1, y_o)
        y = [(x_pos+1, y_o)]
    context.yobj_prev = y

    if not context.xobj_prev and not x:
        x_o = XYJ("X", NumVal("0"))
        y_pos = y[0][0]
        body.insert(y_pos, x_o)
        x = [(y_pos, x_o)]
    context.xobj_prev = x

    if not context.fobj_prev and not f:
        f_o = XYJ("F", NumVal("6000"))
        body.insert(len(body), f_o)
    context.fobj_prev = f

    if not context.gobj_prev and (not g or (not ACCEPTED_G123 & g)):
        body.insert(0, XYJ("G", NumVal("01")))
        context.gobj_prev.add(1)
    else:
        context.gobj_prev = (g & ACCEPTED_G123) | context.gobj_prev

    context.defaults_setted = True


def preprocess_defaults_s(context, cadr, prefix, body, s_cadr, m_commands):
    M2225 = {22, 25, 20}
    x = [(pos, i) for (pos, i) in enumerate(body) if i.cat == 'X']
    y = [(pos, i) for (pos, i) in enumerate(body) if i.cat == 'Y']
    s = set(i.val.ivalue for i in s_cadr)
    g = set(i.val.ivalue for i in body if i.cat == 'G')
    f = [(pos, i) for (pos, i) in enumerate(body) if i.cat == 'F']
    m2225 = [i for i in m_commands if i.val.ivalue in M2225] if m_commands else []

    if not x and not y:
        update_xy_defaults(context, g, s, f, m2225)
        return

    if not context.gobj9091_prev and (not g or (not ACCEPTED_G9091 & g)):
        body.insert(0, XYJ("G", NumVal("90")))
        context.gobj9091_prev.add(90)
    else:
        context.gobj9091_prev = (g & ACCEPTED_G9091) | context.gobj9091_prev

    if not context.sobj_prev and not s:
        y = [(pos, i) for (pos, i) in enumerate(body) if i.cat == 'Y']
        s_o = XYJ("S", NumVal("01"))
        body.insert(y[0][0]+1, s_o)
        s_cadr.append(s_o)
    else:
        context.sobj_prev = context.sobj_prev | s

    dflt_m25_setted = False

    if not context.m2225_prev and not m2225:
        m_o = XYJ("M", NumVal("25"))
        body.insert(len(body), m_o)
        m_commands.append(m_o)
        dflt_m25_setted = True
        context.m_command = 25
    context.m2225_prev = m2225

    context.defaults_s_setted = True
    return dflt_m25_setted


GCNT = 0


def transform_cadr(context, cadr):
    # try:
    #     if cadr.lloc().loc.begin.line == 20:
    #         i = 111
    #     print(cadr.lloc().loc.begin.line, cadr.ltext(context.fname))
    # except AttributeError:
    #     pass
    global GCNT
    GCNT += 1
    tr_cadr = Cadr()
    tr_cadr.num = cadr.num
    tr_cadr.has_number = cadr.has_number
    tr_cadr.src_cadr = cadr
    body = cadr.body
    suffix = []
    if not body:
        return [tr_cadr]
    dflt_m25_setted = False

    m_commands = [XYJ('M', NumVal(item.val.val)) for item in body if item.cat == 'M']
    for cmd in m_commands:
        if cmd.val.ivalue > 99:
            context.print(_("ERROR: unknown {} command in {}:{} - {}").format(cmd, context.fname, cadr.lloc(), cadr.ltext(context.fname)))

    s_cadr = [XYJ('S', NumVal(item.val.val)) for item in body if item.cat == 'S']
    comments = [Cadr(tr_cadr.num, [DisCommand(item.comment)], None) for item in body if not item.comment_empty and item.comment]

    body = [(XYJ(item.cat, NumVal(item.val.val)) if item.has_xyj_conversion else item) for item in body if item.cat != 'M']

    if not context.defaults_setted and (not context.xobj_prev or not context.yobj_prev):
        preprocess_defaults(context, cadr, m_commands, body, s_cadr)

    prefix = comments

    m_command = m_commands[0].val.ivalue if m_commands else None

    if GCNT == 16:
        i = 1

    if m_command is not None:
        # if context.m_command_prev is not None and context.m_command_prev == 25 and m_command != 20:
        #     print("ERROR: after M25 command not going the M20, but M{:02}, file {}:{} - {}".format(m_command,
        #           context.fname, cadr.lloc(), cadr.ltext(context.fname)))
        if m_command == 20:
            if (context.m_command is None or context.m_command == 25):
                context.next_m_command(None)
                if len(m_commands) > 1:
                    m_command = m_commands[1].val.ivalue
                    tr_cadr.m_command = m_commands[1:]  # оставить M00 в конце
                else:
                    m_command = None
                    tr_cadr.m_command = None  # M20 надо удалить если она пытается отменить M25 или нечего отменять (начало программы)
            else:
                t_cadr = [item for item in body if item.cat == 'T']
                if t_cadr:
                    # если текущее M20 и предыдущее не M25 - преобразовать текущее в N..M20 \n Кадр
                    prefix.append(Cadr(tr_cadr.num, [], [XYJ('M', NumVal("20"))]))
                    tr_cadr.m_command = None
                else:
                    tr_cadr.m_command = m_commands  # [1:] # оставить M00 в конце
                # FIX: Оставить как было
                pass
        else:
            if m_command and m_command not in NOT_M25_COMMAND and context.m_command and context.m_command == 25:
                context.print(_("ERROR: M{:02} command after M25 command without M20 in file {}:{} - {}").format(
                    m_command, context.fname, cadr.lloc(), cadr.ltext(context.fname)))

            tr_cadr.m_command = m_commands
            context.next_m_command(m_command)
    elif context.m_command is not None and context.m_command == 25:
        # если текущая M-команда не задана и предыдущая - M25 - подставить M25 в текущий кадр
        if not cadr.skip_m:
            cadr_xyij = [item for item in body if item.cat == 'X' or item.cat == 'Y' or item.cat == 'I' or item.cat == 'J']
            if cadr_xyij:
                tr_cadr.m_command = [XYJ('M', NumVal("25"))]
                m_command = 25

    if s_cadr:
        if tr_cadr.m_command:
            # S-команды в одном кадре с M-командами транслировать в NN G90 S<> \n остальное содержимое кадра M<>
            # S-команды без M-команд в кадре транслировать 1 в 1
            # Но - если встретилось S01 и предыдущее S = S01 - то текущее S01 удалить
            body = [item for item in body if item.cat != 'S']
            assert context.coordinates_absolute
            if len(s_cadr) > 1:
                context.print(_("ERROR: multiple S definitions: {} in file {}:{} - {}").format(
                    jl2s(s_cadr), context.fname, cadr.lloc(), cadr.ltext(context.fname)
                ))
            s_cadr = s_cadr[0]

            if (not (s_cadr.val.ivalue == 1 and context.s and context.s.val.ivalue == 1) and
                not (s_cadr.val.ivalue == 2 and context.s and context.s.val.ivalue == 2)):
                prefix.append(Cadr(tr_cadr.num, [XYJ('G', NumVal("90"))] + [s_cadr], None))
                context.gobj9091_prev.add(90)
                context.set_absolute(True)

            context.next_s_command(s_cadr)
        else:
            if len(s_cadr) > 1:
                context.print(_("ERROR: multiple S definitions: {} in file {}:{} - {}").format(
                    jl2s(s_cadr), context.fname, cadr.lloc(), cadr.ltext(context.fname)
                ))
            s_cadr = s_cadr[0]

            if ((s_cadr.val.ivalue == 1 and context.s and context.s.val.ivalue == 1) or
                (s_cadr.val.ivalue == 2 and context.s and context.s.val.ivalue == 2)):
                body = [item for item in body if item.cat != 'S']
                s_cadr = []

    if not context.defaults_s_setted:
        dflt_m25_setted = preprocess_defaults_s(context, cadr, prefix, body, [s_cadr] if s_cadr else [], m_commands)

    cadres = transform_stage2(context, cadr, tr_cadr, body)
    prefix.extend(cadres)
    cadres = prefix

    pos = []
    for i, _cadr in enumerate(cadres):
        for j, item in enumerate(_cadr.body):
            if item.cat == 'T' and ((_cadr.m_command and
                                     (_cadr.m_command[0].val.ivalue == 22
                                      or _cadr.m_command[0].val.ivalue == 25))
                                    or dflt_m25_setted):
                pos.append((i, j, _cadr, item))

    for i, j, _cadr, item in reversed(pos):
        cadres.insert(i, Cadr(tr_cadr.num, [item], None))
        del _cadr.body[j]

    return prefix + suffix


def jl2s(l):
    if l:
        return " ".join([str(io) for io in l])
    return None

XY_CAT = {'X', 'Y'}
XYIJ_CAT = {'X', 'Y', 'I', 'J'}

def transform_stage2(context, cadr, tr_cadr, body):
    g_cadr = [item.val.ivalue for item in body if item.cat == 'G']
    l_cadr = [item.val.ivalue for item in body if item.cat == 'L']
    if l_cadr:
        tr_cadr.body = body
        return [tr_cadr]

    # body = [XYJ(cmd.cat, NumVal(cmd.val.val)) for cmd in body]
    for item in body:
        item.transform(context)

    i = [cmd for cmd in body if cmd.cat == 'I']
    j = [cmd for cmd in body if cmd.cat == 'J']
    h = [cmd for cmd in body if cmd.cat == 'H']

    gval = g_cadr[0] if g_cadr else None
    if h and gval is None:
        if context.g010203 is None:
            context.print(_("ERROR: current command has H{}, but current and previous cadres has not G01, G02 or G03 in file {}:{} - {}").format(
                jl2s(h), context.fname, cadr.lloc(), cadr.ltext(context.fname)
            ))
        else:
            gval = context.g010203.val.ivalue

    if (i or j) and gval is None:
        gval = context.g010203.val.ivalue
        if gval == 1:
            context.print(_("ERROR: previous command is G01, but current cadr has I: {} or J{} in file {}:{} - {}").format(
                i, j, context.fname, cadr.lloc(), cadr.ltext(context.fname)
            ))

        if not gval:
            context.print(_("ERROR: i or j is setted, but G-command is not defined yet. i: {}, j: {}, file: {}:{} - {}").format(
                jl2s(i), jl2s(j), context.fname, cadr.lloc(), cadr.ltext(context.fname)
            ))
        else:
            body.insert(0, context.g010203.copy())

    suffix = []
    suffix = check_G02_G03_M22(context, cadr, tr_cadr, body, gval, i, j)
    result = transform_G02_G03_M25(context, cadr, tr_cadr, body, gval, i, j, h)
    if result:
        return result 

    result = transform_G01_M25(context, cadr, tr_cadr, body, gval, h)
    if result:
        return result

    need_last_f = [False]
    result = transform_ij_other_cases(context, cadr, tr_cadr, body, gval, i, j, need_last_f)
    if result:
        return result + suffix

    if (i or j) and gval is not None and gval != 2 and gval != 3:
        context.print(_("ERROR G{:02} command has ij, but conversion not supported yet. I: {}, J: {}, file: {}:{} - {}").format(
            gval, jl2s(i), jl2s(j), context.fname, cadr.lloc(), cadr.ltext(context.fname)
        ))

    if gval == 92:
        NOT_CAT = {"X", "Y", "G"}
        x = [item for item in body if item.cat == 'X']
        y = [item for item in body if item.cat == 'Y']
        last_body = [item for item in body if item.cat not in NOT_CAT]
        if last_body or not x or not y or len(x) > 1 or len(y) > 1 or len(x) > 1:
            context.print(_("ERROR: unsupported command in G92: {}, file: {}:{} - {}").format(
                jl2s(last_body), context.fname, cadr.lloc(), cadr.ltext(context.fname)))
        body = [SCmd("(UOT,1,{},{})".format(x[0], y[0]))]
        tr_cadr.body = body
        return [tr_cadr] + suffix

    tr_cadr.body = body

    if need_last_f[0]:
        return [tr_cadr, Cadr(tr_cadr.num, [XYJ('F', NumVal.num(context.f))])]

    dx = context.x - context.x_prev
    dy = context.y - context.y_prev
    d_len = int(round(math.sqrt(dx * dx + dy * dy)))

    if d_len and d_len < context.D.MIN_LEN_01 and context.f > context.D.MAX_F:
        f = [item for item in tr_cadr.body if item.cat == 'F']
        xy = [item for item in tr_cadr.body if item.cat in XY_CAT]
        
        if context.D.VERBOSE_G02_G03_M25:
            try:
                print(_("Converted F on small step {}: {} {}").format(context.fname, cadr.ltext(context.fname), context.fname_tr))
            except AttributeError:
                print(_("Converted F on small step {} {}").format(context.fname, context.fname_tr))
        if f:
            f[0].val.set_transformed(context.get_proportional_f(d_len))
        elif xy:
            tr_cadr.body.append(XYJ('F', NumVal.num(context.get_proportional_f(d_len)))) 

        return [tr_cadr, Cadr(tr_cadr.num, [XYJ('F', NumVal.num(context.f))])]

    return [tr_cadr] + suffix


def check_ij_error_1(i, j, gval, context, cadr):
    if len(i) > 1 or len(j) > 1:
        context.print(_("ERROR G{:02} command has multiple i or j. I: {}, J: {}, file: {}:{} - {}").format(
            gval, jl2s(i), jl2s(j), context.fname, cadr.lloc(), cadr.ltext(context.fname)))
        return True
    return False


def get_ij(i, j, gval, context, cadr, tr_cadr):
    if check_ij_error_1(i, j, gval, context, cadr):
        tr_cadr.error = True

    i_old = i
    j_old = j

    i = i[0] if i else (context.i.copy() if context.i else None)
    j = j[0] if j else (context.j.copy() if context.j else None)

    if not i:
        i = XYJ('I', NumVal("0"))
    if not j:
        j = XYJ('J', NumVal("0"))

    return (i, j, i_old, j_old)


def get_xyval(body, dflt=None):
    xval = [cmd.val.value for cmd in body if cmd.cat == 'X']
    yval = [cmd.val.value for cmd in body if cmd.cat == 'Y']
    xval = xval[0] if xval else dflt
    yval = yval[0] if yval else dflt
    return xval, yval


def get_xyij_curr_new(body, context, i, j, get_absolute=False):
    if not context.coordinates_absolute and not get_absolute:
        xval, yval = get_xyval(body)
        x_rel_prev = context.x_rel_prev if xval is not None else context.x_rel
        y_rel_prev = context.y_rel_prev if yval is not None else context.y_rel

        xval = xval if xval is not None else 0
        yval = yval if yval is not None else 0

        i_new = i.val.value + x_rel_prev
        j_new = j.val.value + y_rel_prev
        # context.print("WARNING: NOT stable relative IJ conversion in {}:{} - {}".format(
        #     context.fname, cadr.lloc(), cadr.ltext(context.fname)))
        x_curr = x_rel_prev
        y_curr = y_rel_prev
        x_next = xval + x_rel_prev
        y_next = yval + y_rel_prev
    else:
        xval, yval = get_xyval(body)
        x_prev = context.x_prev if xval is not None else context.x
        y_prev = context.y_prev if yval is not None else context.y

        i_new = i.val.value + x_prev
        j_new = j.val.value + y_prev
        x_curr = x_prev
        y_curr = y_prev

        if not context.coordinates_absolute:
            xval = (xval if xval is not None else 0) + x_prev
            yval = (yval if yval is not None else 0) + y_prev

        x_next = xval if xval is not None else x_prev
        y_next = yval if yval is not None else y_prev

    return x_curr, y_curr, x_next, y_next, i_new,  j_new


def transform_G02_G03_M25(context, cadr, tr_cadr, body, gval, i, j, h):
    if (gval == 2 or gval == 3) and tr_cadr.m_command and tr_cadr.m_command[0].val.ivalue == 25:
        if not h or h[0].val.ivalue == 0:
            context.print(_("ERROR: G{:02} M25 - h is empty. file: {}{}").format(gval, context.fname, str(cadr.lloc())))
            return None
        # if not context.coordinates_absolute:
        #     context.print(_("ERROR: G{:02} M25 {} - G91 relative coordinates is not supported. file: {}:{} - {}").format(
        #         gval, str(h[0]), context.fname, cadr.lloc(), cadr.ltext(context.fname)))
        if len(h) > 1:
            context.print(_("ERROR: G{:02} M25 - multiple times h {} declaration. file: {}:{} - {}").format(
                gval, jl2s(h), context.fname, cadr.lloc(), cadr.ltext(context.fname)))

        (i, j, i_old, j_old) = get_ij(i, j, gval, context, cadr, tr_cadr)

        h = h[0].val.ivalue

        # TODO: ситуация с пустым X или Y в G02 G03 может пока что не поддерживается

        x_curr, y_curr, x_next, y_next, i_new,  j_new = get_xyij_curr_new(body, context, i, j, True)

        new_x_curr = x_curr - i_new
        new_y_curr = y_curr - j_new

        (phi, ((r_curr, phi_curr), (r_next, phi_next))) = to_polar(x_curr, y_curr, x_next, y_next, i_new, j_new, gval, context)

        phi_step = phi / h
        phi_step = -phi_step if gval == 2 else phi_step

        arc_len = math.radians(abs(phi_step)) * math.sqrt(new_x_curr * new_x_curr + new_y_curr * new_y_curr) 

        commands = []
        if not context.coordinates_absolute:
            commands.extend([Cadr(tr_cadr.num, [XYJ("G", NumVal("90"))], None)])
        
        f_misc = context.get_proportional_f(arc_len)
        context.f_misc = f_misc
        commands.extend([
            # Для выполнения программы с "операндом Н" необходимо вначале ввести абсолютную начальную точку:
            Cadr(tr_cadr.num, [SCmd("(UAO,1)")], None, True),
            # вести временную начальную точку, которая сместит начало координат в центр дуги:
            Cadr(tr_cadr.num, [SCmd("  (UOT,1,X{0},Y{1})".format(NumVal.plus(i_new), NumVal.plus(j_new)))], None, True),
            # Обозначить начальное положение в координатах временной начальной точки:
            Cadr(tr_cadr.num, [SCmd("  G90 S02")], None),
            # определяем параметр "Е25":
            Cadr(tr_cadr.num, [SCmd("  E25=0")], None, True),
            Cadr(tr_cadr.num, [SCmd("  (RPT,{0})".format(h))], None, True),
            # для G02 (ПЧ) - прибавить угол, для G03 (ПрЧ) - вычесть угол
            Cadr(tr_cadr.num, [SCmd("    E25=E25{}".format(NumVal.plus(phi_step)))], None),
            Cadr(tr_cadr.num, [SCmd("    (URT,E25)")], None, True),
            Cadr(tr_cadr.num, [SCmd("    G01 X{0} Y{1} F{2} M25".format(NumVal.num(new_x_curr),
                                                                        NumVal.num(new_y_curr),
                                                                        f_misc))], None, True),  # (точки "1-2-3")
            Cadr(tr_cadr.num, [SCmd("  (ERP)")], None),
            Cadr(tr_cadr.num, [SCmd("(UAO,1)")], None),
            Cadr(tr_cadr.num, [SCmd("(URT,0)")], None),
            Cadr(tr_cadr.num, [XYJ('F', NumVal.num(context.f))])
        ])
        if not context.coordinates_absolute:
            commands.extend([Cadr(tr_cadr.num, [XYJ("G", NumVal("91"))], None)])
        if context.D.VERBOSE_G02_G03_M25:
            context.print(_("Converted G02-G03 M25 in file {}:{} - {}").format(context.fname, cadr.lloc(), cadr.ltext(context.fname)))
        return commands

    return None


def check_G02_G03_M22(context, cadr, tr_cadr, body, gval, i, j):
    if tr_cadr.m_command and tr_cadr.m_command[0].val.ivalue == 22:
        if (gval == 2 or gval == 3) and tr_cadr.m_command:
            body = [item for item in body if item.cat != 'G']

            if context.x_prev is None or context.y_prev is None:
                context.print(_("ERROR: previous x or y is not set in G{:02}. X prev: {}, Y prev: {}, file {}:{} - {}").format(
                    gval, context.x_prev, context.y_prev, context.fname, cadr.lloc(), cadr.ltext(context.fname)))
            elif ((context.coordinates_absolute and context.x_prev == context.x and context.y_prev == context.y) or
                  (not context.coordinates_absolute and context.x_rel_prev == context.x_rel and context.y_rel_prev == context.y_rel)):
                # пробивается полная окружность
                # G02 - по часовой стрелке; G03 - против часовой стрелки
                if context.D.VERBOSE_LOG:
                    context.print(_("Full circle in file {}:{} ").format(context.fname, cadr.lloc()))

            else:
                (ii, jj, i_old, j_old) = get_ij(i, j, gval, context, cadr, tr_cadr)
                # если X или Y еще не обновились, то prev будет находиться в x_rel, y_rel, x, y
                x_curr, y_curr, x_next, y_next, i_new,  j_new = get_xyij_curr_new(body, context, ii, jj)

                (phi, none_val) = to_polar(x_curr, y_curr, x_next, y_next, i_new, j_new, gval, context)
                if phi > context.D.ANGLE_TRESHOLD and context.D.PRINT_ANGLE_WARNS:
                    context.print(_("Not full circle, and angle > 180. angle: {}, file: {}:{} - {}").format(
                        NumVal.num(phi), context.fname, cadr.lloc(), cadr.ltext(context.fname)))

        f = [item for item in body if item.cat == 'F']
        dist = context.get_distance()
        new_f = None
        # if cadr.lloc().loc.begin.line == 28:
        #     print(1)
        if ((gval == 2 or gval == 3) and dist < context.D.MIN_LEN_02) or dist < context.D.MIN_LEN_01:
            if (gval == 2 or gval == 3):
                (ii, jj, i_old, j_old) = get_ij(i, j, gval, context, cadr, tr_cadr)
                x_curr, y_curr, x_next, y_next, i_new, j_new = get_xyij_curr_new(body, context, ii, jj, True)
                arc_len = get_arc_len(context, x_curr, y_curr, x_next, y_next, i_new, j_new, gval)
                new_f = context.get_proportional_f(arc_len)
            else:
                new_f = context.get_proportional_f(context.get_distance())
            
            if context.f > new_f:
                if f:
                    f[0].val.set_transformed(new_f)
                else:
                    tr_cadr.body.append(XYJ('F', NumVal.num(new_f)))
                context.f_misc = new_f

                return [Cadr(tr_cadr.num, [XYJ('F', NumVal.num(context.f))])]

    return []




G0203 = {2, 3}
G9091 = {90, 91}


def get_arc_len(context, x_curr, y_curr, x_next, y_next, i_new, j_new, gval):
    (phi, ((r_curr, phi_curr), (r_next, phi_next))) = to_polar(x_curr, y_curr, x_next, y_next, i_new, j_new, gval, context)
    new_x_curr = x_curr - i_new
    new_y_curr = y_curr - j_new

    arc_len = int(round(math.radians(abs(phi)) * math.sqrt(new_x_curr * new_x_curr + new_y_curr * new_y_curr)))
    return arc_len


def transform_ij_other_cases(context, cadr, tr_cadr, body, gval, i, j, need_last_f):
    if gval in G0203:
        (i, j, i_old, j_old) = get_ij(i, j, gval, context, cadr, tr_cadr)

        x_curr, y_curr, x_next, y_next, i_new, j_new = get_xyij_curr_new(body, context, i, j, True)

        i.val.set_transformed(i_new)
        j.val.set_transformed(j_new)
        arc_len = get_arc_len(context, x_curr, y_curr, x_next, y_next, i_new, j_new, gval)

        f = [item for item in body if item.cat == 'F']
        if arc_len and arc_len < context.D.MIN_LEN_02:
            new_f = context.get_proportional_f(arc_len)
            if context.f < new_f:
                if f:
                    f[0].val.set_transformed(new_f)
                need_last_f[0] = True
                context.f_misc = new_f

        if not context.coordinates_absolute:
            x = [item for item in body if item.cat == 'X']
            y = [item for item in body if item.cat == 'Y']
            for x_o in x:
                x_o.val.set_transformed(x_next)
            for y_o in y:
                y_o.val.set_transformed(y_next)
            new_body = [Cadr(tr_cadr.num, [XYJ("G", NumVal("90"))], None)]
            filtered_body = [item for item in body if (item.cat != 'G' or item.val.ivalue not in G9091)]

            if not f and need_last_f[0]:
                new_body.append(Cadr(tr_cadr.num, filtered_body + [XYJ('F', NumVal.num(new_f))], tr_cadr.m_command))
            else:
                new_body.append(Cadr(tr_cadr.num, filtered_body, tr_cadr.m_command))

            new_body.append(Cadr(tr_cadr.num, [XYJ("G", NumVal("91"))], None))

            if need_last_f[0]:
                new_body.append(Cadr(tr_cadr.num, [XYJ('F', NumVal.num(context.f))]))

            need_last_f[0] = False
            
            return new_body
        else:
            if not f and need_last_f[0]:
                body.append(XYJ('F', NumVal.num(new_f)))

    return None


def transform_G01_M25(context, cadr, tr_cadr, body, gval, h):
    if gval == 1 and tr_cadr.m_command and tr_cadr.m_command[0].val.ivalue == 25:
        body = [cmd for cmd in body if cmd.cat != 'H']

        if h and h[0].val.value == 0:
            context.print(_("Error: G01, H=0, file {}:{} - {}").format(context.fname, cadr.lloc(), cadr.ltext(context.fname)))

        def get_XY(delta_x, delta_y):
            XY = []
            if delta_x is not None and int(delta_x * 100) != 0:
                XY.append(XYJ("X", NumVal.num(delta_x)))
            if delta_y is not None and int(delta_y * 100) != 0:
                XY.append(XYJ("Y", NumVal.num(delta_y)))
            return XY

        if h:
            assert len(h) == 1
            h = h[0]
            xval, yval = get_xyval(body)

            if context.coordinates_absolute:
                delta_x = round((xval - context.x_prev) / h.val.value, 2) if xval else None
                delta_y = round((yval - context.y_prev) / h.val.value, 2) if yval else None
                XY = get_XY(delta_x, delta_y)
                delta_x = delta_x if delta_x else 0
                delta_y = delta_y if delta_y else 0
                xy_len = int(round(math.sqrt(delta_x * delta_x + delta_y * delta_y)))
                f_misc = context.get_proportional_f(xy_len)
                context.f_misc = f_misc
                result = [
                    Cadr(tr_cadr.num, [XYJ('F', NumVal.num(f_misc))]),
                    Cadr(tr_cadr.num, [InternalRpt(h.val.ivalue)], None, True),
                    Cadr(tr_cadr.num, [XYJ('G', NumVal("91"))] + XY, tr_cadr.m_command),
                    Cadr(tr_cadr.num, [InternalErp()], None, True),
                    Cadr(tr_cadr.num, [XYJ('G', NumVal("90"))], None),
                    Cadr(tr_cadr.num, [XYJ('F', NumVal.num(context.f))])
                ]
            else:
                delta_x = round((xval - context.x_rel_prev) / h.val.value, 2) if xval else None
                delta_y = round((yval - context.y_rel_prev) / h.val.value, 2) if yval else None
                XY = get_XY(delta_x, delta_y)
                delta_x = delta_x if delta_x else 0
                delta_y = delta_y if delta_y else 0
                xy_len = math.sqrt(delta_x * delta_x + delta_y * delta_y)
                f_misc = context.get_proportional_f(xy_len)
                context.f_misc = f_misc
                result = [
                    Cadr(tr_cadr.num, [XYJ('F', NumVal.num(f_misc))]),
                    Cadr(tr_cadr.num, [InternalRpt(h.val.ivalue)], None, True),
                    Cadr(tr_cadr.num, [XYJ('G', NumVal("91"))] + XY, tr_cadr.m_command),
                    Cadr(tr_cadr.num, [InternalErp()], None, True),
                    Cadr(tr_cadr.num, [XYJ('F', NumVal.num(context.f))])
                ]

            if context.D.VERBOSE_LOG:
                context.print(_("Converted file G01 H M25: {}:{} - {}").format(context.fname, cadr.lloc(), cadr.ltext(context.fname)))
            return result
    return None
