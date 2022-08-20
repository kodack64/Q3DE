# FPGA implementation of decoding units

(2022 20Aug updated: Add details of evaluation)

This folder is for regenerating the values of Table 4. We checked the codes with benchmarks with Vitis HLS 2021.2.1.

- `./40-BASE/` and `80-BASE`: `decoder.cpp` and `decoder.h` are the target of HLS. `tbench.cpp` and the contents in `benchmark` folder are files for random input/output testing.
- `./40-Q3DE/` and `80-Q3DE`: File tree is the same as `[40,80]-BASE`, but this implementation takes an anomaly region into account.

Each directory corresponds to each row of the table.

# Instllation

Vitis HLS 2021.2.1 is required for synthesizing circuits from these codes. Note that the installation of Vitis HLS requires about 200GB.
To regenerate the results in the main text, we need to check `Devices -> Devices for Custom Platforms -> UltraScale+` in the instllation.


# Workflow (for each directory)
- Load environment setting script `settings64.sh` (`settings64.bat` for Windows) located in the installed folder of Vitis HLS.
- Launch `vitis_hls`
- Create a new project with Vitis HLS 2021.2.1.
  - choose `Create Project`
  - Put workspace just under the `tab4_fpga_implementation` directory.
  - Add `decoder.cpp` and `decoder.h` to the source and select `decoder` as top function. push `Next`.
  - Add the following files to Test Bench. Then, push `Next`.
    - `tbench.cpp`
    - `benchmark/tbench.h`
    - `benchmark/tbench_io.cpp`
    - `benchmark/tbench_match.cpp`
    - `benchmark/tbench_util.cpp`
    - `benchmark/tbench_visualize.cpp`
  - Open `tbench.cpp` and set the path at the 11th line.
    - Change from `string path = "../../../../40-BASE/benchmark/graph_idling/";` to the relative path from project file or the absolute path.
  - Set implementation targets.
    - Set target device as `Zynq Ultrascale+ XCZU7EV-2FFVC1156 MPSoC (ZCU104 evaluation board)`
    - For the clock period, see `Configuration`.
    - Push `Finish`.
- Run C Simulation (functional test)
  - Simulation will report the results of random input/output tests.
- Run C Synthesis
  - Synthesis will report the latency cycles for 1000 matching.
- Run Implementation
    - Choose `RTL Synthesis, Place & Route`.
    - For the clock period, see `Configuration`.
    - The result shows the achieved clock period and usage of LUT and Flip-Flop. The value itself may vary from the paper due to the different seed values.

# Evaluation
## Functional test
If the test passes, `C Simulation` ends with the following message.
```
INFO: [SIM 211-1] CSim done with 0 errors.
INFO: [SIM 211-3] *************** CSIM finish ***************
INFO: [HLS 200-111] Finished Command csim_design CPU user time: 1 seconds. CPU system time: 0 seconds. Elapsed time: 2.339 seconds; current allocated memory: 94.137 MB.
INFO: [HLS 200-112] Total CPU user time: 2 seconds. Total CPU system time: 1 seconds. Total elapsed time: 3.388 seconds; peak allocated memory: 1.084 GB.
Finished C simulation.
```

## Matching per code cycle
- `C`: The latency cycle for 1000 matching. This value is reported after `Run C Synthesis` as `Latency (cycles)`
- `T`: The clock period of implemented circuits. This value is reported after `Run Implementation` as `CP achieved post-implementation`

Then, the matching per cycle can be calculated as follows.

```
1000 [ns / code cycle]
----------------------------------
(C/1000) [clock cycles / matching] * T [ns / clock cycle] 
```

## Resource usage
The resource usage is reported after `Run Implementation`

# Recommended configuration

In the submitted paper, we optimized clock periods for `C Synthesis` and `IMPLEMENTATION` as follows.

- `BASE-40`: C Synthesis `1.66 ns`, IMPLEMENTATION `2.5ns`
- `Q3DE-40`: C Synthesis `1.66 ns`, IMPLEMENTATION `2.5ns`
- `BASE-80`: C Synthesis `1 ns`, IMPLEMENTATION `2.5ns`
- `Q3DE-80`: C Synthesis `1 ns`, IMPLEMENTATION `2.5ns`

We chose a more strict clock period in `C Synthesis` since choosing the same clock period may violate the timing restriction in `IMPLEMENTATION`.
Note that `C Synthesis` may warn `Timing Violation` with the above clock period. This is not a problem since we can satisfy the timing at the `IMPLEMENTATION`.
While the achieved performance would vary for each run due to the randomness in the optimization, these configurations will output values similar to the match/cycle listed in the paper.

After we worked on the arrangement of artifact evaluation, we found that the following setting is more stable and outputs higher matching per code cycle.

- `BASE-40`: C Synthesis `2 ns`, IMPLEMENTATION `2 ns`
- `Q3DE-40`: C Synthesis `2.22 ns`, IMPLEMENTATION `2.22 ns`
- `BASE-80`: C Synthesis `3.0 ns`, IMPLEMENTATION `3.0 ns`
- `Q3DE-80`: C Synthesis `3.0 ns`, IMPLEMENTATION `3.0 ns`

Even with the above setting, we can observe that the degradation of introducing the mechanism of Q3DE is modest.

# Trouble shooting
## Path error
At `C Simulation`, you may encounter the following error message.
```
file not found
@E Simulation failed: Function 'main' returns nonzero value '1'.
ERROR: [SIM 211-100] 'csim_design' failed: nonzero return value.
INFO: [SIM 211-3] *************** CSIM finish ***************
INFO: [HLS 200-111] Finished Command csim_design CPU user time: 1 seconds. CPU system time: 0 seconds. Elapsed time: 2.309 seconds; current allocated memory: 101.730 MB.
4
    while executing
"source D:/research/vitis_instruction/ArtifactEvaluationQ3DE/solution1/csim.tcl"
    invoked from within
"hls::main D:/research/vitis_instruction/ArtifactEvaluationQ3DE/solution1/csim.tcl"
    ("uplevel" body line 1)
    invoked from within
"uplevel 1 hls::main {*}$newargs"
    (procedure "hls_proc" line 16)
    invoked from within
"hls_proc [info nameofexecutable] $argv"
INFO: [HLS 200-112] Total CPU user time: 2 seconds. Total CPU system time: 1 seconds. Total elapsed time: 3.391 seconds; peak allocated memory: 1.085 GB.
Finished C simulation.
```

This is due to the wrong path specification and cannot find the lattice information to generate test data.
Please check the path string, and confirm the path ends with `/`.


## Revision overflow
At `IMPLEMENTATION -> Route and Place`, you may encounter the following error message.
```
source run_ippack.tcl -notrace
ERROR: '2208101851' is an invalid argument. Please specify an integer value.
    while executing
"rdi::set_property core_revision 2208101851 {component component_1}"
    invoked from within
"set_property core_revision $Revision $core"
    (file "run_ippack.tcl" line 904)
INFO: [Common 17-206] Exiting Vivado at Wed Aug 10 18:51:17 2022...
ERROR: [IMPL 213-28] Failed to generate IP.
INFO: [HLS 200-111] Finished Command export_design CPU user time: 3 seconds. CPU system time: 0 seconds. Elapsed time: 12.838 seconds; current allocated memory: 2.871 MB.
command 'ap_source' returned error code
    while executing
"source C:/Users/lab/AppData/Roaming/Xilinx/Vitis/Q3DETest/solution1/export.tcl"
    invoked from within
"hls::main C:/Users/lab/AppData/Roaming/Xilinx/Vitis/Q3DETest/solution1/export.tcl"
    ("uplevel" body line 1)
    invoked from within
"uplevel 1 hls::main {*}$newargs"
    (procedure "hls_proc" line 16)
    invoked from within
"hls_proc [info nameofexecutable] $argv"
INFO: [HLS 200-112] Total CPU user time: 5 seconds. Total CPU system time: 1 seconds. Total elapsed time: 14.448 seconds; peak allocated memory: 1.166 GB.
Finished Export RTL/Implementation.
```

This is caused by the revision number overflow problem of Vitis HLS. A script to fix the problem is officially provided in the following URL.

https://support.xilinx.com/s/article/76960

# Verified environment at authors

- OS: Ubuntu 16.04 LTS and Windows 11
- Xilinx Vitis HLS 2021.2.1
