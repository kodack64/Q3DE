# Copyright 2022 NTT CORPORATION

import numpy as np
import pickle
from _util_content import get_file_from_prop, get_content
import os

def _get_mu_map_pkl(folder_name, cycle, ratio):
    folder_name = folder_name + "_mumap_pkl__"
    if not os.path.exists(folder_name):
        os.mkdir(folder_name)
    fname = get_file_from_prop(folder_name, cycle, ratio)
    fname = fname.replace(".txt", ".mumap_pkl")
    try:
        return pickle.load(open(fname, "rb"))
    except:
        return None


def _save_mu_map_pkl(obj, folder_name, cycle, ratio):
    folder_name = folder_name + "_mumap_pkl__"
    fname = get_file_from_prop(folder_name, cycle, ratio)
    fname = fname.replace(".txt", ".mumap_pkl")
    fout = open(fname, "wb")
    pickle.dump(obj, fout)
    fout.close()


def get_mu_map(folder_name: str, cycle: int, ratio: int) -> np.ndarray:
    """Get mu_map of dataset

    Args:
        folder_name (str): folder name 
        cycle (int): cycle
        ratio (int): ratio

    Returns:
        np.ndarray: two-dimensional map of frequency_per_cycle.
            value[i,j] (i-th row and j-th column) indicates value for i-th syndrome and j-th sample.
            Thus, value.shape = (syndrome_count, sample_count)
    """
    mu_mat = _get_mu_map_pkl(folder_name, cycle, ratio)
    if mu_mat is not None:
        return mu_mat

    dic_sort = get_content(folder_name, cycle, ratio)
    mu_mat = []
    for key in dic_sort:
        mus = [val[1] for val in dic_sort[key]]
        mu_mat.append(mus)
    mu_mat = np.array(mu_mat)
    _save_mu_map_pkl(mu_mat, folder_name, cycle, ratio)
    return mu_mat
