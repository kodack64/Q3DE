# Copyright 2022 NTT CORPORATION

import matplotlib.pyplot as plt
import tqdm
import glob
import numpy as np
import pickle

trajectory_folder = "data_trajectory"

def recalc_position():
    fposlist = glob.glob(f"./{trajectory_folder}/*.txt.pos")
    fposfile = fposlist[0]
    try:
        fin = open("./result/_pospick.pkl", "rb")
        pospick = pickle.load(fin)
        fin.close()
    except:
        posmap = np.loadtxt(fposfile, dtype=np.int32)
        pospick = np.vstack([posmap[210000:210420],posmap[1050000:1050420]])
        with open("./result/_pospick.pkl", "wb") as fout:
            pickle.dump(pospick, fout)

    print(pospick)
    ano_x = []
    ano_y = []
    for row in pospick:
        if row[3]==1:
            ano_x.append(row[0])
            ano_y.append(row[1])
    mean_ano_x = np.mean(ano_x)
    mean_ano_y = np.mean(ano_y)
    print(mean_ano_x, mean_ano_y, len(ano_x))


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
        fin = open(pklname, "rb")
        data = pickle.load(fin)
        fin.close()

        ratio = int(fname.split(".")[-2].split("_")[-2].replace("ratio", ""))
        if ratio == 1:
            continue
        window, threshold = required_window[ratio]
        x = []
        y = []
        ymin = []
        ymax = []
        ystd = []
        result_mean_x = []
        result_mean_y = []
        result_med_x = []
        result_med_y = []
        for start in tqdm.tqdm(range(2000 - window - 1)):
            above_map = np.sum(
                data[:, :, start:start+window, :], axis=2) > threshold
            # above_map.shape = 100, 2, <>, 420
            xposlist = []
            yposlist = []
            xmedlist = []
            ymedlist = []
            for index in range(100):
                vec = np.concatenate([above_map[index][0],above_map[index][1]])
                above_indices = np.where(vec)
                xpos = np.mean(pospick[above_indices][:,0])
                ypos = np.mean(pospick[above_indices][:,1])
                xmed = np.median(pospick[above_indices][:,0])
                ymed = np.median(pospick[above_indices][:,1])
                xposlist.append(xpos)
                yposlist.append(ypos)
                xmedlist.append(xmed)
                ymedlist.append(ymed)
            x.append(start+window)
            result_mean_x.append(xposlist)
            result_mean_y.append(yposlist)
            result_med_x.append(xmedlist)
            result_med_y.append(ymedlist)
            if start+window+1 >= 2000:
                break
        x = np.array(x)
        result_mean_x = np.array(result_mean_x)
        result_mean_y = np.array(result_mean_y)
        result_med_x = np.array(result_med_x)
        result_med_y = np.array(result_med_y)
        result_dict[ratio] = (x, result_mean_x, result_mean_y, 
        result_med_x, result_med_y, mean_ano_x, mean_ano_y)

    with open("./result/_result_position_error.pkl", "wb") as fout:
        pickle.dump(result_dict, fout)

nth = 20
recalc_position()


