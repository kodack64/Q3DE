# Copyright 2022 NTT CORPORATION

import numpy as np
import subprocess

proc = "../bin/main.out"


def check(anomaly_size: float, anomaly_lifetime: int, distance: int, freq: float, q3de: bool, lp_base: float = 0.1, p_pth: float = 0.1, max_cycle: int = 10**9, logname: str = "_log.txt") -> float:
    """Check the averaged logical error rate using C++ executable

    Args:
        anomaly_size (float): size of anomaly
        anomaly_lifetime (int): lifetime of anomaly (unit is code cycle)
        distance (int): code distance
        freq (float): frequency of cosmic rays
        q3de (bool): use q3de or not
        lp_base (float, optional): coefficient of logical error ansatz. Defaults to 0.1.
        p_pth (float, optional): exponential base of logical error ansatz. Defaults to 0.1.
        max_cycle (int, optional): simulation cycle number. Defaults to 10**9.
        logname (str, optional): Logname. Defaults to "_log.txt".

    Returns:
        float: logical error rate
    """
    arg = [proc, logname, anomaly_size, anomaly_lifetime, distance, freq, lp_base, p_pth, max_cycle]
    arg = list(map(str, arg))
    process = subprocess.Popen(arg, stdout=subprocess.PIPE)
    process.wait()
    result = process.stdout.readline().decode()
    element = result.split()
    lp_q3de = float(element[-2])
    lp_noq3de = float(element[-1])
    if q3de:
        return lp_q3de
    else:
        return lp_noq3de


def diagnose(anomaly_freq_org: float, chip_area_ratio: float, num_qubit_org: int, anomaly_size_org: float, anomaly_lifetime: int, q3de: bool, lp_require: float) -> tuple:
    """Check the required chip density according to a given chip area size by iteratively calculating the logical error rate

    Args:
        anomaly_freq_org (float): frequency of cosmic rays in reference chip
        chip_area_ratio (float): the area ratio of the chip to diagnose to the reference
        num_qubit_org (int): number of qubits in reference chip
        anomaly_size_org (float): size of anomaly in reference chip
        anomaly_lifetime (int): lifetime of anomaly (unit is code cycle)
        q3de (bool): use q3de or not
        lp_require (float): required logical error rate

    Returns:
        tuple: _description_
    """

    # We assume d=11 is at least required for lp_require, this is correct in the region of our interest
    distance = 11
    while True:
        # calculate freq/size of anomaly for achieving the distance
        num_qubit = distance**2 * 2
        anomaly_freq = anomaly_freq_org * chip_area_ratio
        qubit_density_ratio = (num_qubit / num_qubit_org) / chip_area_ratio
        anomaly_size = anomaly_size_org * np.sqrt(qubit_density_ratio)
        print(q3de, chip_area_ratio, qubit_density_ratio, "/", anomaly_size, anomaly_lifetime, distance, anomaly_freq, end=" ", flush=True)

        # calculate the logical error
        lp = check(anomaly_size, anomaly_lifetime, distance, anomaly_freq, q3de)
        print(lp)

        # if requirement satisfies, return information
        if lp < lp_require:
            return (q3de, anomaly_size_org, chip_area_ratio, qubit_density_ratio, chip_area_ratio * qubit_density_ratio,  distance, lp)

        # if density exceeds x100, abort
        if qubit_density_ratio > 1e2:
            return (q3de, anomaly_size_org, chip_area_ratio, None, None, None, None)
        distance += 2
