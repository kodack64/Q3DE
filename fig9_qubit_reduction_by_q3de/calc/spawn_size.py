# Copyright 2022 NTT CORPORATION

import pprint
import numpy as np
from _spawn_common import diagnose
import os

def run():
    for anomaly_size_org in [8, 6, 4, 2]:
        for q3de in [False, True]:
            mbbe_free = False
            lp_require = 1e-10

            code_cycle = 1e-6
            if q3de:
                anomaly_lifetime = 30
            else:
                anomaly_lifetime = int(2.5e-2 / code_cycle + 1e-16)
            # anomaly_size_org = 8
            anomaly_freq_org = 0.1 * code_cycle  # freq per cycle
            num_qubit_org = 26
            chip_area_ratio_list = np.logspace(0, 2, 11)

            if mbbe_free:
                anomaly_lifetime = 0
                anomaly_size_org = 0

            result = []
            for chip_area_ratio in reversed(chip_area_ratio_list):
                item = diagnose(anomaly_freq_org, chip_area_ratio, num_qubit_org, anomaly_size_org, anomaly_lifetime, q3de, lp_require)
                if item[-1] is None:
                    break
                result.append(item)

            fout = open(f"./result/result_size_{q3de}_{anomaly_size_org}.txt", "w")
            for item in result:
                fout.write(" ".join(map(str, item)) + "\n")
            fout.close()
            pprint.pprint(result)

if not os.path.exists("./result/"):
    os.mkdir("./result/")

run()
