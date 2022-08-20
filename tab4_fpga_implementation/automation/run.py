import pprint
from eval_q3de import evaluate_q3de_synthesis

result = evaluate_q3de_synthesis(
    queue_size=40,
    consider_anomaly=False,
    clock_freq_MHz=350,
    margin_ratio=0,
    part_name="xczu7ev-ffvc1156-2-e",
    relative_path="../../Q3DE/tab4_fpga_implementation",
    vitis_setting_path="D:/Xilinx/Vitis_HLS/2021.2/settings64.bat",
    overwrite=False
)

pprint.pprint(result)
