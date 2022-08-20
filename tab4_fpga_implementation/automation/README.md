# VitisHLS synthesis automation

The folder `vitis_hls_automation` contains a set of scripts to perform high-level synthesis with python scripts.

The script `eval_q3de.py` contains a function that specifies the files of HLS targets and runs `C Synthesis` and `IMPLEMENTATION` with given parameter sets and calculates the achieved performance from the reports output from the VitisHLS.
The implementation can be executed from `run.py`, or `run_with_arg.py` with arguments.

## Usage
Before exuecting the scripts, open `run.py` and `run_with_arg.py` and updates
```
    relative_path="../../Q3DE/tab4_fpga_implementation",
    vitis_setting_path="D:/Xilinx/Vitis_HLS/2021.2/settings64.bat",
```
to these point the appropriate directories.

```
# Run synthesis with parameters in the script
python run.py

# Run synthesis for BASE-40 folder with 300MHz clock freq. The clock period is reduced by 30% in the C Synthesis.
# The last integer is folder ID, which is attached in the end of result folder.
python run_with_arg.py BASE 40 300 0.3 0
```
