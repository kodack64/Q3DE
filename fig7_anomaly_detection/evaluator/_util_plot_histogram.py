# Copyright 2022 NTT CORPORATION

import matplotlib.pyplot as plt
import numpy as np
from _util_content import get_content, get_threshold_count


def plot_subfig_cumulative(folder_name, cycle, ratio, label, color):
    """Plot the cumulative plot with the first file.
    """
    dic_sort = get_content(folder_name, cycle, ratio)

    vals = []
    for key in dic_sort.keys():
        head_value = dic_sort[key][0]
        vals.append(head_value[1:])
    vals.sort()
    vals = np.array(vals)

    y = np.arange(len(vals))
    x = vals[:, 0] * cycle
    x_range = vals[:, 1] * cycle
    plt.plot(x, y, label=label, color=color)
    # plt.fill_betweenx(y, x+x_range, x-x_range, alpha=0.1, color=color)


def plot_subfig_threshold(folder_name, cycle):
    """Plot the threshold to the cumulative plot
    """
    upper_limit, size = get_threshold_count(folder_name, cycle)
    y = [0, size]
    x = [upper_limit, upper_limit]
    plt.plot(x, y, ":", label="threshold", color="black")
