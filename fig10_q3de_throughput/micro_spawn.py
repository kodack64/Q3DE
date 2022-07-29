# Copyright 2022 NTT CORPORATION

import numpy as np
import itertools
import multiprocessing
import threading
import subprocess
import time
import sys
import os

if len(sys.argv) != 3:
    print("input data_folder thread_num")
    exit(0)

folder = sys.argv[1]
max_spawn = int(sys.argv[2])

if not os.path.exists(folder):
    os.mkdir(folder)
os.chdir(folder)

if os.name == "nt":
    proc = "../bin/throughput.exe"
else:
    proc = "../bin/throughput.out"

task_list = []
width = [7]
n_inst = [10000]
trial = [10]
ano_prob = [0] + list(np.logspace(-6,-4, 20))
ano_life = [100, 1000]

task_list = itertools.product(width, n_inst, trial, ano_prob, ano_life)
task_list = list(task_list)
task_list = [list(map(str, val)) for val in task_list]
print(task_list)
task_list = task_list * 1000
print("max_spawn = ", max_spawn)

task_lock = threading.Lock()
task_index = 0


def spawn_and_wait(thread_id):
    global task_list
    global task_lock
    global task_index
    while True:
        finish_flag = False
        my_task_index = 0

        task_lock.acquire()
        try:
            if task_index == len(task_list):
                finish_flag = True
            else:
                my_task_index = task_index
                task_index += 1
        finally:
            task_lock.release()

        if finish_flag:
            print("thread{:2} exits".format(thread_id))
            break
        else:
            fname = "result{}.txt".format(thread_id)
            arg = [proc, fname] + task_list[my_task_index]
            print(arg)
            process = subprocess.Popen(arg)
            # process = subprocess.Popen(arg, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            start = time.time()
            print("thread{:2} : start task {}: {}".format(thread_id, my_task_index, arg))
            process.wait()
            elapsed = time.time() - start
            print("thread{:2} : finish task {}: elp:{}".format(thread_id, my_task_index, elapsed))


thread_pool = []
for ind in range(max_spawn):
    thread = threading.Thread(target=spawn_and_wait, args=[ind])
    thread_pool.append(thread)
for thread in thread_pool:
    thread.start()
for thread in thread_pool:
    thread.join()
