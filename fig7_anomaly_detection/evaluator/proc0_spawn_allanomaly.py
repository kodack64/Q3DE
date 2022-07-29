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

repeat = 1

cycle_list = sorted(list(set(np.logspace(1, 3, 100).astype(int))))

task_list = []
for anomaly_ratio in [1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]:
    for cycle in cycle_list:
        distance = 21
        anomaly_size = 1000
        physical_error_rate = 1e-3
        sample = 1000
        cycle = int(cycle)
        arg = ["../../generator/bin/surface_code_3d_anomaly", f"result_ratio{anomaly_ratio}_cycle{cycle}.txt", 
        f"{distance}", f"{cycle}", f"{sample}", f"{physical_error_rate}", str(anomaly_ratio), f"{anomaly_size}", "0"]
        task_list.append(arg)
task_list = task_list * repeat
print("\n".join(list(map(str,task_list))))

print("max_spawn = ", max_spawn)

task_lock = threading.Lock()
task_index = 0


def spawn_and_wait(id):
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
            print("thread{:2} exits".format(id))
            break
        else:
            arg = task_list[my_task_index]
            process = subprocess.Popen(arg)
            start = time.time()
            print("thread{:2} : start task {}: {}".format(id, my_task_index, arg))
            process.wait()
            elapsed = time.time() - start
            print("thread{:2} : finish task {}: elp:{}".format(id, my_task_index, elapsed))


thread_pool = []
for ind in range(max_spawn):
    thread = threading.Thread(target=spawn_and_wait, args=[ind])
    thread_pool.append(thread)
for thread in thread_pool:
    thread.start()
for thread in thread_pool:
    thread.join()
