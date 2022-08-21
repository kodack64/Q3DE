# Copyright 2022 NTT CORPORATION

from __future__ import annotations
import os
from dataclasses import dataclass


@dataclass
class Config:
    vitis_setting_path: str

    project_name: str
    solution_name: str
    part_name: str

    vitis_hls_script_name: str
    vitis_hls_shell_name: str

    src_file_list: list[str]
    top_func_name: str
    src_file_list_tbench: list[str]

    clock_period_csynth: float
    clock_period_implementation: float

    execute_csim: bool
    execute_cocsim: bool


def generate_config(
        vitis_setting_path: str,
        project_name: str,
        part_name: str,
        src_file_list: list[str],
        top_func_name: str,
        clock_period_csynth: float,
        clock_period_implementation: float,
        execute_csim: bool = False,
        execute_cocsim: bool = False,
        vitis_hls_script_name: str = None,
        vitis_hls_shell_name: str = None,
        solution_name: str = "solution1",
        src_file_list_tbench: list[str] = [],
        ) -> Config:
    """Generate config

    Args:
        vitis_setting_path (str): path to settings64.sh or settings64.bat
        project_name (str): project name
        part_name (str): part name
        src_file_list (list[str]): list of source files
        top_func_name (str): top function name
        clock_period_csynth (float): clock period of csynth
        clock_period_implementation (float): clock period of implementation

        vitis_hls_script_name (str): script name of Vitis HLS. If None, use the same name as project name. Defaults to None.
        vitis_hls_shell_name (str): shell script name to execute Vitis HLS script. If None, use the same name as project name. Defaults to None.
        src_file_list_tbench (list[str]): list of tbench source files. Defaults to [].
        solution_name (str): solution name. Defaults to "solution1"
        execute_csim (bool, optional): If true, execute csim. Defaults to False.
        execute_cocsim (bool, optional): If true, execute cocsim. Defaults to False.

    Returns:
        Config: config
    """
    if vitis_hls_script_name is None:
        vitis_hls_script_name = project_name + ".tcl"
    if vitis_hls_shell_name is None:
        vitis_hls_shell_name = project_name + (".bat" if os.name == "nt" else ".sh")
    config = Config(
        vitis_setting_path=vitis_setting_path,
        project_name=project_name,
        solution_name=solution_name,
        part_name=part_name,
        vitis_hls_script_name=vitis_hls_script_name,
        vitis_hls_shell_name=vitis_hls_shell_name,
        src_file_list=src_file_list,
        top_func_name=top_func_name,
        src_file_list_tbench=src_file_list_tbench,
        clock_period_csynth=clock_period_csynth,
        clock_period_implementation=clock_period_implementation,
        execute_csim=execute_csim,
        execute_cocsim=execute_cocsim,
    )
    return config
