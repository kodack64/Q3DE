# FPGA implementation of decoding units

This folder is for regenerating the values of Table 4.
The execution of them requires We checked the codes with benchmarks with Vitis HLS 2021.2.1.

- `./without_anomaly/`: `decoder.cpp` and `decoder.h` are the target of HLS. `tbench.cpp` and the contents in `benchmark` folder are files for random input/output testing.
- `./with_anomaly/`: File tree is the same as `without_anomaly`, but this implementation takes an anomaly region into account.


# Workflow

- Create a new project with Vitis HLS 2021.2.1.
  - Set target device as `Zynq Ultrascale+ XCZU7EV-2FFVC1156
MPSoC (ZCU104 evaluation board)`
  - Set operating frequency as 400 MHz
- Add `decoder.cpp` and `decoder.h` to the source.
- Add the following files to Test Bench
  - `tbench_io.cpp`
  - `tbench_match.cpp`
  - `tbench_util.cpp`
  - `tbench_visualize.cpp`
  - `tbench.cpp`
  - `tbench.h`
- For loading the decoding lattice to generate errors and syndromes, please modify the path at the L9 of `tbench.cpp` to allocate `[with,without]_anomaly/benchmark/graph_idling/`
- Running test bench will test for random input/output.
- Circuit synthesis will report the latency for 1000 code cycles. The matching per sec can be calculated from it.
  - ANQ (active node queue size) can be tuned at L10 of `decoder.h`.

# Verified environment at authors

- OS: Windows 11
- Xilinx Vitis HLS 2021.2.1
