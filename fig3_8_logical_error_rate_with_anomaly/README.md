# Calculation of logical errors with anomalies
Logical error rates per cycle with and without an anomalous region are evaluated with the Monte-Carlo sampling. 
The results are used for plotting Figure 3 and 8. 

- `./src/`: C++ codes for calculating logical errors with and without anomalies.
- `./build.sh`: Shell script that builds executables in `./bin/` folder.
- `./spawn_fig3.py`: Python script that will sample the logical error for fig 3. The results are saved in the first argument.
- `./plot_fig3.py`: Python script that plots Figure 3.
- `./spawn_fig8_1.py`: Python script that will sample the logical error for the above two figures of fig 8. The results are saved in the first argument.
- `./spawn_fig8_2.py`: Python script that will sample the logical error for the bottom two figures of fig 8. The results are saved in the first argument.
- `./plot_fig8.py`: Python script that plots Figure 8.

# Usage
```shell
# Build executables (use build.bat for Visual Studio)
./build.sh

# Run 8 executables in parallel and save results in the folder named `data`
# Change the value "8" to a preferable value.
python spawn_fig3.py data 8
python spawn_fig8_1.py data 8
python spawn_fig8_2.py data 8

# Plot results (these script can be executed during the above `spawn_*.py` script running.)
python plot_fig3.py
python plot_fig8.py
```

# Verified environment at authors

- OS: Ubuntu 20.04 LTS on WSL2 (Installed via windows store on Windows 11)
- Compiler: g++ 9.4.0
- Python: 3.9.6
- Note: `blossom5` folder in the `src` is the Kolmogorov's implementation of Minimum-weight perfect matching, which is cited and mentioned in the paper.


# Runtime
While the variances of logical error rates will converge quickly, 
the effective reduction of code distances unavoidably have huge uncertainties as explained in the paper.
To reduce the variance of them, we need massive number of samplings.

Therefore, we recommend parallelizing the sampling by executing multiple processes.
A script `spawn_*.py` will spawn multiple processes with difference configurations.
A single exuection is enough for `spawn_fig3.py` and `spawn_fig8_1`, which lasts about a day with 8-process parallelization.
On the other hand, the script `spawn_fig8_2.py` needs multiple runs. The variances in the paper are achieved with about 120000 shots.
