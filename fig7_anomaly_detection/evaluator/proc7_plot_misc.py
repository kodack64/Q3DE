# Copyright 2022 NTT CORPORATION

import matplotlib.pyplot as plt
import numpy as np
import pickle


nth = 20

with open("./result/_result_trajectory.pkl", "rb") as fin:
    result_trajectory_dict = pickle.load(fin)

with open("./result/_result_position_error.pkl", "rb") as fin:
    result_position_dict = pickle.load(fin)

with open("./result/_result_detection_latency.pkl", "rb") as fin:
    result_above_dict = pickle.load(fin)

# ratios = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
ratios = [10, 30, 50, 70]
plt.figure(figsize=(8, 6))

plt.subplot(2, 1, 1)
cmap = plt.get_cmap("tab10")
for ri, ratio in enumerate(ratios):
    x, y, ystd, ymin, ymax = result_trajectory_dict[ratio]
    plt.plot(x, y, label=f"ratio={ratio}", color=cmap(ri))
    # plt.fill_between(x, y-ystd, y+ystd, alpha=0.2, color=cmap(ri))
    plt.fill_between(x, ymin, ymax, alpha=0.2, color=cmap(ri))

plt.plot([500, 500], [0, 40], ":", color="black")
# plt.plot([1500, 1500], [0, 40], ":", color="black")
plt.plot([0, 1000], [nth, nth], color="black", label=r"$n_{\rm th}$")
plt.xlabel(r"Code cycle", fontsize=16)
plt.ylabel(r"Detected syndrome count", fontsize=16)
plt.xlim(0, 1000)
plt.ylim(0, 40)
plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
plt.xticks(fontsize=16)
plt.yticks(fontsize=16)
plt.legend(loc="upper left", fontsize=16)
# plt.tight_layout()
# plt.savefig("fig_anomaly_detection_latency.pdf")
# plt.show()

plt.subplot(2, 1, 2)
for ri, ratio in enumerate(ratios):
    x, result_mean_x, result_mean_y, result_med_x, result_med_y, mean_ano_x, mean_ano_y = result_position_dict[ratio]
    yabove = result_above_dict[ratio]
    # xtrace = np.nanmean(result_med_x, axis=1)
    # ytrace = np.nanmean(result_med_y, axis=1)
    xtrace = result_med_x
    ytrace = result_med_y
    dist = np.sqrt((xtrace-mean_ano_x)**2 + (ytrace-mean_ano_y)**2)
    print(dist.shape)
    print(yabove)
    for si in range(dist.shape[1]):
        dist[:yabove[si], si] = np.nan
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

plt.xlabel(r"Code cycle", fontsize=16)
plt.ylabel(r"Anomaly position error", fontsize=16)
plt.xlim(0, 1000)
plt.ylim(0, 10)
plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
plt.xticks(fontsize=16)
plt.yticks(fontsize=16)
plt.legend(loc="upper left", fontsize=16)


plt.tight_layout()
plt.savefig("./figure/fig_anomaly_detection_latency.pdf")
plt.show()


ratios = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
plt.figure(figsize=(8, 3))
cmap = plt.get_cmap("tab10")
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
    yabove = result_above_dict[ratio]
    x.append(ratio)
    y.append(np.mean(yabove) + required_window[int(ratio)][0] - 500)
    ymax.append(np.max(yabove) + required_window[int(ratio)][0] - 500)
    ymin.append(np.min(yabove) + required_window[int(ratio)][0] - 500)
    # y.append(np.mean(yabove) + required_window[int(ratio)][0] - 500)
plt.plot(x, y)
plt.fill_between(x, ymin, ymax, alpha=0.2)
plt.xlim(0, 100)
plt.ylim(0, 400)
plt.xlabel(r"Physical error ratio", fontsize=16)
plt.ylabel(r"Latency cycles", fontsize=16)
plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
plt.xticks(fontsize=16)
plt.yticks(fontsize=16)
# plt.legend(loc="upper left", fontsize=16)

plt.tight_layout()
plt.savefig("./figure/fig_anomaly_detection_latency_plot.pdf")
plt.show()
