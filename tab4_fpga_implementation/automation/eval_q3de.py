

from vitis_hls_automation import generate_config, execute_vitis_hls, get_statistics


def evaluate_q3de_synthesis(queue_size: int,
                     consider_anomaly: bool,
                     clock_freq_MHz: float,
                     margin_ratio: float,
                     relative_path: str,
                     vitis_setting_path: str,
                     part_name: str,
                     overwrite: bool = False,
                     synth_id: int = 0
                     ) -> dict:
    """evaluate q3de codes

    Args:
        queue_size (int): Queue size. 40 or 80.
        consider_anomaly (bool): Use Q3DE architecture or not
        clock_freq_MHz (float): Clock frequency in MHz
        margin_ratio (float): Ratio of margin. Clock period at CSynth is reduced by (1 - margin_ratio) from implementation.
        relative_path (str): Relative path to Q3DE codes
        vitis_setting_path (str): Path to the settings64 of Vitis HLS.
        part_name (str): part name
        overwrite (bool, optional): If true, re-execute the synthesis even if analysis is already done. Defaults to False.
        synth_id (int, optional): If not None, used as an identifier. Defaults to 0.
        
    Returns:
        dict: result info
    """
    assert(queue_size in [40, 80])
    arc_str = "Q3DE" if consider_anomaly else "BASE"
    src_file_list = []
    src_file_list.append(f"{relative_path}/{queue_size}-{arc_str}/decoder.cpp")
    src_file_list.append(f"{relative_path}/{queue_size}-{arc_str}/decoder.h")
    project_name = f"{arc_str}_{queue_size}_{int(clock_freq_MHz)}_{int(margin_ratio*100)}_{synth_id}"
    top_func_name = "decoder"
    clock_period_impl_ns = 1000 / clock_freq_MHz
    clock_period_csynth_ns = clock_period_impl_ns * (1 - margin_ratio)

    config = generate_config(
        vitis_setting_path=vitis_setting_path,
        project_name=project_name,
        part_name=part_name,
        src_file_list=src_file_list,
        top_func_name=top_func_name,
        clock_period_csynth=clock_period_csynth_ns,
        clock_period_implementation=clock_period_impl_ns
    )

    if overwrite:
        execute_vitis_hls(config)
        result = get_statistics(config)

    else:
        # try fetch
        try:
            result = get_statistics(config)
        except FileNotFoundError:
            execute_vitis_hls(config)
            result = get_statistics(config)

    return result

