# Copyright 2022 NTT CORPORATION

# calculate window size

import numpy as np
import pickle
from _util_fileproperty import get_file_prop_list
from _util_count_threshold import get_threshold_count
from _util_mumap_analysis import get_above_count

d = 21
allowed_confidence = 1e-2

stab_num = d*(d-1)*2
assert(stab_num == 840)
folder_name = "data_allanomaly"
cycles, ratios = get_file_prop_list(folder_name)

def create_reference(perf_dict: dict) -> tuple:
    """Create a reference data

    This function returns two items.
     threshold_dict: window_size -> threshold count: a map from window_size to threshold_count calculated from CLT
     above_count_ref: window_size -> false-positive count: a map from a certain error-rate ratio and window size to number of false positive count

    Returns:
        tuple: threshold_dict and above_count_ref. See the document.
    """
    threshold_dict = {}
    print("* create reference")
    ratio = ratios[0]
    # assume the first ratio is no anomaly
    assert(ratio == 1)
    above_count_ref = []
    above_count_ref_std = []
    for _, cycle in enumerate(cycles):
        # For each ratio and cycle, calculate the threshold count of active nodes at a potision
        threshold_count, _ = get_threshold_count(folder_name, cycle)
        threshold_dict[cycle] = threshold_count

        # For each ratio and cycle, count the number of syndrome positions above the threshold count
        #  In the reference case, this count corresponds to the False-positive, i.e., classify normal region as anomalous region.
        val, valstd = get_above_count(folder_name, cycle, ratio, threshold_count)
        above_count_ref.append(val)
        above_count_ref_std.append(valstd)
        print(f"ratio={ratio} cycle_duration={cycle} -> calculated threshold {threshold_count} and above count {val}")

    perf_dict[ratio] = {
        "x": cycles,
        "ratio": ratio,
        "above_count": above_count_ref,
        "above_count_std": above_count_ref_std,
    }

    return threshold_dict, above_count_ref

def check_required_window_size(perf_dict: dict, threshold_dict: dict, above_count_ref: list) -> dict:
    """Create required window size for satisfying required confidence

    Args:
        threshold_dict (dict): threshold value obtained from the frequency of active nodes in normal qubits
        above_count_ref (list): the number of false-positive rate in the reference (normal region)

    Returns:
        dict: required window sizes
    """
    required_window_dict = {}
    print("* calculate window size for each ratio")
    for _, ratio in enumerate(ratios[1:]):

        above_count = []
        for ci, cycle in enumerate(cycles):

            assert(cycle in threshold_dict)
            threshold_count = threshold_dict[cycle]
            # For each ratio and cycle, count the number of syndrome positions above the threshold count
            #  In the test case, this count corresponds to the True-positive, i.e., classify anomalous region as anomalous region.
            val, _ = get_above_count(folder_name, cycle, ratio, threshold_count)
            above_count.append(val)

            # If this "cycle" is the minimum value that satisfies the following, set to required window
            #  1. In the test data True-negative rate is below confidence
            #  2. In the reference data, Falst-positive rate is below confidence
            print(f"ratio={ratio} cycle_duration={cycle} calculated threshold {threshold_count}")
            print(f"  -> above count {val} True-negative {1-val/stab_num} False-positive = {above_count_ref[ci]/stab_num}")
            if (1-val/stab_num < allowed_confidence) and (above_count_ref[ci]/stab_num < allowed_confidence):
                assert(ratio not in required_window_dict)
                print(" *** accept ***")
                required_window_dict[ratio] = cycle
                break

        # if list of cycles is too small and not sufficient for reliable detection, assign None
        if ratio not in required_window_dict:
            required_window_dict[ratio] = None

    return required_window_dict

def run():
    perf_dict = {}
    threshold_dict, above_count_ref = create_reference(perf_dict)
    print(f"threshold_dict = {threshold_dict}, above_count_ref={above_count_ref}")
    required_window_dict = check_required_window_size(perf_dict, threshold_dict, above_count_ref)

    print(f"* result {required_window_dict}")

    fout = open("./result/_windowsize.txt", "w")
    for ratio in ratios[1:]:
        window = required_window_dict[ratio]
        threshold = threshold_dict[window]
        fout.write(f"{ratio} {window} {threshold}\n")
    fout.close()

    window_sizes = [required_window_dict[r] for r in ratios[1:]]
    perf_dict["window"] = {
        "ratios": ratios[1:],
        "window_sizes": window_sizes
    }
    with open("./result/_result_anomaly_detection_perf.pkl", "wb") as fout:
        pickle.dump(perf_dict, fout)


run()
