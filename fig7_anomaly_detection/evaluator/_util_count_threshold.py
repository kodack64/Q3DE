# Copyright 2022 NTT CORPORATION

import numpy as np
import scipy.special
from _util_content import get_content

def get_threshold_count(folder_name: str, cycle: int, confidence:float = 0.99) -> tuple:
    """Get threshold active-node count for a given window size

    Args:
        folder_name (str): data folder name
        cycle (int): number of window cycles
        confidence (float, optional): confidence interval. Defaults to 0.99.

    Returns:
        tuple: pair of values. Confidence and number of samples.
    """

    # get data
    dic_sort = get_content(folder_name, cycle, 1)

    vals = []
    for key in dic_sort.keys():
        head_value = dic_sort[key][0]
        vals.append(head_value[1:])
    vals.sort()
    vals = np.array(vals)

    # mu := E[active_count_in_window / window_size]
    mu_mean = np.mean(vals[:,1])

    # mu_std := sqrt(V[active_count_in_window / window_size]) = sigma/sqrt(window_size)
    mu_std = np.std(vals[:,1])

    # upper_limit := window_size * mu + sqrt(2*window_size*sigma^2) erfinv
    # = cycle*mu + cycle*sqrt(2)*mu_std*erfinv
    upper_limit = cycle * (mu_mean + np.sqrt(2)*mu_std*scipy.special.erfinv(confidence))
    return upper_limit, len(vals)

