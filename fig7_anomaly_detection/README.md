
# Anomaly detection performance evaluation
The performance of anomaly detection for the occurrence of anomalous region is tested with the Monte-Carlo sampling.
The results are used for plotting Figure 7. 

- `generator`: C++ program to simulate the trajectories under high-error-rate qubits.
- `evaluator`: Python scripts to evaluate the anomaly-detection performace step-by-step.

The process of `evaluator` consists of several sequential analysis. See `README.md` in the `evaluator` folder for details.

## Usage
```shell

# build
cd generator
./build.sh

# perform sampling and analysis
cd ../evaluator
./proc_all.sh
```

This numerical evaluation take long time and about 30GB disk space for intermediate outputs.

