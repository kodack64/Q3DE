# Copyright 2022 NTT CORPORATION

import numpy as np
import matplotlib.pyplot as plt
import pickle

with open("./result/_result_trajectory.pkl", "rb") as fin:
    result_trajectory_dict = pickle.load(fin)

with open("./result/_result_position_error.pkl", "rb") as fin:
    result_position_error_dict = pickle.load(fin)

with open("./result/_result_detection_latency.pkl", "rb") as fin:
    result_detection_latency_dict = pickle.load(fin)

with open("./result/_result_anomaly_detection_perf.pkl", "rb") as fin:
    result_anomaly_dict = pickle.load(fin)


def plot_anomaly_detection_rate():
    ratios = [1, 10, 30, 50, 70]
    cmap = plt.get_cmap("tab10")
    for ri, ratio in enumerate(ratios):
        cycles = result_anomaly_dict[ratio]["ratios"]
        above_count = result_anomaly_dict[ratio]["above_count"]
        above_count_std = result_anomaly_dict[ratio]["above_count_std"]
        if ratio == 1:
            label = "normal state"
            plt.plot(cycles, above_count, ":", label=label, color="black")
            plt.fill_between(cycles, above_count-above_count_std, above_count + above_count_std, alpha=0.2, color="black")
        else:
            label = f"ratio={ratio}"
            plt.plot(cycles, 1-above_count, label=label, color=cmap(ri-1))
            plt.fill_between(cycles, 1-above_count-above_count_std, 1-above_count + above_count_std, alpha=0.2, color=cmap(ri-1))
        plt.xscale("log")
        plt.yscale("log")
        plt.xlim(10,1000)
        plt.ylim(1e-3,1e0)
        plt.xlabel("Window size", fontsize=14)
        plt.ylabel("Detection error rate", fontsize=14)
        plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
        plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
        plt.xticks(fontsize=16)
        plt.yticks(fontsize=16)
        # plt.legend(loc="upper right", fontsize=16, bbox_to_anchor=(1.4, 1.05))
        plt.legend(loc="upper right", fontsize=12)


def plot_required_window_size():
    """
    plt.xlim(0,100)
    plt.ylim(0,500)
    plt.xlabel(r"Ratio of physical error rate", fontsize=16)
    plt.ylabel(r"Required window size", fontsize=14)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    plt.xticks(fontsize=16)
    plt.yticks(fontsize=16)
    """

def plot_detection_latency():
    cmap = plt.get_cmap("tab10")
    x = result_anomaly_dict["window"]["ratios"]
    y = result_anomaly_dict["window"]["window_sizes"]
    plt.plot(x, y, label="Required window size", color=cmap(0))

    ratios = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
    x = []
    y = []
    ymax = []
    ymin = []
    required_window = {}
    with open("./result/_windowsize.txt", "r") as fin:
        for line in fin:
            ratio, size, threshold = line.split(" ")
            required_window[int(ratio)] = (int(size), float(threshold))
    for ri, ratio in enumerate(ratios):
        yabove = result_detection_latency_dict[ratio]
        x.append(ratio)
        y.append(np.mean(yabove) + required_window[int(ratio)][0] - 500)
        ymax.append(np.max(yabove) + required_window[int(ratio)][0] - 500)
        ymin.append(np.min(yabove) + required_window[int(ratio)][0] - 500)
        # y.append(np.mean(yabove) + required_window[int(ratio)][0] - 500)
    plt.plot(x, y, label="Latency", color=cmap(1))
    plt.fill_between(x, ymin, ymax, alpha=0.2, color=cmap(1))
    plt.xlim(0, 100)
    plt.ylim(0, 500)
    plt.xlabel("Ratio of physical error rates", fontsize=14)
    plt.ylabel("Code cycles", fontsize=14)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    plt.xticks(fontsize=16)
    plt.yticks(fontsize=16)
    plt.legend(loc="upper right", fontsize=12)

def plot_trajectory_count():
    ratios = [10, 30, 50, 70]
    nth = 20
    cmap = plt.get_cmap("tab10")
    for ri, ratio in enumerate(ratios):
        x,y,ystd,ymin,ymax = result_trajectory_dict[ratio]
        plt.plot(x, y, label=f"ratio={ratio}", color=cmap(ri))
        # plt.fill_between(x, y-ystd, y+ystd, alpha=0.2, color=cmap(ri))
        plt.fill_between(x, ymin, ymax, alpha=0.2, color=cmap(ri))

    plt.plot([500, 500], [0, 40], ":", color="black")
    # plt.plot([1500, 1500], [0, 40], ":", color="black")
    plt.plot([0, 1000], [nth, nth], color="black", label=r"$n_{\rm th}$")
    plt.xlabel("Code cycles", fontsize=14)
    plt.ylabel("Active syndrome count", fontsize=14)
    plt.xlim(0, 1000)
    plt.ylim(0, 40)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    plt.xticks(fontsize=16)
    plt.yticks(fontsize=16)
    plt.legend(loc="upper left", fontsize=12)

def plot_trajectory_position():
    # ratios = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
    ratios = [10, 30, 50, 70]
    cmap = plt.get_cmap("tab10")
    for ri, ratio in enumerate(ratios):
        x, result_mean_x, result_mean_y, result_med_x, result_med_y, mean_ano_x, mean_ano_y = result_position_error_dict[ratio]
        yabove = result_detection_latency_dict[ratio]
        # xtrace = np.nanmean(result_med_x, axis=1)
        # ytrace = np.nanmean(result_med_y, axis=1)
        xtrace = result_med_x
        ytrace = result_med_y
        dist = np.sqrt((xtrace-mean_ano_x)**2 + (ytrace-mean_ano_y)**2)
        print(dist.shape)
        print(yabove)
        for si in range(dist.shape[1]):
            dist[:yabove[si],si] = np.nan
        dist_mean = np.nanmean(dist, axis=1)
        dist_std = np.nanstd(dist, axis=1)
        dist_max = np.nanmax(dist, axis=1)
        dist_min = np.nanmin(dist, axis=1)
        plt.plot(x, dist_mean, label=f"ratio={ratio}", color=cmap(ri))
        # plt.fill_between(x, y-ystd, y+ystd, alpha=0.5, color=cmap(ri))
        # plt.fill_between(x, dist_mean, dist_mean+dist_std, alpha=0.2, color=cmap(ri))
        plt.fill_between(x, dist_min, dist_max, alpha=0.2, color=cmap(ri))
    plt.plot([500, 500], [0, 40], ":", color="black")
    # plt.plot([1500, 1500], [0, 40], ":", color="black")
    center_dist = np.sqrt(5**2 + 5**2)
    plt.plot([0, 1000], [center_dist, center_dist], color="black", label="distance to center")

    plt.xlabel("Code cycles", fontsize=14)
    plt.ylabel("Anomaly position error", fontsize=14)
    plt.xlim(0, 1000)
    plt.ylim(0, 10)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    plt.xticks(fontsize=16)
    plt.yticks(fontsize=16)
    plt.legend(loc="upper left", fontsize=12)

def plot_trajectory_position2():
    ratios = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
    error_mean = []
    error_min = []
    error_max = []
    error_std = []
    cmap = plt.get_cmap("tab10")
    for ri, ratio in enumerate(ratios):
        x, result_mean_x, result_mean_y, result_med_x, result_med_y, mean_ano_x, mean_ano_y = result_position_error_dict[ratio]
        yabove = result_detection_latency_dict[ratio]
        # xtrace = np.nanmean(result_med_x, axis=1)
        # ytrace = np.nanmean(result_med_y, axis=1)
        xtrace = result_med_x
        ytrace = result_med_y
        dist = np.sqrt((xtrace-mean_ano_x)**2 + (ytrace-mean_ano_y)**2)

        first_above_error = []
        for si in range(dist.shape[1]):
            first_above_error.append(dist[yabove[si], si])
        print(ratio, first_above_error)
        error_mean.append(np.mean(first_above_error))
        error_min.append(np.min(first_above_error))
        error_max.append(np.max(first_above_error))
        error_std.append(np.std(first_above_error))

    center_dist = np.sqrt(5**2 + 5**2)
    # plt.plot([0, max(ratios)], [center_dist, center_dist], color="black", label="distance to center")

    error_mean = np.array(error_mean)
    error_std = np.array(error_std)

    plt.plot(ratios, error_mean, color=cmap(1))
    # plt.fill_between(ratios, error_min, error_max, alpha=0.2, color=cmap(1))
    plt.fill_between(ratios, error_mean-error_std, error_mean+error_std, alpha=0.2, color=cmap(1))
    plt.xlim(0, 100)
    plt.ylim(0, 5)
    plt.xlabel("Ratio of physical error rates", fontsize=14)
    plt.ylabel("Position error", fontsize=14)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    plt.xticks(fontsize=16)
    plt.yticks(fontsize=16)
    # plt.legend(loc="upper right", fontsize=12)

plt.figure(figsize=(8,3))
# plt.subplot(1,2,1)
# plot_anomaly_detection_rate()

plt.subplot(1,2,1)
plot_detection_latency()
plot_required_window_size()

# plt.subplot(2,2,2)
# plot_trajectory_count()
plt.subplot(1,2,2)
# plot_trajectory_position()
plot_trajectory_position2()
plt.tight_layout()
plt.savefig("./figure/fig_anomaly_detection_all_micro.pdf")
plt.show()

