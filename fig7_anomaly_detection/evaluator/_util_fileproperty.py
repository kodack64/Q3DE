# Copyright 2022 NTT CORPORATION

import glob
from typing import Tuple

def get_file_prop(file_name: str) -> Tuple[int, int]:
    """Get cycle and ratio setting of file from its name

    Args:
        file_name (str): filename

    Returns:
        tuple: pair of cycle and ratio
    """
    ratio = int(file_name.split("result")[
                1].split("_")[1].replace("ratio", ""))
    cycle = int(file_name.split("result")[1].split("_")[
                2].replace("cycle", "").replace(".txt", ""))
    return cycle, ratio


def get_file_prop_list(folder_name: str) -> Tuple[list[int], list[int]]:
    """Get cycles and ratios contained in the folder

    Args:
        folder_name (str): folder name

    Returns:
        Tuple[list[int], list[int]]: pair of list of cycle and ratio
    """
    flist = glob.glob(f"./{folder_name}/*.txt")
    ratios = set()
    cycles = set()
    for fname in flist:
        cycle, ratio = get_file_prop(fname)
        cycles.add(cycle)
        ratios.add(ratio)
    cycles = list(sorted(list(cycles)))
    ratios = list(sorted(list(ratios)))
    return cycles, ratios


def get_file_from_prop(folder_name: str, cycle: int, ratio: int) -> str:
    """Get filepath from settings

    Args:
        folder_name (str): folder name
        cycle (int): cycle
        ratio (int): ratio

    Returns:
        str: relative path to data file
    """
    fname = f"./{folder_name}/result_ratio{ratio}_cycle{cycle}.txt"
    return fname

