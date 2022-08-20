# Copyright 2022 NTT CORPORATION

import os
from .config import Config
import xml.etree.ElementTree as ET


def _get_first_item_from_xml(xml_path: str, item_list: list[str]) -> dict:
    """Get first attributes of item list in XML at a given path

    Args:
        xml_path (str): path of XML files
        item_list (list[str]): list of attributions

    Raises:
        FileNotFoundError: XML file not found

    Returns:
        dict: result dictionary
    """
    if not os.path.exists(xml_path):
        raise FileNotFoundError(f"File {xml_path} not found")
    xml_tree = ET.parse(xml_path)
    result = {}
    for item in item_list:
        vals = list(xml_tree.iter(item))
        if len(vals) == 0:
            raise Exception(f"item {item} not found in file: {xml_path}")
        val = vals[0]
        result[item] = val.text
    return result


def _get_csynth_result(config: Config) -> dict:
    """Fetch CSynth reuslt

    Args:
        config (Config): config

    Returns:
        dict: result
    """
    xml_path = f"./{config.project_name}/{config.project_name}/{config.solution_name}/syn/report/csynth.xml"
    item_list = [
        "TargetClockPeriod",
        "ClockUncertainty",
        "EstimatedClockPeriod",
        "Worst-caseLatency",
    ]
    result = _get_first_item_from_xml(xml_path, item_list)
    return result


def _get_implementation_result(config: Config) -> dict:
    """Fetch Implementation reuslt

    Args:
        config (Config): config

    Returns:
        dict: result
    """
    xml_path = f"./{config.project_name}/{config.project_name}/{config.solution_name}/impl/report/verilog/export_impl.xml"
    item_list = [
        "TargetClockPeriod",
        "AchievedClockPeriod",
        "CP_FINAL",
        "CP_ROUTE",
        "CP_SYNTH",
        "CP_TARGET",
        "TIMING_MET",
    ]
    result = _get_first_item_from_xml(xml_path, item_list)
    assert(float(result["TargetClockPeriod"]) == float(result["CP_TARGET"]))
    assert(float(result["CP_ROUTE"]) == float(result["CP_FINAL"]))
    assert(float(result["CP_ROUTE"]) == float(result["AchievedClockPeriod"]))
    return result


def _get_performance(result: dict) -> dict:
    """Calculate resutl dictionary

    Args:
        result (dict): result including those of csynth and implementation

    Returns:
        dict: result
    """
    target_clock_period_synth = float(result["csynth"]["TargetClockPeriod"])
    target_clock_period_impl = float(result["implementation"]["TargetClockPeriod"])
    latency_cycle = int(result["csynth"]["Worst-caseLatency"])
    clock_period = float(result["implementation"]["AchievedClockPeriod"])
    timing_met = (result["implementation"]["TIMING_MET"] != "FALSE")
    matching_cycle = 1000
    latency_per_match_ns = latency_cycle / matching_cycle * clock_period
    code_cycle_ns = 1000
    match_per_cycle = code_cycle_ns / latency_per_match_ns
    result_perf = {
        "target_clock_period_synth": target_clock_period_synth,
        "target_clock_period_impl": target_clock_period_impl,
        "code_cycle_ns": code_cycle_ns,
        "matching_cycle": matching_cycle,
        "timing_met": timing_met,
        "latency_clock_cycle_per_match": latency_cycle/matching_cycle,
        "clock_period_ns": clock_period,
        "match_per_code_cycle": match_per_cycle
    }
    return result_perf


def get_statistics(config: Config) -> dict:
    """Get statistics of given config

    Args:
        config (Config): config

    Returns:
        dict: result dict
    """
    result_csynth = _get_csynth_result(config)
    result_impl = _get_implementation_result(config)
    result = {"csynth": result_csynth, "implementation": result_impl}
    result["performance"] = _get_performance(result)
    return result
