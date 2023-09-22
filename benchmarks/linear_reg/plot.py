from cProfile import label
import matplotlib.pyplot as plt
import json
import numpy as np
import scipy.stats
from math import log2, exp

colors=['#23ef68','#32efff','#2eaf9f','#22222f','#eeeff1','#eee112','#00ef00','#aa0000','#0000aa','#000999','#2e3f56','#7eef1f','#eeef11']

font = {'weight' : 'bold',
        'size'   : 16}
plt.rcParams["figure.figsize"] = (16, 7)
plt.rc('font', **font)


def load_data(data, cpu_files, DPU_files, DPU_MASTER_files):
    for i in range(len(cpu_files)):
        data["CPU"][i] = np.loadtxt(cpu_files[i]) /1000

    for i in range(len(DPU_files)):
        tmp = np.loadtxt(DPU_files[i])/1000
        data["DPU_initial_transfer"][i] = tmp[0]
        data["DPU_Kernel"][i] = tmp[1]
        data["DPU_D2C"][i] = tmp[2]
        data["DPU_C2D"][i] = tmp[4]
        data["DPU"][i] = tmp[5]
    
    for i in range(len(DPU_MASTER_files)):
        tmp = np.loadtxt(DPU_MASTER_files[i])/1000
        data["DPU_MASTER_initial_transfer"][i] = tmp[0]
        data["DPU_MASTER_Kernel"][i] = tmp[1]
        data["DPU_MASTER_D2C"][i] = tmp[2]
        data["DPU_MASTER_C2D"][i] = tmp[4]
        data["DPU_MASTER"][i] = tmp[5]


    


dim_data={
    "title":"varing_input_dimension",
    "x_name":"input dimension",
    "x_axis":["5", "10", "20"],
    "CPU":np.zeros(3),

    "DPU":np.zeros(3),
    "DPU_Kernel":np.zeros(3),
    "DPU_initial_transfer":np.zeros(3),
    "DPU_C2D":np.zeros(3),
    "DPU_D2C":np.zeros(3),

    "DPU_MASTER":np.zeros(3),
    "DPU_MASTER_Kernel": np.zeros(3),
    "DPU_MASTER_initial_transfer": np.zeros(3),
    "DPU_MASTER_C2D": np.zeros(3),
    "DPU_MASTER_D2C": np.zeros(3),

}


num_data={
    "title":"varing_#data",
    "x_name":"number of input data points",
    "x_axis":["100000", "1000000", "10000000"],
    "CPU":np.zeros(3),

    "DPU":np.zeros(3),
    "DPU_Kernel":np.zeros(3),
    "DPU_initial_transfer":np.zeros(3),
    "DPU_C2D":np.zeros(3),
    "DPU_D2C":np.zeros(3),

    "DPU_MASTER":np.zeros(3),
    "DPU_MASTER_Kernel": np.zeros(3),
    "DPU_MASTER_initial_transfer": np.zeros(3),
    "DPU_MASTER_C2D": np.zeros(3),
    "DPU_MASTER_D2C": np.zeros(3),
}

num_dpus_data={
    "title":"varing_#dpus",
    "x_name":"number of dpus",
    "x_axis":["128", "512", "2048"],
    "CPU":np.zeros(3),

    "DPU":np.zeros(3),
    "DPU_Kernel":np.zeros(3),
    "DPU_initial_transfer":np.zeros(3),
    "DPU_C2D":np.zeros(3),
    "DPU_D2C":np.zeros(3),

    "DPU_MASTER":np.zeros(3),
    "DPU_MASTER_Kernel": np.zeros(3),
    "DPU_MASTER_initial_transfer": np.zeros(3),
    "DPU_MASTER_C2D": np.zeros(3),
    "DPU_MASTER_D2C": np.zeros(3),
}


def mean_confidence_interval(data, confidence=0.99):
    a = 1.0 * np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.sem(a)
    h = se * scipy.stats.t.ppf((1 + confidence) / 2., n-1)
    return h

def concate(arr_list):
    arr_list = [x.reshape(len(x), 1) for x in arr_list] 
    return np.concatenate(arr_list, axis = 1)

def plot_res(data):
    x = data["x_axis"]
    title = data["title"]
    _, ax = plt.subplots()

    line_width = 0.25
    x_pos = np.arange(len(x))
    
    
    bar1 = ax.bar(x_pos - line_width + line_width*0, data["CPU"], width=line_width, edgecolor='k', color=colors[0], label ="CPU version")

    bar2 = ax.bar(x_pos - line_width + line_width*1, data["DPU"], width=line_width, edgecolor='k', color=colors[1], label ="CPU-reduce")
    bar4 = ax.bar(x_pos - line_width + line_width*1, data["DPU_Kernel"]+data["DPU_initial_transfer"]+data["DPU_D2C"]+data["DPU_C2D"], width=line_width, edgecolor='k', color=colors[2], label="C2D transfer")
    bar5 = ax.bar(x_pos - line_width + line_width*1, data["DPU_Kernel"]+data["DPU_initial_transfer"]+data["DPU_D2C"], width=line_width, edgecolor='k', color=colors[3], label="D2C transfer")
    bar6 = ax.bar(x_pos - line_width + line_width*1, data["DPU_Kernel"]+data["DPU_initial_transfer"], width=line_width, edgecolor='k', color=colors[4], label="initial transfer")
    bar7 = ax.bar(x_pos - line_width + line_width*1, data["DPU_Kernel"], width=line_width, edgecolor='k', color=colors[5], label="DPU kernel")

    bar3 = ax.bar(x_pos - line_width + line_width*2, data["DPU_MASTER"], width=line_width, edgecolor='k', color=colors[1])
    ax.bar(x_pos - line_width + line_width*2, data["DPU_MASTER_Kernel"]+data["DPU_MASTER_initial_transfer"]+data["DPU_MASTER_D2C"]+data["DPU_MASTER_C2D"], width=line_width, edgecolor='k', color=colors[2])
    ax.bar(x_pos - line_width + line_width*2, data["DPU_MASTER_Kernel"]+data["DPU_MASTER_initial_transfer"]+data["DPU_MASTER_D2C"], width=line_width, edgecolor='k', color=colors[3])
    ax.bar(x_pos - line_width + line_width*2, data["DPU_MASTER_Kernel"]+data["DPU_MASTER_initial_transfer"], width=line_width, edgecolor='k', color=colors[4])
    ax.bar(x_pos - line_width + line_width*2, data["DPU_MASTER_Kernel"], width=line_width, edgecolor='k', color=colors[5])



    for i in range(len(bar1 + bar2 +bar3)):
        rect = (bar1 + bar2 +bar3)[i]
        height = rect.get_height()
        if(not height == 0):
            if i//3 == 0:
                text = "cpu"
            elif i//3 == 1:
                text = "pim-f"
            else:
                text = "pim-h"
            plt.text(rect.get_x()+rect.get_width() / 2, height, text, ha = 'center', va = 'bottom', fontdict={'size': 16})
    
    #plt.yscale('log',base=2)
    ax.set_xticks(x_pos-line_width*0)
    ax.set_xticklabels(x) 
    ax.set_title(title)
    plt.xlabel(data["x_name"], fontdict=font)
    plt.ylabel("time in s", fontdict=font)

    legend1 = plt.legend(handles=[bar1, bar2], loc='upper left', shadow=True, bbox_to_anchor=(0, -0.12, 0, 0))
    ax.add_artist(legend1)
    legend2 = plt.legend(handles=[bar3, bar4, bar7], loc='upper left', shadow=True, bbox_to_anchor=(0.425, -0.12, 0, 0))
    ax.add_artist(legend2)
    plt.legend(handles=[bar5, bar6], loc='upper left', shadow=True, bbox_to_anchor=(0.2, -0.12, 0, 0))
    plt.savefig("images/"+title, bbox_inches='tight')
    plt.clf()
    plt.close()
    



if __name__=="__main__":
    dir = "results/"
    load_data(dim_data, [dir+ i for i in ["cpu_5_1000000.csv", "cpu_10_1000000.csv", "cpu_20_1000000.csv"]], [dir+i for i in ["framework_2523_5_1000000", "framework_2523_10_1000000", "framework_2523_20_1000000"]], [dir+i for i in ["human_2523_5_1000000", "human_2523_10_1000000"]])
    plot_res(dim_data)
    load_data(num_data, [dir+ i for i in ["cpu_10_100000.csv", "cpu_10_1000000.csv", "cpu_10_10000000.csv"]], [dir+i for i in ["framework_2523_10_100000", "framework_2523_10_1000000", "framework_2523_10_10000000"]], [dir+i for i in ["human_2523_10_100000", "human_2523_10_1000000",  "human_2523_10_10000000"]])
    plot_res(num_data)
    load_data(num_dpus_data, [], [dir+i for i in ["framework_128_10_1000000", "framework_512_10_1000000", "framework_2048_10_1000000"]], [dir+i for i in ["human_128_10_1000000",  "human_512_10_1000000", "human_2048_10_1000000"]])
    plot_res(num_dpus_data)
