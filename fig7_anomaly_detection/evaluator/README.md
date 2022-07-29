
# Anomaly detection performance check

Our proposal tries to detect the occurrence of anomalous regions from the frequency of the active syndrome nodes.
This folder contains a set of scripts to perform the following analysis.

1. Numerically calculate the sufficient size of the detection window for anomaly detection of which the false-positive and true-negative ratios are below 1%.
2. Using the calculated detection window size, we evaluate the latency with its uncertainty and the error of estimated position of the anomaly detection

For the purpose, the script should be executed in the following order.

0. Generate executables named `surface_code_3d_anomaly` and `surface_code_anomaly_long` in `generator` folder. The first one outputs the list of active syndrome-node counts for each syndrome positions when all the qubits are anomalous. The second one simulate a more practical case; all the qubits are normal at first, but the anomalous region happens at the 500-th cycle.
1. Run `proc0_spawn_allanomaly.py`, which spawns the first executable in parallel to simulate d=21 surface codes where all the qubits are anomalous with several cycle durations and anomalous qubits' physical error rates. We assume the first argument is `data_allanomaly`. The second argument is the number of process that run in paralle.
2. Run `proc1_calculate_window.py`, which calulates the required detection window size for each anomalous qubits' physical error rates to supress the false-positive and true-negative ratios are below 1%.
3. Run `proc2_spawn_latency.py`, which spawns the second executable in parallel to simulate d=21 surface codes where an anomalous region happens at the 500-th cycle. We assume the first argument is `data_trajectory`. The second argument is the number of process that run in paralle.
4. Run `proc3_calculate_trajectory.py`. This script performs pre-process for the output of the previous step and produces several pickles.
5. Run `proc4_calculate_detection_latency.py`. This script calculates the statistics of latency of anomaly detection, i.e., calculate latency = (detection_cycle_index - 500). Output several results to the `./result/` folder.
6. Run `proc5_calculate_position_error.py`. This script calculates the error of estimated position of anomalous region.
7. Run `proc6_plot_micro_figure.py`, which will plots the figure in the paper using all the above data, and save the figure to the `./figure/` folder.
8. Run `proc7_plot_misc.py`, which will plots all the miscellaneous figures and save to the `./figure/` folder.

Note that there are several unused codes, which is because we have plotted several more figures but they are removed in the rebuttal phase of ISCA2022 (rejected) to make a room for putting the rebuttal information. The above procedure generates files above 30GB, so please be careful for the available disk space.

# Usage

```shell
./proc_all.sh
```

