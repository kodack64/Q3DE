# Copyright 2022 NTT CORPORATION

import numpy as np
import matplotlib.pyplot as plt

def load_ano(q3de, size):
    fname = f"./result/result_size_{q3de}_{size}.txt"
    fin = open(fname)
    x = []
    y = []
    for line in fin:
        elems = line.split()[2:]
        try:
            area = float(elems[0])
            req_den = float(elems[1])
        except:
            continue
        x.append(area)
        y.append(req_den)
    fin.close()
    return x,y

def load_lifetime(q3de, lifetime):
    if lifetime==1:
        return load_ano(q3de, 8)
    fname = f"./result/result_lifetime_{q3de}_{lifetime}.txt"
    fin = open(fname)
    x = []
    y = []
    for line in fin:
        elems = line.split()[2:]
        try:
            area = float(elems[0])
            req_den = float(elems[1])
        except:
            continue
        x.append(area)
        y.append(req_den)
    fin.close()
    return x,y

def load_freq(q3de, freq):
    if freq==1:
        return load_ano(q3de, 8)
    fname = f"./result/result_freq_{q3de}_{freq}.txt"
    fin = open(fname)
    x = []
    y = []
    for line in fin:
        elems = line.split()[2:]
        try:
            area = float(elems[0])
            req_den = float(elems[1])
        except:
            continue
        x.append(area)
        y.append(req_den)
    fin.close()
    return x,y

def plot_lifetime(q3de, lifetime, cidx):
    cmap = plt.get_cmap("tab10")
    name = "Q3DE" if q3de else "baseline"
    ls = "-" if q3de else ":"
    x,y = load_lifetime(q3de,lifetime)
    label = f"{name} error duration x{lifetime}"
    plt.plot(x,y,label=label,c=cmap(cidx),ls=ls,marker="*")

def plot_lifetime_q3de():
    cmap = plt.get_cmap("tab10")
    name = "Q3DE"
    ls = "-"
    x,y = load_ano(True, 8)
    label = f"{name}"
    plt.plot(x,y,label=label,c=cmap(0),ls=ls,marker="*")

def plot_ano(q3de, size):
    cmap = plt.get_cmap("tab10")
    name = "Q3DE" if q3de else "baseline"
    ls = "-" if q3de else ":"
    x,y = load_ano(q3de,size)

    # The definition of anomaly size in the paper is the half of Manhattan size of anomaly
    label = f"{name} anomaly size={size//2}"
    plt.plot(x,y,label=label,c=cmap(4-size//2),ls=ls,marker="*")

def plot_freq(q3de, freq, cidx):
    cmap = plt.get_cmap("tab10")
    name = "Q3DE" if q3de else "baseline"
    ls = "-" if q3de else ":"
    x,y = load_freq(q3de, freq)
    label = f"{name} anomaly freq x{1/freq}"
    plt.plot(x,y,label=label,c=cmap(cidx),ls=ls,marker="*")


def plot_ano_all():
    plot_ano(True, 8)
    plot_ano(True, 6)
    plot_ano(True, 4)
    plot_ano(True, 2)
    plot_ano(False, 8)
    plot_ano(False, 6)
    plot_ano(False, 4)
    plot_ano(False, 2)
    plt.xscale("log")
    plt.yscale("log")
    plt.xlim(1,1e2)
    plt.ylim(1,1e3)
    plt.xlabel("Chip area ratio", fontsize=12)
    plt.ylabel("Qubit density ratio", fontsize=12)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    plt.xticks(fontsize=12)
    plt.yticks(fontsize=12)
    plt.legend(loc="upper right", fontsize=7, bbox_to_anchor=(1,1))

def plot_lifetime_all():
    plot_lifetime_q3de()
    plot_lifetime(False, 1., 0)
    plot_lifetime(False, 0.1, 1)
    plot_lifetime(False, 0.01, 2)
    plt.xscale("log")
    plt.yscale("log")
    plt.xlim(1,1e2)
    plt.ylim(1,1e3)
    plt.xlabel("Chip area ratio", fontsize=12)
    plt.ylabel("Qubit density ratio", fontsize=12)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    plt.xticks(fontsize=12)
    plt.yticks(fontsize=12)
    plt.legend(loc="upper right", fontsize=7, bbox_to_anchor=(1,1))

def plot_freq_all():
    plot_freq(True, 1, 0)
    plot_freq(True, 10, 1)
    plot_freq(True, 100, 2)
    plot_freq(False, 1, 0)
    plot_freq(False, 10, 1)
    plot_freq(False, 100, 2)
    plt.xscale("log")
    plt.yscale("log")
    plt.xlim(1,1e2)
    plt.ylim(1,1e3)
    plt.xlabel("Chip area ratio", fontsize=12)
    plt.ylabel("Qubit density ratio", fontsize=12)
    plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
    plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
    plt.xticks(fontsize=12)
    plt.yticks(fontsize=12)
    plt.legend(loc="upper right", fontsize=7, bbox_to_anchor=(1,1))

plt.figure(figsize=(8, 4))
plt.subplot(1,3,1)
plot_ano_all()
plt.subplot(1,3,2)
plot_lifetime_all()
plt.subplot(1,3,3)
plot_freq_all()
plt.tight_layout()
plt.savefig("fig_applicability_sweep.pdf")
plt.show()
