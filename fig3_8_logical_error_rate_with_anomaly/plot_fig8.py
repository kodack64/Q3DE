# Copyright 2022 NTT CORPORATION

import matplotlib
import matplotlib.pyplot as plt
import glob
import numpy as np
import uncertainties
import uncertainties.umath

flist = []
flist += glob.glob("./data2/*.txt")
flist += glob.glob("./data/*.txt")
print(flist)

# allowed uncertainty
allowed_unc = 4

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
    # 1-(1-p_L)^d = er
    # p_L = 1-(1-er)^(1/d)
    error_rate = 1 - (1 - error_rate) ** (1. / d)
    error_rate_std = np.sqrt(error_rate * (1 - error_rate) / sum_trial)
    
    dat[(d, a, f, p)] = (error_rate, error_rate_std)
    fin.close()

a_list = list(sorted(list(a_list)))
d_list = list(sorted(list(d_list)))
p_list = list(sorted(list(p_list)))
flag_list = list(sorted(list(flag_list)))

ls = ["--", "-", ":"]
cmap = plt.get_cmap("tab10")

def plot_anomaly(fig, ax, a_size, legend=False, xlabel=False, ylabel=False, title=False):
    a_list = [0, a_size]
    for ai, a in enumerate(a_list):
        flag_ite = [0] if a==0 else [0,1]
        for flag in flag_ite:
            for di, d in enumerate(d_list):
                if d % 6 != 3:
                    continue
                x = []
                y = []
                yerr = []
                for p in p_list:
                    if (d, a, flag, p) not in dat:
                        continue
                    x.append(p)
                    y.append(dat[d, a, flag, p][0])
                    yerr.append(dat[d, a, flag, p][1])
                # s = "With rollback" if flag else "Without rollback"
                # alpha = 1.0 if a!=0 else 0.4
                if a==0: ls="-"
                elif flag==0: ls=":"
                else: ls="-."
                print(a,flag)
                if a==0:
                    label = f"d={d} MBBE free"
                elif flag==0:
                    label = f"d={d} without rollback"
                else:
                    label = f"d={d} with rollback"
                # plt.plot(x, y, label=label, color=cmap((d - 9) // 2), linestyle=ls)
                plt.errorbar(x, y, yerr=yerr, label=label, color=cmap((d - 9) // 2), linestyle=ls, capsize=2)
    if title:
        plt.title(f"Anomaly size = {a_size}", fontsize=12)
    if xlabel:
        plt.xlabel(r"Physical error rate", fontsize=12)
    if ylabel:
        plt.ylabel(r"Logical error rate", fontsize=12)
    plt.yscale("log")
    plt.xscale("log")
    plt.xlim(0.004, 0.04)
    plt.ylim(1e-5, 2e-1)
    plt.yticks(fontsize=10)
    plt.xticks([0.004, 0.01, 0.04], fontsize=10)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    if legend:
        plt.legend(loc="upper left", fontsize=10, bbox_to_anchor=(1.0, 1.0))


def plot_curve_fit(fig, ax, a_size, legend=False, xlabel=False, ylabel=False, title=False):
    for di, d in enumerate(d_list):
        x = []
        d_dec_ano = []
        d_dec_opt = []
        d_dec_ano_err = []
        d_dec_opt_err = []
        for p in p_list:
            key_num = (d,0,0,p)
            key_div = (d-2,0,0,p)
            key_ano = (d,a_size,0,p)
            key_opt = (d,a_size,1,p)
            
            print(key_num, key_div)
            print(key_num not in dat)
            print(key_div not in dat)
            print(key_ano not in dat)
            print(key_opt not in dat)


            if key_num not in dat or key_div not in dat or key_ano not in dat or key_opt not in dat:
                continue
            y_num = uncertainties.ufloat(dat[key_num][0], dat[key_num][1])
            y_div = uncertainties.ufloat(dat[key_div][0], dat[key_div][1])
            y_ano = uncertainties.ufloat(dat[key_ano][0], dat[key_div][1])
            y_opt = uncertainties.ufloat(dat[key_opt][0], dat[key_div][1])
            if y_num ==0 or y_div==0 or y_ano==0 or y_opt == 0:
                continue
            y_val1 = 0.5 * uncertainties.umath.log(y_num/y_div)
            y_val2 = uncertainties.umath.log(y_num/y_ano)
            y_val3 = uncertainties.umath.log(y_num/y_opt)
            if y_val1.n ==0 or y_val2.n==0 or y_val3.n==0:
                continue
            d_dec_ano_val = y_val2/y_val1
            d_dec_opt_val = y_val3/y_val1
            # print(d_dec_ano_val)
            # print(d_dec_opt_val)
            if d_dec_ano_val.s > allowed_unc or d_dec_opt_val.s > allowed_unc:
                continue
            x.append(p)
            d_dec_ano.append(d_dec_ano_val.n)
            d_dec_opt.append(d_dec_opt_val.n)
            d_dec_ano_err.append(d_dec_ano_val.s/4)
            d_dec_opt_err.append(d_dec_opt_val.s/4)
        if len(x)==0:
            continue
        if d >= 18:
            continue
        # plt.plot(x, d_dec_ano, label=f"d={d} with anomaly", color=cmap((d - 9) // 2), linestyle=":")
        # plt.plot(x, d_dec_opt, label=f"d={d} optimized", color=cmap((d - 9) // 2), linestyle="-.")
        plt.errorbar(x, d_dec_ano, yerr=d_dec_ano_err, label=f"d={d} without rollback", color=cmap((d - 9) // 2), linestyle=":", capsize=2)
        plt.errorbar(x, d_dec_opt, yerr=d_dec_opt_err, label=f"d={d} with rollback", color=cmap((d - 9) // 2), linestyle="-.", capsize=2)
    if title:
        plt.title(f"Anomaly size = {a_size}", fontsize=12)
    if xlabel:
        plt.xlabel(r"Physical error rate", fontsize=12)
    if ylabel:
        plt.ylabel(r"Code distance reduction", fontsize=12)
    plt.xscale("log")
    plt.xlim(0.004, 0.04)
    plt.ylim(0, 15)
    plt.yticks(fontsize=10)
    plt.xticks([0.004, 0.01, 0.04], fontsize=10)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    if legend:
        plt.legend(loc="upper left", fontsize=10, bbox_to_anchor=(1.0, 1.0))

fig = plt.figure(figsize=(8, 6))

ax = fig.add_subplot(2,2,1)
plot_anomaly(fig, ax, 2, False, False, True, True)
ax.get_xaxis().set_minor_formatter(matplotlib.ticker.NullFormatter())

ax = fig.add_subplot(2,2,2)
plot_anomaly(fig, ax, 4, True, False, False, True)
ax.get_xaxis().set_minor_formatter(matplotlib.ticker.NullFormatter())

ax = fig.add_subplot(2,2,3)
plot_curve_fit(fig, ax, 2, False, True, True, False)
ax.get_xaxis().set_minor_formatter(matplotlib.ticker.NullFormatter())

ax = fig.add_subplot(2,2,4)
plot_curve_fit(fig, ax, 4, True, True, False, False)
ax.get_xaxis().set_minor_formatter(matplotlib.ticker.NullFormatter())

plt.tight_layout()
plt.savefig("fig_decoder_below_threshold_micro.pdf")
plt.show()
