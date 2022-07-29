# Copyright 2022 NTT CORPORATION

python proc0_spawn_allanomaly.py data_allanomaly 1
python proc1_calculate_window.py
python proc2_spawn_latency.py data_trajectory 1
python proc3_calculate_trajectory.py
python proc4_calculate_detection_latency.py
python proc5_calculate_position_error.py
python proc6_plot_micro_figure.py
python proc7_plot_misc.py



