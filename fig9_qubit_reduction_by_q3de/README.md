# Qubit reduction evaluation
The effective code distances under randomly allocated anomalous regions are evaluated with the Monte-Carlo sampling.
The results are used for plotting Figure 9. 

# Usage

```shell
# build binary
./build.sh

# calculate
cd calc
python spawn_freq.py
python spawn_lifetime.py
python spawn_size.py

# plot figure
python plot_all.py
```

When we test the configuration with long-live anomalous regions with high frequencies, this program sometimes consume a few GB memory space.


# Verified environment at authors

- OS: Ubuntu 20.04 LTS on WSL2 (Installed via windows store on Windows 11)
- Compiler: g++ 9.4.0
- Python: 3.9.6
