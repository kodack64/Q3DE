# Copyright 2022 NTT CORPORATION

import matplotlib.pyplot as plt
import glob
import numpy as np

flist = []
flist += glob.glob("./data/*.txt")
print(flist)

a_list = set()
d_list = set()
p_list = set()
flag_list = set()
dat = {}
for fname in flist:
    fname = fname.replace("\\", "/")
    fin = open(fname)
    elem = fname.split("result_")[1].replace(".txt", "").split("_")
    print(fname, elem)
    d = int(elem[0])
    if d%6 != 3: continue
    a = int(elem[1])
    f = int(elem[2])
    p = float(elem[3])
    a_list.add(a)
    d_list.add(d)
    p_list.add(p)
    flag_list.add(f)

    sum_trial = 0.
    sum_fail = 0.
    for line in fin:
        trial, error = line.split()
        trial = int(trial)
        error = float(error)
        sum_trial += trial
        sum_fail += trial * error
    error_rate = sum_fail / sum_trial
    error_rate_std = np.sqrt(error_rate * (1 - error_rate) / sum_trial)
    dat[(d, a, f, p)] = (error_rate, error_rate_std)
    fin.close()

a_list = list(sorted(list(a_list)))
d_list = list(sorted(list(d_list)))
p_list = list(sorted(list(p_list)))
flag_list = list(sorted(list(flag_list)))


ls = ["--", "-", ":"]
cmap = plt.get_cmap("tab10")

def plot_anomaly(a_size, legend=False, xlabel=False):
    a_list = [0, a_size]
    for ai, a in enumerate(a_list):
        flag_ite = [0] if a==0 else [0]
        for flag in flag_ite:
            for di, d in enumerate(d_list):
                x = []
                y = []
                yerr = []
                for p in p_list:
                    if (d, a, flag, p) not in dat:
                        continue
                    x.append(p)
                    y.append(dat[d, a, flag, p][0])
                    yerr.append(dat[d, a, flag, p][1])
                if a==0: ls="-"
                elif flag==0: ls=":"
                else: ls="-."
                assert(flag==0)
                label = f"d={d} without MBBE" if a==0 else f"d={d} with MBBE"
                plt.errorbar(x, y, yerr=yerr, label=label, color=cmap((d - 9) // 6), linestyle=ls, capsize=2)
    if xlabel:
        plt.xlabel(r"Physical error rate", fontsize=16)
    plt.ylabel(r"Logical error rate", fontsize=16)
    plt.yscale("log")
    plt.xscale("log")
    plt.xlim(0.004, 0.04)
    plt.ylim(1e-5, 2e-1)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    plt.xticks(fontsize=14)
    plt.yticks(fontsize=14)
    plt.legend(loc="upper left", fontsize=10)

plt.figure(figsize=(8, 3))
plot_anomaly(4, True, True)
plt.tight_layout()
# plt.savefig("fig_decoder_below_threshold_norollback_micro.pdf")
plt.show()
