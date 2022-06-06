#!/usr/bin/env python3

import subprocess
import json
from subprocess import TimeoutExpired
from os.path import join, sep
import unittest
from itertools import product
import platform
from datetime import datetime
from colored_testrunner import ColoredTextTestRunner
import multiprocessing

TIMEOUT = 15  # sec


class Args:
   def __init__(self, profile, os_name, compiler, bit):
       self.tmpname = "{profile}_{compiler}_{os}{bits}".format(
           profile=profile, os=os_name, compiler=compiler, bits=bit)
       self.builddir = join(self.tmpname, "bin")
       self.profile = profile
       self.os_name = os_name
       self.compiler = compiler
       self.bit = bit

   def __str__(self):
       return "builddir: {0}".format(self.builddir)


def get_subprocess_string(cmd):
    ps = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    res = []
    for i in ps.communicate(timeout=TIMEOUT):
        if i:
            res.append(i.decode('latin-1'))
    return "\n".join(res)


def append_if_not_none(l, prefix, val):
    if val is not None:
        l.append(prefix + str(val))


class SchedulerTestcase(unittest.TestCase):
    def __init__(self, params, *args, **kwargs):
        self.params = params
        self.tick = None
        self.post_message = None
        unittest.TestCase.__init__(self, *args, **kwargs)

    def runTest(self):
        pass

    def setUp(self):
        self.tick = datetime.now()

    def tearDown(self):
        self.tick = (datetime.now() - self.tick).total_seconds()


class TestScheduler(SchedulerTestcase):
    app_scheduler = "scheduler"

    def shortDescription(self):
        return join(self.params.builddir, TestScheduler.app_scheduler)

    def test_scheduler(self):
        try:
            cmd = [join(self.params.builddir, TestScheduler.app_scheduler)]
            s = get_subprocess_string(cmd)
        except TimeoutExpired as err:
            self.fail("Fail test scheduler: timeout expired. " + str(err))


class TestFind(SchedulerTestcase):
    app_find_coroutines = "test_find_coroutines"
    app_find_systhread  = "test_find_systhread"

    def shortDescription(self):
        s1 =  "{0}{1}{2}".format(self.params.builddir, sep, TestFind.app_find_coroutines)
        s2 =  "{0}{1}{2}".format(self.params.builddir, sep, TestFind.app_find_systhread)
        return s1 + "  " + s2


    def testing_find_proc(self, name, params, cmd):

        try:
            out = get_subprocess_string(cmd)
            fail = None
            try:
                arr_obj = json.loads("[" + out + "]")
            except ValueError as err:
                fail = str(err)
            if fail is not None:
                self.fail("{0} - json read error: {1}\n{2}. ".format(name, str(fail), str(out)))
                return

            obj = {}
            for i in arr_obj:
                for k in i.keys():
                    obj[k] = i[k]

            if 'error' in obj:
                self.fail("{0} - error in test:\n{2}. ".format(name, str(obj)))

            t = obj['time usec']
            if 'result' not in obj:
                self.fail("{0} - result not in return str. ".format(name))
            return t, obj

        except TimeoutExpired as err:
            self.fail("{0} - timeout expired. ".format(name) + str(err))

    def test_find_coroutines(self):
        wt_cnt        = [multiprocessing.cpu_count()]
        timeslice     = [6]
        thread_pool   = [10 * multiprocessing.cpu_count()]
        array_size    = [40000000]
        task_array_size = [1000000]

        for wt, timesl, pool, arr_sz, task_szs in product(wt_cnt, timeslice, thread_pool, array_size, task_array_size):
            argl = []
            append_if_not_none(argl, "--wt_cnt=", wt)
            append_if_not_none(argl, "--timeslice=", timesl)
            append_if_not_none(argl, "--tasks_count=", pool)
            append_if_not_none(argl, "--array_size=", arr_sz)
            append_if_not_none(argl, "--task_array_size=", task_szs)

            co_cmd = [join(self.params.builddir, TestFind.app_find_coroutines)] + argl
            th_cmd = [join(self.params.builddir, TestFind.app_find_systhread)] + argl
            co, co_obj = self.testing_find_proc(TestFind.app_find_coroutines, self.params, co_cmd)
            th, th_obj = self.testing_find_proc(TestFind.app_find_systhread, self.params, th_cmd)
            usec_div = 1000000.0

            if arr_sz is not None:
                if co_obj['array_size'] != arr_sz:
                  self.fail(' coroutines array_size: {{"program": {0}, "arg": {1}}}\n arguments: {2}'.format(
                      co_obj['array_size'], arr_sz, " ".join(co_cmd)))

            th_div_co = th / co
            strargs = " ".join(argl)
            message = ("th/co: {0:5.2f}, co: {1:8.5f} sec, th: {2:8.5f} sec".format(
                       th_div_co, co / usec_div, th / usec_div, ))

            self.post_message = ("        " + message + "\n"
                                 "        args: " + strargs)

            if self.params.profile == 'debug':
                estimate = 0.3
            else:
                estimate = 0.7

            if th_div_co < estimate:
                self.fail("Coroutines execution is slow.\n  " + message + "\n  args:" + strargs)


class TestAffinityWakeup(SchedulerTestcase):
    app_affinity_wakeup = "test_affinity_wakeup"

    def shortDescription(self):
        return join(self.params.builddir, TestAffinityWakeup.app_affinity_wakeup)

    def test_affinity_wakeup(self):
        thread_pool = [100]
        wt_cnt      = [4]
        timeslice   = [6]
        task_suspend_pass = [1000]
        task_sleep_iteration_interval = [100000]
        schedule_deschedule_cnt = [5]

        args = [tuple(l) for l in product(thread_pool, wt_cnt, timeslice,
                                          schedule_deschedule_cnt, task_suspend_pass, task_sleep_iteration_interval)]

        for th, wt, tm, shdsh, susp_pass, sl_interval in args:
            argl = []
            append_if_not_none(argl, "--tasks_count=", th)
            append_if_not_none(argl, "--wt_cnt=", wt)
            append_if_not_none(argl, "--timeslice=", tm)
            append_if_not_none(argl, "--sleep_wakeup_per_task=", shdsh)
            append_if_not_none(argl, "--task_suspend_pass=", susp_pass)
            append_if_not_none(argl, "--task_sleep_interval=", sl_interval)

            try:
                cmd = [join(self.params.builddir, TestAffinityWakeup.app_affinity_wakeup)] + argl
                s = get_subprocess_string(cmd)

            except TimeoutExpired as err:
                self.fail("Fail test scheduler: timeout expired. " + str(err))
                return

            s1 = "[" + s.strip()[:-1] + "]"
            try:
                objs = json.loads(s1)
            except ValueError as err:
                msg = "args: " + " ".join(argl) + "\n"
                fname = "/tmp/" + self.params.tmpname + "_test_affinity_wakeup"
                with open(fname, "w") as f:
                   f.write(s1)
                self.fail(msg + "Fail test scheduler: bad JSON output. Writed to {0}. {1}".format(
                    fname, str(err)))
                return

            tasks = [o['task'] for o in objs]
            scheduled = {i: {'cpu': None, "state": None} for i in tasks}
            for i, o in enumerate(objs):
                obj = scheduled[o['task']]
                obj['idx'] = i
                cpu = o.get('cpu', None)
                if cpu is not None:
                    obj['cpu'] = cpu
                state = o.get('state', None)
                if state is not None and obj['state'] != "finished":
                    obj['state'] = state

            statuses = {}
            tasks_set = set([i for i in range(0, th + 1)])
            for tid, task in scheduled.items():
                if tid in tasks_set:
                    tasks_set.remove(tid)
                try:
                    statuses[task['state']] += 1
                except KeyError:
                    statuses[task['state']] = 1

            if statuses['finished'] != th:
                message = ("Finisched tasks count is {0}. But must be {1}. Unfinisched tasks: {2}".format(
                    statuses['finished'], th, str(list(sorted(set(tasks_set))))) + "\n" + " ".join(cmd))
                self.fail(message)


def test_all():
    profiles  = ["release", "debug"]
    compilers = ["clang", "gcc"]
    bits      = ["64", "32"]

    testdirs = [(profile, platform.system(), compiler, bit)
                for (profile, compiler, bit) in product(profiles, compilers, bits)]

    tests = []
    for (profile, os_name, compiler, bit) in testdirs:
        args = Args(profile, os_name, compiler, bit)
        tests.extend([TestScheduler(args, "test_scheduler"),
                      TestAffinityWakeup(args, "test_affinity_wakeup"),
                      TestFind(args, "test_find_coroutines")])
    suite = unittest.TestSuite(tests=tests)
    runner = ColoredTextTestRunner()
    runner.run(suite)


if __name__ == '__main__':
    test_all()
