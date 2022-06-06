#!/usr/bin/env python3

# source /home/artem/devs/linthreads/misc/scheduler/scripts/gdbhelper/schedtracer.py 


import shelve
import skiplist_node 
import json


def describe_rq(rq):
    return ('cpu', rq.attributes["cpu_num"].value,)


def get_framed_task_id(frame):
    datetime_now = frame[0]
    for stack_node in reversed(frame[1]):
        if stack_node.funname == "enqueue_task":
            return (datetime_now, "enqueued_task", stack_node.items["p"].attributes["tid"].value, 'on rq') + describe_rq(stack_node.items["self"])
        if stack_node.funname == "dequeue_task":
            return (datetime_now, "dequeued_task", stack_node.items["p"].attributes["tid"].value, 'on rq') + describe_rq(stack_node.items["self"])


def testdumper():
    with shelve.open('/tmp/linschedtrace.db') as debugdb:
        frames = debugdb["frames"]
        
    for frame in frames:    
        print(get_framed_task_id(frame))
        
def analyze_lognull():            
    with open("/tmp/linaffinity_currnull.json", "r") as f:
        s = f.read()
        
    with open("/tmp/cpuswitch_init.json", "r") as f:
        s1 = f.read()
        
    filter_tid = 39
    filter_cpu = 3
    task_routine_pop_set_null = json.loads("[" + s.strip()[:-1] + "]")
    cpu_switch_log = json.loads("[" + s1.strip()[:-1] + "]")
    
    def transform_list(i):
        return [i[-1], i[0]] + list(i)[1:-1]
    
    task_routine_pop_set_null = [transform_list(i) + ["task_routine_pop_set_null"] for i in task_routine_pop_set_null]
    cpu_switch_log = [transform_list(i) + ["cpu_switch_log"] for i in cpu_switch_log]
    
    TID_IDX = 3
    CPU_IDX = 2
    
    for node in sorted(task_routine_pop_set_null + cpu_switch_log):
        # niffies, cpu, tid,   = node
        if node[TID_IDX] == filter_tid or node[CPU_IDX] == filter_cpu:
            print(node)
            
            
def analyze_output():   
    path = "/home/artem/devs/linthreads/misc/build-test_affinity_wakeup-gcc5_64-Debug/bin/log.json"
    with open(path, "r") as f:
        s = "[" + f.read().strip()[:-1] + "]"
        
    objs = json.loads(s)    
    
    tasks = [o['task'] for o in objs]
    scheduled = {i:{'cpu':None, "state": None} for i in tasks}
    for i, o in enumerate(objs):
        obj = scheduled[o['task']]
        obj['idx'] = i
        cpu = o.get('cpu', None)
        if cpu is not None:
            obj['cpu'] = cpu
        state = o.get('state', None)
        if state is not None and obj['state'] != "finished":
            obj['state'] = state
            
    tasks_res = {task['idx']: {'state':task['state'], 'cpu':task['cpu'], 'tid': tid} for (tid,task) in scheduled.items()}
    
    statuses = {}
    for v in tasks_res.values():
        try:
            statuses[v['state']] += 1
        except KeyError:
            statuses[v['state']] = 1
            
    print(statuses)        
            
            
    # for i in sorted(tasks_res):        
    #     print(i, tasks_res[i])
   
    
    
if __name__ == "__main__":
    analyze_output()