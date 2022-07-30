# Instruction throughput calculation
The instruction throughput of lattice surgeries on random two logical qubits with randomly allocated anomalous regions is evaluated by simulation.
The results are used for plotting Figure 10 of the paper. 

- `./src/`: C++ codes for calculating the instruction throughput for random lattice surgeries.
- `./build.sh`: Shell script that generate an executable in `./bin/` folder.
- `./micro_spawn.py`: Python script that will perform sampling with several configurations in parallel.
- `./micro_stat.py`: Python script that plots Figure 10.

# Usage
```shell
# Build executables
./build.sh

# Run 8 processes in parallel and save results in the folder named `data`
python micro_spawn.py data 8

# Plot results
python micro_stat.py
```

# Verified environment at authors

- OS: Ubuntu 20.04 LTS on WSL2 (Installed via windows store on Windows 11)
- Compiler: g++ 9.4.0
- Python: 3.9.6

