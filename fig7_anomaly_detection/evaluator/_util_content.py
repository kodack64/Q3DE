# Copyright 2022 NTT CORPORATION

import pickle
from typing import Any
from collections import defaultdict
import numpy as np
import scipy.special
import os

from _util_fileproperty import get_file_from_prop

def _get_content_pkl(folder_name: str, cycle: int, ratio: int) -> Any:
    folder_name = folder_name + "_content_pkl__"
    if not os.path.exists(folder_name):
        os.mkdir(folder_name)
    fname = get_file_from_prop(folder_name, cycle, ratio)
    fname = fname.replace(".txt", ".pkl")
    try:
        return pickle.load(open(fname, "rb"))
    except:
        return None


def _save_content_pkl(obj: Any, folder_name: str, cycle: int, ratio: int) -> None:
    folder_name = folder_name + "_content_pkl__"
    fname = get_file_from_prop(folder_name, cycle, ratio)
    fname = fname.replace(".txt", ".pkl")
    fout = open(fname, "wb")
    pickle.dump(obj, fout)
    fout.close()


def _get_content_txt(folder_name: str, cycle: int, ratio: int, confidence: float = 0.99) -> dict:
    fname = get_file_from_prop(folder_name, cycle, ratio)
    fin = open(fname, "r")
    dic_sort = defaultdict(list)
    for line in fin:
        key = tuple(line.split(" ")[:3])
        count = int(line.split(" ")[3])
        mu = count/cycle
        mu_range = np.sqrt(2 * mu * (1-mu)/cycle) * scipy.special.erfinv(confidence)
        dic_sort[key].append((count, mu, mu_range))
    return dic_sort


def get_content(folder_name: str, cycle: int, ratio: int) -> dict:
    """Get formatted content of dataset

    Args:
        folder_name (str): folder name
        cycle (int): cycle
        ratio (int): ratio

    Returns:
        dict: dict of dataset. Key is a tuple of (X_or_Z, x_pos, y_pos).  
            Value is a list of tuples. 
            The i-th tuple contains the following values from the i-th simulation.
            (count, frequency_of_detection_per_cycle, and range with confidence 99%)
    """
    obj = _get_content_pkl(folder_name, cycle, ratio)
    if obj is not None:
        return obj
    obj = _get_content_txt(folder_name, cycle, ratio)
    _save_content_pkl(obj, folder_name, cycle, ratio)
    return obj

