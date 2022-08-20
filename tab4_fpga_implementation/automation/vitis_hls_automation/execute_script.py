# Copyright 2022 NTT CORPORATION

import subprocess
import os
from .config import Config


def _create_working_directory(folder_name: str) -> None:
    """Move to a given folder and create it if it does not not exist

    Args:
        folder_name (str): folder name
    """
    if not os.path.exists(folder_name):
        os.mkdir(folder_name)
    os.chdir(folder_name)


def _leave_working_directory() -> None:
    """Leave working directory
    """
    os.chdir("../")


def _get_vitis_hls_script(config: Config) -> str:
    """Generate string of Vitis HLS script

    Args:
        config (Config): config

    Returns:
        str: script string
    """
    template_script_lines = []
    template_script_lines.append(f"open_project {config.project_name}")
    template_script_lines.append(f"set_top {config.top_func_name}")

    for file_name in config.src_file_list:
        template_script_lines.append(f"add_files {file_name}")
    for tbench_file_name in config.src_file_list_tbench:
        template_script_lines.append(f"add_files -tb {tbench_file_name}")

    template_script_lines.append(f"open_solution \"{config.solution_name}\" -flow_target vivado")
    part_name_fix = "{" + config.part_name + "}"
    template_script_lines.append(f"set_part {part_name_fix}")
    template_script_lines.append(f"create_clock -period {config.clock_period_csynth} -name default")
    template_script_lines.append(f"config_export -format ip_catalog -rtl verilog -vivado_clock {config.clock_period_implementation}")

    if config.execute_csim:
        template_script_lines.append("csim_design")
    template_script_lines.append("csynth_design")
    if config.execute_cocsim:
        template_script_lines.append("cosim_design")
    template_script_lines.append("export_design -flow impl -rtl verilog -format ip_catalog")
    template_script_lines.append("exit")

    script_str = "\n".join(template_script_lines) + "\n"
    return script_str


def _get_vitis_hls_shell_script_windows(config: Config) -> str:
    """Get windows shell script string to execute Vitis HLS script

    Args:
        config (Config): config

    Returns:
        str: script
    """
    script_lines = []
    windows_path = config.vitis_setting_path.replace("/", "\\")
    script_lines.append(f"call {windows_path}")
    script_lines.append(f"call vitis_hls -f {config.vitis_hls_script_name}")
    script_str = "\n".join(script_lines) + "\n"
    return script_str


def _get_vitis_hls_shell_script_linux(config: Config) -> str:
    """Get linux shell script string to execute Vitis HLS script

    Args:
        config (Config): config

    Returns:
        str: script
    """
    script_lines = []
    linux_path = config.vitis_setting_path.replace("\\", "/")
    script_lines.append(f"{linux_path}")
    script_lines.append(f"vitis_hls -f {config.vitis_hls_script_name} > {config.log_file_name}.txt")
    script_str = "\n".join(script_lines) + "\n"
    return script_str


def _generate_script_file(config: Config) -> None:
    """Generate required script to execute Vitis HLS

    Args:
        config (Config): config
    """
    vitis_hls_script = _get_vitis_hls_script(config)
    if os.name == "nt":
        vitis_hls_shell_script = _get_vitis_hls_shell_script_windows(config)
    else:
        vitis_hls_shell_script = _get_vitis_hls_shell_script_linux(config)

    with open(config.vitis_hls_script_name, "w") as fout:
        fout.write(vitis_hls_script)

    with open(config.vitis_hls_shell_name, "w") as fout:
        fout.write(vitis_hls_shell_script)


def execute_vitis_hls(config: Config) -> None:
    """Create Vitis HLS script files

    Args:
        config (Config): config
    """
    _create_working_directory(config.project_name)
    _generate_script_file(config)
    try:
        args = [config.vitis_hls_shell_name]
        subprocess.run(args, shell=False, capture_output=False)
    finally:
        _leave_working_directory()

