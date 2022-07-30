This repository contains all the source codes and scripts to regenerate the figures and tables of the paper, Q3DE: A fault-tolerant quantum computer architecture for multi-bit burst errors by cosmic rays.
Several source codes are distributed only for the validation of the paper. See `./license.docx` for details.

## Software dependencies
- C++ compiler: g++ 9.0.4 or Visual Studio C++ 2019
- Data aggregation and plots: Python 3.9.4
- Environment-independent build: CMake 3.21.1
- Circuit synthesis: Xilinx Vitis HLS 2021.2.1
- Edmonds' Blossom algorithm: Blossom V

## Installation of Blossom V
Several codes depend on the Kolmogorov's implementation of Blossom V.
Please download the codes of Blossom V from [this URL](https://pub.ist.ac.at/~vnk/software.html).
Then, unzip and rename the folder to `blossom5` and place it to the following two directories
- `./fig3_8_logical_error_rate_with_anomaly/src/`
- `./fig7_anomaly_detection/generator/src/`
so that compilers can find `./fig3_8_logical_error_rate_with_anomaly/src/blossom5/PerfectMatching.h`, for example.

## Validation
Each folder will typically regenerate figures or tables used in the paper. 
The regenerated figures should be the same within a statistical fluctuation by Monte-Carlo sampling.
The parameters of high-level synthesis also fluctuate according to the chosen random seed.

## Tested environment
We tested our codes on Windows 11 and Ubuntu 20.04 LTS on Windows Subsystem Linux.

## Contents
This repository consists of five folders. These can be independently compiled and executed to generate different figures or tables.

### Evaluation of the effect of anomalous regions on logical errors (`fig3_8_logical_error_rate_with_anomaly`)
Logical error rates per cycle with and without an anomalous region are evaluated with the Monte-Carlo sampling. 
The results are used for plotting Figures 3 and 8. 

### Anomaly detection performance evaluation (`fig7_anomaly_detection`)
The performance of anomaly detection for the occurrence of the anomalous region is tested with the Monte-Carlo sampling.
The results are used for plotting Figure 7. 

### Evaluation of the qubit reduction rate by Q3DE (`fig9_qubit_reduction_by_q3de`)
The effective code distances under randomly allocated anomalous regions are evaluated with the Monte-Carlo sampling.
The results are used for plotting Figure 9. 

### Instruction throughput evaluation (`fig10_q3de_throughput`)
The instruction throughput of lattice surgeries on two random logical qubits with randomly allocated anomalous regions is evaluated by simulation.
The results are used for plotting Figure 10. 

### FPGA implementation of decoding units (`tab4_fpga_implementation`)
The performance and overhead of anomaly-position-aware error estimation are evaluated with high-level circuit synthesis.
We used Vitis HLS 2021.2.1 for evaluation. The obtained parameters are used in Table 4.

