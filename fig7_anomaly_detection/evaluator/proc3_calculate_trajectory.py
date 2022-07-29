# Copyright 2022 NTT CORPORATION

import matplotlib.pyplot as plt
import tqdm
import glob
import numpy as np
import pickle

trajectory_folder = "data_trajectory"

def recalc_latency():
    required_window = {}
    with open("./result/_windowsize.txt", "r") as fin:
        for line in fin:
            ratio, size, threshold = line.split(" ")
            required_window[int(ratio)] = (int(size), float(threshold))

    flist = glob.glob(f"./{trajectory_folder}/*.txt")
    num_sample = 100
    d = 21
    num_synd = d*(d-1)
    ratios = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]

    result_dict = {}

    for fname in flist:
        pklname = fname.replace(".txt", ".pkl")
        try:
            fin = open(pklname, "rb")
            data = pickle.load(fin)
            fin.close()
        except:
            val = np.loadtxt(fname, dtype=np.int8)
            data = val.reshape([num_sample, 2, -1, num_synd])
            fout = open(pklname, "wb")
            pickle.dump(data, fout)
            fout.close()

        print(fname)
        ratio = int(fname.split(".")[-2].split("_")[-2].replace("ratio", ""))
        if ratio == 1:
            continue
        window, threshold = required_window[ratio]
        x = []
        y = []
        ymin = []
        ymax = []
        ystd = []
        for start in tqdm.tqdm(range(2000 - window - 1)):
            above_map = np.sum(
                data[:, :, start:start+window, :], axis=2) > threshold
            # above_map.shape = 100, 2, <>, 420 (sample, zx, window, positions)
            above_count = np.sum(above_map, axis=2)
            above_count = np.sum(above_count, axis=1)
            # print(ratio, start, np.mean(above_count))
            x.append(start+window)
            y.append(np.mean(above_count))
            ymin.append(np.min(above_count))
            ymax.append(np.max(above_count))
            ystd.append(np.std(above_count))
            if start+window+1 >= 2000:
                break
        x = np.array(x)
        y = np.array(y)
        ystd = np.array(ystd)
        ymin = np.array(ymin)
        ymax = np.array(ymax)
        findex = ratios.index(ratio)
        result_dict[ratio] = (x,y,ystd,ymin,ymax)

    with open("./result/_result_trajectory.pkl", "wb") as fout:
        pickle.dump(result_dict, fout)

nth = 20
recalc_latency()

