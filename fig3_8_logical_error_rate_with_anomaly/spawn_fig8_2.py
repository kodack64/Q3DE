# Copyright 2022 NTT CORPORATION

import numpy as np
import itertools
import multiprocessing
import threading
import subprocess
import time
import sys
import os


# code distances
distances = [7, 9, 11, 13, 15, 17, 21]

# consider weight of anomaly or not. 1 = consider
use_weight = [0, 1]

# number of samples per iteration
trial_count = [1000]

# physical error probabilities
error_probs = np.logspace(np.log10(0.004), np.log10(0.04), 20)

# size of anomalies
anomaly_sizes = [0, 2, 4]

# number of iterations
repeat = 100



if len(sys.argv) != 3:
    print("input data_folder thread_num")
    exit(0)

folder = sys.argv[1]
max_spawn = int(sys.argv[2])

if not os.path.exists(folder):
    os.mkdir(folder)
os.chdir(folder)

if os.name == "nt":
    proc = "../bin/main.exe"
else:
    proc = "../bin/main"

task_list = []

task_list = itertools.product(distances, anomaly_sizes, use_weight, trial_count, error_probs)
task_list = list(task_list)
task_list = [list(map(str, val)) for val in task_list]
print(task_list)
task_list = task_list * repeat
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
            procname = proc
            arg = [procname] + task_list[my_task_index]
            size = arg[-1] 
            print(arg)
            process = subprocess.Popen(arg)
            # process = subprocess.Popen(arg, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            start = time.time()
            print("thread{:2} : start task {}: {}".format(
                thread_id, my_task_index, arg))
            process.wait()
            elapsed = time.time() - start
            print("thread{:2} : finish task {}: elp:{}".format(
                thread_id, my_task_index, elapsed))


thread_pool = []
for ind in range(max_spawn):
    thread = threading.Thread(target=spawn_and_wait, args=[ind])
    thread_pool.append(thread)
for thread in thread_pool:
    thread.start()
for thread in thread_pool:
    thread.join()
