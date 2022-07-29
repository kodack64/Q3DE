# Copyright 2022 NTT CORPORATION

import numpy as np
import glob
from collections import defaultdict
import matplotlib.pyplot as plt

flist = glob.glob("./data/*.txt")

data_dict = defaultdict(list)
for fname in flist:
    fin = open(fname)
    for line in fin:
        elem = line.split(" ")
        width = int(elem[0])
        trial = int(elem[1])
        assert(width==7)
        assert(trial==10000)
        ano_prob = float(elem[2])
        ano_time = float(elem[3])
        throughput = trial/int(elem[4])
        key = (ano_prob, ano_time)
        data_dict[key].append(throughput)

stat_dict = defaultdict(list)
ref = []
for key, val in data_dict.items():
    stat_dict[key[1]].append( (key[0], np.mean(val), np.std(val)) )
    if key[0] == 0:
        ref = np.mean(val)

plt.figure(figsize=(3,2.5))
plt.plot([1e-6, 1e-4], [ref, ref], "--", label="MBBE free", color="black")
plt.plot([1e-6, 1e-4], [ref/2, ref/2], "-", label="baseline", color="black")
for key, val in stat_dict.items():
    val = np.array(sorted(val))
    label = r"$\tau_{\rm ano}/(d \tau_{\rm cyc})$ = " + str(key)
    plt.errorbar(val[:,0], val[:,1], val[:,2], label=label, capsize=2)
plt.xlabel(r"Cosmic ray frequency $d \tau_{\rm cyc} f_{\rm ano}$", fontsize=12)
plt.ylabel("Instruction throughput", fontsize=12)
plt.xscale("log")
# plt.yscale("log")
plt.xlim(1e-6, 1e-4)
plt.ylim(0, 6)
plt.grid(which='major', color='black', linestyle='-', alpha=0.2)
plt.grid(which='minor', color='black', linestyle='-', alpha=0.2)
plt.xticks(fontsize=12)
plt.yticks(fontsize=12)
plt.legend(loc="lower left", fontsize=9)
plt.tight_layout()
plt.savefig("fig_scalability_instruction_throughput_micro.pdf")
plt.show()




