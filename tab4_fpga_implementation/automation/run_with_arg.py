# Copyright 2022 NTT CORPORATION

import pprint
from eval_q3de import evaluate_q3de_synthesis
import sys
import os

# fetch arg
arc_name = sys.argv[1]
if arc_name == "BASE":
    consider_anomaly = False
elif arc_name == "Q3DE":
    consider_anomaly = True
else:
    raise ValueError(f"Invalid arcname {arc_name}")
queue_size = int(sys.argv[2])
clock_freq_MHz = float(sys.argv[3])
margin_ratio = float(sys.argv[4])
synth_id = int(sys.argv[5])

vitis_setting_path = ""
if os.name == "nt":
    vitis_setting_path = "D:/Xilinx/Vitis_HLS/2021.2/settings64.bat"
else:
    vitis_setting_path = "/tools/Xilinx/Vitis_HLS/2021.2/settings64.sh"

# execute
print(f"{arc_name}-{queue_size} with {clock_freq_MHz} MHz for margin {margin_ratio}")
result = evaluate_q3de_synthesis(
    consider_anomaly=consider_anomaly,
    queue_size=queue_size,
    clock_freq_MHz=clock_freq_MHz,
    margin_ratio=margin_ratio,
    part_name="xczu7ev-ffvc1156-2-e",
    relative_path="../../../../Q3DE/tab4_fpga_implementation",
    vitis_setting_path=vitis_setting_path,
    overwrite=False,
    synth_id=synth_id,
    execute_csim=True
)

# print result
pprint.pprint(result)
