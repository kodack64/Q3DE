# FPGA implementation of decoding units

This folder is for regenerating the values of Table 4.
The execution of them requires We checked the codes with benchmarks with Vitis HLS 2021.2.1.

- `./40-BASE/` and `80-BASE`: `decoder.cpp` and `decoder.h` are the target of HLS. `tbench.cpp` and the contents in `benchmark` folder are files for random input/output testing.
- `./40-Q3DE/` and `80-Q3DE`: File tree is the same as `[40,80]-BASE`, but this implementation takes an anomaly region into account.

Each directory corresponds to each row of the table.

# Workflow (for each directory)
- Load environment setting script `settings64.sh`
- Launch `vitis_hls`
- Create a new project with Vitis HLS 2021.2.1.
  - choose `Create Project`
  - Put workspace just under the `tab4_fpga_implementation` directory.
  - Add `decoder.cpp` and `decoder.h` to the source and select `decoder` as top function. push `Next`.
  - Add the following files to Test Bench. then, push `Next`.
    - `tbench.cpp`
    - `benchmark/tbench.h`
    - `benchmark/tbench_io.cpp`
    - `benchmark/tbench_match.cpp`
    - `benchmark/tbench_util.cpp`
    - `benchmark/tbench_visualize.cpp`
  - Set implementation targets.
    - Set target device as `Zynq Ultrascale+ XCZU7EV-2FFVC1156 MPSoC (ZCU104 evaluation board)`
    - Set clock period as 2.5 ns
    - Push `Finish`.
- Run C Simulation (functional test)
- Run C Synthesis
  - Synthesis will report the latency for 1000 code cycles. The matching per sec can be calculated from it.
    - throughput = 1000 ns / {reported latency} ns
- Run Implementation
    - Choose `RTL Synthesis, Place & Route`
    - The result shows the usage of LUT and Flip-Flop. The value itself may vary from the paper due to the different seed value.

# Verified environment at authors

- OS: Ubuntu 16.04 LTS
- Xilinx Vitis HLS 2021.2.1
