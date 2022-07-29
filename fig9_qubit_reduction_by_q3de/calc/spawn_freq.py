# Copyright 2022 NTT CORPORATION

import pprint
import numpy as np
from _spawn_common import diagnose


def run():
    anomaly_size_org = 8
    mbbe_free = False
    lp_require = 1e-10
    code_cycle = 1e-6
    anomaly_freq_base = 0.1 * code_cycle  # freq per cycle
    num_qubit_org = 26
    for anomaly_freq_ratio in [10, 100]:
        anomaly_freq_org = anomaly_freq_base / anomaly_freq_ratio
        for q3de in [True, False]:
            if q3de:
                anomaly_lifetime = 30
            else:
                anomaly_lifetime = int(2.5e-2 / code_cycle + 1e-16)
            chip_area_ratio_list = np.logspace(0, 2, 11)
            print(chip_area_ratio_list)

            if mbbe_free:
                anomaly_lifetime = 0
                anomaly_size_org = 0

            result = []
            for chip_area_ratio in reversed(chip_area_ratio_list):
                item = diagnose(anomaly_freq_org, chip_area_ratio, num_qubit_org, anomaly_size_org, anomaly_lifetime, q3de, lp_require)
                if item[-1] is None:
                    break
                result.append(item)

            fout = open(f"./result/result_freq_{q3de}_{anomaly_freq_ratio}.txt", "w")
            for item in result:
                fout.write(" ".join(map(str, item)) + "\n")
            fout.close()
            pprint.pprint(result)


run()
