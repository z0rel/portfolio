#!/usr/bin/env python3

import sys
import json
import multiprocessing
import subprocess
import argparse
import datetime
from itertools import chain
from find_clustering import Clustering

skip_real_testing = False


def arr_sz(mb, abi):
    # 5 - единичное число задач. TODO: целесообразно переделать масштабирование, например по
    # числу процессоров
    sz = int(mb * 1024 * 1024 / 4)

    max32_sz = 481574400
    if abi == "32" and sz > 481574400:
        return max32_sz

    return sz


def make_config_item(tasks, array_size, offset, task_array_size, off_sz_val, yield_interval, ntimes):
    return [tasks, array_size, offset, task_array_size, off_sz_val, yield_interval, ntimes]


def mkconf_item2(tasks, arr, task_arr, yld, ntimes):
    off_sz_val = int((arr - task_arr) / (tasks - 1))
    return [tasks, arr, off_sz_val, task_arr, off_sz_val, yld, ntimes]


def config_by_nooverlapped(abi, cpus, array_size_mb, tasks_multiplier, yield_interval, ntimes):
    def tasks_cnt(coeff):
        return (cpus + 1) * coeff

    def off_sz(mb, coeff):
        return int(arr_sz(mb, abi) / tasks_cnt(coeff))

    def chunck_arr_len(mb, coeff):
        return off_sz(mb, coeff) + arr_sz(mb, abi) - off_sz(mb, coeff) * tasks_cnt(coeff)

    array_size = arr_sz(array_size_mb, abi)
    tasks = (cpus + 1) * tasks_multiplier
    offset = int(array_size / tasks)
    task_array_size = int(array_size / tasks)
    assert(offset > 0)
    return make_config_item(tasks, array_size, offset, task_array_size, off_sz(array_size_mb, tasks_multiplier),
                            yield_interval, ntimes)


def config_by_time(abi, cpus, array_size_max, array_size_mb, tasks_multiplier, op_usec, restrict_time_sec,
                   yield_interval, ntimes):
    restrict_time_usec = restrict_time_sec * 1000000

    array_size_max = arr_sz(array_size_max, abi)
    array_size = arr_sz(array_size_mb, abi)
    tasks = (cpus + 1) * tasks_multiplier
    tasks_size = int(restrict_time_usec / (op_usec * tasks))
    if tasks_size > array_size:
        tasks_size = array_size

    tasks_offset = int((array_size - tasks_size) / (tasks - 1))
    if array_size == 0:
        assert(False)

    return make_config_item(tasks, array_size_max, tasks_offset, tasks_size, tasks_offset, yield_interval, ntimes)


def get_system_rr_interval():
    return int(subprocess.check_output(["/bin/cat", "/proc/sys/kernel/sched_rr_timeslice_ms"]))


class WorkflowConfig:
    def __init__(self, args):
        cpus = multiprocessing.cpu_count()
        self.result_fname = "result_stat_find_test.json"
        self.bin_path_coroutines = str()
        self.bin_path_systhreads = str()
        self.array_size = 0
        self.error = False
        self.skip_randgen_stage = False

        self.random_seed = 123
        self.ntimes = 10

        self.rq_cnt = [cpus]

        self.timeslice_msec = [get_system_rr_interval()]
        self.yield_interval = [None, 1, 10, 100, 500, 1000, 3000, 10000, 20000]

        self.tasks_multipliers = [1, 10, 100, 1000]
        self.array_sizes = [500]
        divisors = [100000, 10000, 1000, 100, 10, 1]

        self.key_coroutines = 'coroutines'
        self.key_systhreads = 'systhread'
        self.fmt_cachefile = 'tmp_cache_{0}.cache'
        self.logfile = 'log_find_statistics'

        cfg = self.workflow_argparser().parse_args(args=args)
        self.result_fname = cfg.result_fname
        self.bin_path_coroutines = cfg.bin_coroutines
        self.bin_path_systhreads = cfg.bin_systhreads
        self.logfile = cfg.logfile

        global testing64_build
        if "32" in self.bin_path_coroutines:
             testing64_build = False

        assert(self.bin_path_coroutines is not None)
        assert(len(self.bin_path_coroutines) > 0)
        assert(isinstance(self.bin_path_coroutines, str))

        assert(self.bin_path_systhreads is not None)
        assert(len(self.bin_path_systhreads) > 0)
        assert(isinstance(self.bin_path_systhreads, str))

        self.random_seed = cfg.random_seed
        self.ntimes = cfg.ntimes
        self.rq_cnt = json.loads(cfg.rq_cnt)
        self.abi = cfg.abi
        if self.abi not in {"32", "64"}:
            self.abi = "64"

        cpus = self.rq_cnt[0]

        self.tasks_multipliers = json.loads(cfg.tasks_multipliers)
        self.array_sizes = json.loads(cfg.array_sizes)
        self.timeslice_msec = json.loads(cfg.timeslice_msec)

        if cfg.yield_interval is not None and len(cfg.yield_interval) > 0:
            self.yield_interval = [(None if x == 0 else x) for x in json.loads(cfg.yield_interval)]

        if cfg.skip_randgen_stage:
            self.skip_randgen_stage = True

        counts = 10000000
        args = [self.bin_path_systhreads, "--wt_cnt=1", "--tasks_count=1", "--array_size={0}".format(counts),
                "--chunck_offset=0"]

        out = read_test_output(args, self, -2)
        if not out:
            self.error = True
            return

        op_usec = out['time usec'] / counts

        self.tasks_data = []

        # short tests
        # for yield_interval in cfg.yield_interval:
        #     for tasks_multiplier in self.tasks_multipliers:
        #         for divisor in divisors:
        #             for array_size in self.array_sizes:
        #                 self.tasks_data.append(config_by_nooverlapped(self.abi, cpus,
        #                                                               array_size_mb=array_size / divisor,
        #                                                               tasks_multiplier=tasks_multiplier,
        #                                                               yield_interval=yield_interval,
        #                                                               ntimes=cfg.ntimes))

        for yld in self.yield_interval:
            self.tasks_data.extend([
                mkconf_item2(tasks=(cpus + 1) * 100, arr=131072000, task_arr=26207000, yld=yld, ntimes=5),
                mkconf_item2(tasks=(cpus + 1) * 1000, arr=131072000, task_arr=2620700, yld=yld, ntimes=5)
            ])

        # long tests
        for yield_interval in self.yield_interval:
            for tasks_multiplier in self.tasks_multipliers:
                for divisor in divisors:
                    array_size = max(self.array_sizes)
                    x = config_by_time(self.abi, cpus, array_size, array_size_mb=array_size / divisor,
                                       tasks_multiplier=tasks_multiplier,
                                       op_usec=op_usec, restrict_time_sec=100, yield_interval=yield_interval,
                                       ntimes=cfg.ntimes)
                    if x is not None:
                        self.tasks_data.append(x)

        self.array_size = max([k[1] for k in self.tasks_data])

    def workflow_argparser(self):
        a = argparse.ArgumentParser(description='Аргументы командной строки для поискового теста производительности')
        a.add_argument('-c', '--bin_coroutines', required=True, type=str,
                       help='Исполняемый файл теста с корутинами')
        a.add_argument('-s', '--bin_systhreads', required=True, type=str,
                       help='Исполняемый файл теста с системными потоками')
        a.add_argument('-o', '--result_fname', default=self.result_fname, type=str,
                       help='Выходной файл, в который будет записана статистика')
        a.add_argument('--abi',
                       help='Разрядность архитектуры', default="64", type=str)
        a.add_argument('-r', '--random_seed',
                       help='Значение зерна сидирования', default=self.random_seed, type=int)
        a.add_argument('-n', '--ntimes', default=self.ntimes, type=int,
                       help='Количества повторений исполнения в одном тесте')
        a.add_argument('-w', '--rq_cnt',
                       help='Количество рабочих нитей', default=str(self.rq_cnt))
        a.add_argument('-S', '--skip_randgen_stage', action='store_true',
                       help='Пропустить этап генерации тестовых данных')
        a.add_argument('-t', '--tasks_multipliers', default=str(self.tasks_multipliers),
                       help='Количество задач относительно количества процессоров')
        a.add_argument('-a', '--array_sizes',
                       help='Размеры массива тестовых данных', default=str(self.array_sizes))
        a.add_argument('-l', '--timeslice_msec', default=str(self.timeslice_msec),
                       help='Длина кванта времени (в миллисекундах)')
        a.add_argument('-y', '--yield_interval',
                       help='Количество итераций для теста yield_force')
        a.add_argument('--logfile', type=str, default=self.logfile,
                       help='Имя файла лога')
        return a


class SubprocessOutputLogger:
    def __init__(self, arglist, logfile, test_num):
        self.proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.test_num = test_num
        self.logfile = logfile
        self.arglist = arglist
        self.result = ""
        self.retcode = 0
        self.wait()

    def wait(self):
        res = []
        with open(self.logfile, "ab") as f:
            f.write("test_num: {0}, {1}: running '{2}\n'".format(
                    self.test_num, datetime.datetime.now(), " ".join(self.arglist)).encode())
            while self.proc.poll() is None:
                for i in chain(self.proc.stdout, self.proc.stderr):
                    f.write(((str(datetime.datetime.now()) + ": ").encode()))
                    f.write(i)
                    f.write(b"\n")
                    res.append(i.decode())

            self.result = "".join(res)
            self.retcode = self.proc.returncode
            f.write("{0}: done with retcode {1}\n".format(datetime.datetime.now(), self.retcode).encode())

        if self.retcode != 0:
            print()
            print("error: retcode {0} in command: {1}".format(self.retcode, " ".join(self.arglist)))
            print("               command output: ", self.result)


def read_test_output(args, cfg, test_num):
    out = SubprocessOutputLogger(args, cfg.logfile, test_num)
    if out.retcode != 0:
        return None
    out = out.result
    try:
        res_list = json.loads("[" + out + "]")
        res = {}
        for i in res_list:
            for k in i.keys():
                res[k] = i[k]
        if 'error' in res:
            print("{0} - error in test:\n{2}. ".format(cfg.bin_path_coroutines, str(res)))
            return None
    except ValueError as err:
        print("{0} - json read error: {1}\n{2}. ".format(cfg.bin_path_coroutines, str(err), str(out)))
        return None

    return res


def test_usage():
    cfg = WorkflowConfig(sys.argv[1:])
    if cfg.error:
        return

    key_coroutines = cfg.key_coroutines
    key_systhreads = cfg.key_systhreads
    result = []

    with open(cfg.logfile, "w") as f:
        pass  # перезапись файла

    if not cfg.skip_randgen_stage:
        args = [
            cfg.bin_path_coroutines,
            "--wt_cnt={0}".format(multiprocessing.cpu_count() + 1),
            "--array_size={0}".format(cfg.array_size),
            ("--save_cache_file=" + cfg.fmt_cachefile.format('coroutines')),
            "--exit_after_save"
        ]
        if SubprocessOutputLogger(args, cfg.logfile, -1).retcode != 0:
            return

    test_num = 0

    print("[")
    for (tasks_count, array_size, chunck_offset, task_array_size, off_sz, yield_interval, ntimes) in cfg.tasks_data:
        for wt_cnt in cfg.rq_cnt:
            for timeslice_msec in cfg.timeslice_msec:
                    test_num += 1
                    if yield_interval is not None:
                        timeslice_msec = 0
                    arglist = [
                        "--wt_cnt={0}".format(wt_cnt),
                        "--timeslice={0}".format(timeslice_msec),
                        "--tasks_count={0}".format(tasks_count),
                        "--array_size={0}".format(array_size),
                        "--random_seed={0}".format(cfg.random_seed),
                        "--task_array_size={0}".format(task_array_size)
                    ]

                    if yield_interval is not None:
                        arglist.append("--yield_interval={0}".format(yield_interval))

                    stat = {k: [] for k in [key_coroutines, key_systhreads]}

                    def print_state(test_num, wt, ts_msec, tasks_count, array_size, task_array_size, chunck_offset):
                        sys.stdout.write(('{{"test_num": {0:2}, "cpu": {1:2}, "ts": {2:4}, "tasks": {3:4}, ' +
                                          '"sz": {4:11}, "tsz": {5:11}, "of": {6:11}, '
                                          ).format(test_num, wt, ts_msec, tasks_count, array_size,
                                                   task_array_size, chunck_offset))
                        sys.stdout.flush()

                    for i in range(0, ntimes):

                        sys.stdout.write("\r")
                        print_state(test_num, wt_cnt, timeslice_msec, tasks_count, array_size, task_array_size,
                                    off_sz)
                        sys.stdout.write(" .... ")
                        sys.stdout.write("{0}/{1}".format(i, cfg.ntimes))
                        sys.stdout.flush()
                        for cat, path in [(key_coroutines, cfg.bin_path_coroutines),
                                          (key_systhreads, cfg.bin_path_systhreads)]:

                            cachefile_name = ("--load_cache_file=" + cfg.fmt_cachefile).format('coroutines')
                            args = [path] + arglist + [cachefile_name]

                            if skip_real_testing:
                                res = {
                                    'array_size': array_size,
                                    'result': -2147483648,
                                    'time usec': 17005,
                                    'chunck_offset': chunck_offset,
                                    'chunck_array_size': task_array_size
                                }
                            else:
                                res = read_test_output(args, cfg, test_num)
                            if res is None:
                               break

                            task_array_size = res["chunck_array_size"]
                            arr_offset_size = res["chunck_offset"]
                            stat[cat].append(res['time usec'])
                        else:
                            continue
                        break
                    else:
                        l_co = Clustering(key_coroutines, stat[key_coroutines])
                        l_sys = Clustering(key_systhreads, stat[key_systhreads])

                        diff = float(l_sys.median) / float(l_co.median)
                        # sys_rr = get_system_rr_interval()

                        sys.stdout.write("\r")
                        sys.stdout.flush()

                        print_state(test_num, wt_cnt, timeslice_msec, tasks_count, array_size, task_array_size,
                                    arr_offset_size)

                        if yield_interval is not None:
                            one_cpu_iters = (task_array_size * tasks_count) / multiprocessing.cpu_count()
                            yield_interval_msec = round(((l_co.median / one_cpu_iters) * yield_interval) / 1000, 7)
                            print('"yld": {0:5}, "yld_msec": {1:5f}, '.format(yield_interval, yield_interval_msec), end='')
                        else:
                            print('"yld": None, "yld_msec": None, ', end='')

                        print(str(l_co) + ', ' + str(l_sys), end='')
                        print(', "diff": {0:5.2f}}},'.format(diff))

                        Clustering.balance_fmt_clusters([l_co, l_sys])
                        # print('    ', l_co.format_clusters())
                        # print('    ', l_sys.format_clusters())
                        # print('    ', l_sys.format_clusters_diffs(Clustering.get_diff_fmt_clusters(l_sys, l_co)))

                        node = {
                            'timeslice_msec': timeslice_msec,
                            'worker_cpus': wt_cnt,
                            'pool': tasks_count,
                            'size': array_size,
                            l_co.k: l_co.stat,
                            l_sys.k: l_sys.stat
                        }
                        if yield_interval is not None:
                            node['yield'] = yield_interval

                        result.append(node)

    print("]")
    s = json.dumps(result, sort_keys=True, indent=4)
    with open(cfg.result_fname, "w") as f:
        f.write(s)


if __name__ == '__main__':
    test_usage()
