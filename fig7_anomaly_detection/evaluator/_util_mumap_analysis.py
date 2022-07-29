# Copyright 2022 NTT CORPORATION

import numpy as np

from _util_mumap import get_mu_map
from _util_content import get_content

def get_above_criteria(folder_name, threshold_list, cycle, ratio):
    mu_mat = get_mu_map(folder_name, cycle, ratio)
    result = []
    for threshold in threshold_list:
        mu_above = mu_mat >= threshold
        mu_above_ratio = np.sum(mu_above, axis=0)
        mu_above_mean = np.mean(mu_above_ratio)
        mu_above_std = np.std(mu_above_ratio)
        result.append((mu_above_mean, mu_above_std))
    return result


def get_above_count(folder_name, cycle, ratio, threshold_count):
    mu_map = get_mu_map(folder_name, cycle, ratio) * cycle
    mu_above = mu_map > threshold_count
    mu_above_count = np.sum(mu_above, axis=0)
    mu_above_count_mean = np.mean(mu_above_count)
    mu_above_count_std = np.std(mu_above_count)
    return mu_above_count_mean, mu_above_count_std


def get_anomaly_position(folder_name, threshold_list, cycle, ratio):
    mu_mat = get_mu_map(folder_name, cycle, ratio)
    keys = list(get_content(folder_name, cycle, ratio).keys())
    pos_list = np.array([(int(v[1]), int(v[2])) for v in keys])
    # [pos, xy]
    result = []
    for threshold in threshold_list:
        mu_above = mu_mat >= threshold
        pos_select = []
        for ind in range(mu_above.shape[1]):
            pos = (
                np.mean(pos_list[:,0], where=mu_above[:,ind]), 
                np.mean(pos_list[:,1], where=mu_above[:,ind])
                )
            pos_select.append(pos)
        pos_select = np.array(pos_select)
        pos_center_mean = np.mean(pos_select, axis=0)
        pos_center_std = np.std(pos_select, axis=0)
        result.append((pos_center_mean, pos_center_std))
    return result
