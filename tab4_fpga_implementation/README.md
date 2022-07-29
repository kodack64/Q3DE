# FPGA analysis

This folder is for regenerating the values of Table 4.
The execution of them requires We checked the codes with benchmarks with Vitis HLS 2021.2.

- `./without_anomaly/`: `decoder.cpp` and `decoder.h` are the target of HLS. `tbench.cpp` and contents in `benchmark` are benchmark files for input/output testing.
- `./with_anomaly/`: the same as the above, except that this implementation cosiders the 
