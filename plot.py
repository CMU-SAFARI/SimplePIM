
#[[framewok>>], [human>>]]
import matplotlib.pyplot as plt
import numpy as np

font = {
    'weight' : 'bold',
    'size'   : 30
    }

font_subtitle = {
    'weight' : 'bold',
    'size'   : 36
    }
plt.rcParams["figure.figsize"] = (100, 10)
plt.rc('font', **font)

colors=['#eee00f','#e00fff','#123456','#000999','#123fff','#1eff1f','#2edf4f','#2eaf9f',
    '#eeeff1','#eee112','#00ef00','#aa0000','#0000aa','#32efff','#23ef68','#2e3f56','#7eef1f','#eeef11']

weak_scaling={
    "Vector Addition":[[26.14, 26.27, 28.28],[29.53, 29.56, 29.92]], #1.1, 1.15
    "Reduction":[[(1.89, 20.58), (2.81, 20.642), (4.12, 20.94)],[(2.31, 20.55), (2.94, 20.67), (4.11, 20.89)]],
    "Histogram":[[(2.19, 55.20), (2.64, 55.34), (4.99, 55.62)],[(1.129, 54.68), (2.64, 55.34), (3.79, 54.97)]],
    "K-Means":[[(1.45, 110.55), (2.5, 110.7), (3.21, 112.99)],[(0.52, 155.7), (0.32, 159.19), (0.26, 153.38)]], #1.37, 1.43
    "Linear Regression":[[(2.03,47.04), (1.08,47.17), (2.16,48.886)],[(0.6,47.156), (0.7,47.22), (1.05,47.37)]],
    "Logistic Regression":[[(1.48, 88.74), (1.98, 88.75), (2.7, 91.27)],[(0.46, 107.24), (0.63, 107.29), (0.92, 107.34)]], #1.17, 1.22
}


strong_scaling ={
    "Vector Addition":[[26.14,  13.3, 7.18], [29.53, 15.30, 8.6]],
    "Reduction":[[(1.89, 20.58), (2.96, 10.46), (3.31, 5.44)],[(2.42, 20.52), ( 2.7, 10.52), (4.22, 5.62)]],
    "Histogram":[[(2.19, 55.20), (3.74, 28.25), (4.32, 14.55)],[(1.129, 54.68), (1.81, 32.68), (3.82, 18.1)]],
    "K-Means":[[(1.45, 110.55), (2.24, 55.72), (2.21, 32.35)],[(0.26, 153.38), (0.68, 87.08), (1.82, 50.0)]],
    "Linear Regression":[[(2.03,47.04), (1.61,23.86), (2.17,14.27)],[(0.6,47.156), (0.62,23.85), (0.87,19.80)]],
    "Logistic Regression":[[(1.48, 88.74), (2.21, 44.74), (2.23, 22.75)],[(0.46, 107.24), (0.64, 54.16), (1.1, 31.54)]],
}

zip_comparison = [[7.18, 13.3, 26.14, 52.04],[(10.97, 5.39), (20.91, 10.53), (41.39, 20.9), (81.093, 40.70)]]

kernel_color = 'lightgreen'
cup_red_color = 'gray'
sp_h_fontsize = 24

speed_up_human_color = "b"
speed_up_framework_color = "r"

plot_width = 2
markersize = 10
markers = ["s", "*"]

bar_edge_width = 2
nbins = 4

def plot_red_tradeoff():
    sp_h_fontsize = 28
    markersize = 18
    plt.rcParams["figure.figsize"] = (20, 8)
    title = "Comparison for Two In-Cache Reduction Schemes for Histogram Application"
    x = ["256", "512", "1024", "2048", "4096"]
    save_path = "plots/red_comparisons.pdf"
    x_label, y_label = "Histogram Size in Number of 32-bit Integers", "Time in ms"
    line_width = 0.23
    x_pos = np.arange(len(x))
    num_comparisons = 2
    kernel_time = np.array([[95, 95.76, 96.1, 97.1, 95.5],[55, 56, 78.9, 156.7, 292.2]])
    red_time = np.array([[6.48, 6.83, 6.41, 6.2, 6.7],[6.61, 7.68, 7.16, 7.31, 6.98]])

    fig, ax = plt.subplots(1, 1)


    i=0
    bars = ax.bar(x_pos - line_width + line_width*(i+0.5), kernel_time[i]+red_time[i], width=line_width, edgecolor='k', color=cup_red_color, linewidth=bar_edge_width, hatch="/", label = "CPU Time")
    ax.bar(x_pos - line_width + line_width*(i+0.5), kernel_time[i], width=line_width, edgecolor='k', color=kernel_color, linewidth=bar_edge_width, label = "PIM Kernel Time")
    for b in bars:
        height = b.get_height()
        text = "S" if i==0 else "P"
        shift = 0.03 if i==0 else -0.03
        ax.text(b.get_x()+b.get_width() / 2 - shift, height, text, ha = 'center', va = 'bottom', fontdict={'size': sp_h_fontsize})
    for i in range(1, num_comparisons):
        bars = ax.bar(x_pos - line_width + line_width*(i+0.5), kernel_time[i]+red_time[i], width=line_width, edgecolor='k', color=cup_red_color, linewidth=bar_edge_width, hatch="/")
        ax.bar(x_pos - line_width + line_width*(i+0.5), kernel_time[i], width=line_width, edgecolor='k', color=kernel_color, linewidth=bar_edge_width)
        #plot L or LF
        for b in bars:
            height = b.get_height()
            text = "S" if i==0 else "P"
            shift = 0.01 if i==0 else -0.01 
            ax.text(b.get_x()+b.get_width() / 2 - shift, height, text, ha = 'center', va = 'bottom', fontdict={'size': sp_h_fontsize})
    
    ax_twin = ax.twinx()
    ax_twin.plot(x_pos, [11.98, 11.98, 8, 4, 2], marker=markers[0], color = "b", linewidth=plot_width*2, markersize=markersize, label="#Threads: Private Version(P)")
    ax_twin.plot(x_pos, [12, 12, 12, 12, 12], marker=markers[1], color = "r", linewidth=plot_width*2, markersize=markersize, label="#Threads: Shared Version(S)")
    ax.set_xticks(x_pos)
    ax.set_xticklabels(x)

    #plt.suptitle(title, fontsize=30, fontweight="bold")

    lines_labels = [ax.get_legend_handles_labels() for ax in fig.axes]
    lines, labels = [sum(lol, []) for lol in zip(*lines_labels)]
    fig.legend(lines, labels, loc='lower right',fontsize=font['size']-3, bbox_to_anchor=(0.4735, 0.42, 0., 0))

    fig.text(0.25, 0.03, x_label, va='center', fontdict=font)
    fig.text(0.01, 0.5, y_label, va='center', rotation=90, fontdict=font)
    fig.text(0.98, 0.55, 'Number of Threads', va='center', rotation=90, fontdict=font)

    plt.subplots_adjust(
                    bottom=0.12,  
                    top=0.958, 
                    left=0.085,
                    right=0.94,
                    hspace=0.35,
                    wspace = 0.4,
                    )
    plt.savefig(save_path)
    #plt.show()
    plt.close()
        


def plot_scaling(run_weak_scaling = True):
    font = {
    'weight' : 'bold',
    'size'   : 28
    }


    title = "Weak Scaling Comparisons for SimplePIM and Human Code" if run_weak_scaling else "Strong Scaling Comparisons for SimplePIM and Human Code"

    data = weak_scaling if run_weak_scaling else strong_scaling
    save_path = "plots/weak_scaling.pdf" if run_weak_scaling else "plots/strong_scaling.pdf"

    x_label, y_label = "Number of PIM Cores", "Time in ms"

    line_width = 0.23
    x = ["608", "1216", "2432"]
    num_comparisons = 2
    x_pos = np.arange(len(x))

    fig, axs = plt.subplots(2, 3, figsize=(20, 10))
    #plt.suptitle(title, fontsize=30, fontweight="bold")

    handle = []

    # vector add
    ax = axs[0, 0]
    ax.set_ylim(0, 33)
    if(not run_weak_scaling):
        ax.yaxis.set_major_locator(plt.MaxNLocator(nbins))
        ax_twin = ax.twinx()
        ax_twin.yaxis.set_major_locator(plt.MaxNLocator(nbins))
        speedup_framework = (data["Vector Addition"][0][0]/np.array(data["Vector Addition"][0]))*[2**i for i in range(len(x))] if run_weak_scaling else data["Vector Addition"][0][0]/np.array(data["Vector Addition"][0])
        speedup_human = (data["Vector Addition"][1][0]/np.array(data["Vector Addition"][1]))*[2**i for i in range(len(x))] if run_weak_scaling else data["Vector Addition"][1][0]/np.array(data["Vector Addition"][1])
        ax_twin.plot(x_pos, speedup_framework, marker=markers[0], color = speed_up_framework_color, linewidth=plot_width, markersize=markersize, label="SimplePIM (SP) Speedup")
        ax_twin.plot(x_pos, speedup_human, marker=markers[1], color = speed_up_human_color, linewidth=plot_width, markersize=markersize, label="Hand-optimized Impl. (H) Speedup")
    for i in range(num_comparisons):
        label_kernel = "PIM Kernel Time" if i == 0 else ""
        label_red = "CPU Time" if i == 0 else ""
        ax.bar(x_pos - line_width + line_width*(i+0.5), np.zeros(len(data["Vector Addition"][i])), width=line_width, edgecolor='k', color=cup_red_color, linewidth=bar_edge_width, hatch="/", label = label_red)
        bars = ax.bar(x_pos - line_width + line_width*(i+0.5),data["Vector Addition"][i], width=line_width, edgecolor='k', color=kernel_color, linewidth=bar_edge_width, label =label_kernel)
        #plot st or human
        for b in bars:
            height = b.get_height()
            text = "SP" if i==0 else "H"
            shift = 0.04 if i==0 else -0.03
            ax.text(b.get_x()+b.get_width() / 2 - shift, height, text, ha = 'center', va = 'bottom', fontdict={'size':sp_h_fontsize})
    ax.set_title('Vector Addition', fontdict=font_subtitle)
    ax.set_xticks(x_pos)
    ax.set_xticklabels(x)

    apps = ["Reduction", "Histogram", "K-Means", "Linear Regression", "Logistic Regression"]
    # reduction
    for i in range(len(apps)):
        app_name = apps[i]
        ax = axs[((i+1)//3), (i+1)%3]
        framework_data, human_data = list(zip(*data[app_name][0])), list(zip(*data[app_name][1]))
        framework_total, human_total = np.array([i[0]+i[1] for i in data[app_name][0]]), np.array([i[0]+i[1] for i in data[app_name][1]])
        red_time = [np.array(framework_data[0]), np.array(human_data[0])]
        kernel_time = [np.array(framework_data[1]), np.array(human_data[1])]      
        ax.set_ylim(0, (np. asarray(red_time)+np. asarray(kernel_time)).max()*1.1)
        if(not run_weak_scaling):
            ax.yaxis.set_major_locator(plt.MaxNLocator(nbins))
            ax_twin = ax.twinx()
            ax_twin.yaxis.set_major_locator(plt.MaxNLocator(nbins))
            framework_data, human_data = list(zip(*data[app_name][0])), list(zip(*data[app_name][1]))
            framework_total, human_total = np.array([i[0]+i[1] for i in data[app_name][0]]), np.array([i[0]+i[1] for i in data[app_name][1]])
            red_time = [np.array(framework_data[0]), np.array(human_data[0])]
            kernel_time = [np.array(framework_data[1]), np.array(human_data[1])]
            speedup_framework = (framework_total[0]/framework_total)*[2**i for i in range(len(x))] if run_weak_scaling else framework_total[0]/framework_total
            speedup_human = (human_total[0]/human_total)*[2**i for i in range(len(x))] if run_weak_scaling else human_total[0]/human_total
            ax_twin.plot(x_pos, speedup_framework, marker=markers[0], color = speed_up_framework_color, linewidth=plot_width, markersize=markersize)
            ax_twin.plot(x_pos, speedup_human, marker=markers[1], color = speed_up_human_color, linewidth=plot_width, markersize=markersize)
        
        for i in range(num_comparisons):
            bars = ax.bar(x_pos - line_width + line_width*(i+0.5), kernel_time[i]+red_time[i], width=line_width, edgecolor='k', color=cup_red_color, linewidth=bar_edge_width, hatch="/")
            ax.bar(x_pos - line_width + line_width*(i+0.5), kernel_time[i], width=line_width, edgecolor='k', color=kernel_color, linewidth=bar_edge_width)
            #plot st or human
            for b in bars:
                height = b.get_height()
                text = "SP" if i==0 else "H"
                shift = 0.04 if i==0 else -0.03
                ax.text(b.get_x()+b.get_width() / 2 - shift, height, text, ha = 'center', va = 'bottom', fontdict={'size':sp_h_fontsize})
        
        ax.set_title(app_name, fontdict=font_subtitle)
        ax.set_xticks(x_pos)
        ax.set_xticklabels(x)
        
    font = {
    'weight' : 'bold',
    'size'   : 30
    }

    if(not run_weak_scaling):
        fig.text(0.95, 0.6, 'Speedup', va='center', rotation=90, fontdict=font)

    lines_labels = [ax.get_legend_handles_labels() for ax in fig.axes]
    lines, labels = [sum(lol, []) for lol in zip(*lines_labels)]
    if(not run_weak_scaling):
        fig.legend(lines, labels, loc='lower right',fontsize=font['size']-2, bbox_to_anchor=(0.87, -0.001, 0, 0), ncol=2)
        fig.text(0.02, 0.6, y_label, va='center', rotation=90, fontdict=font)
        fig.text(0.37, 0.163, x_label, va='center', fontdict=font)
        plt.subplots_adjust(
                    bottom=0.23,  
                    top=0.95,
                    left=0.1,
                    hspace=0.35,
                    wspace = 0.5,
                    )
    else:
        fig.legend(lines, labels, loc='lower right',fontsize=font['size']-2, bbox_to_anchor=(0.77, -0.01, 0, 0), ncol=2)
        fig.text(0.02, 0.6, y_label, va='center', rotation=90, fontdict=font)
        fig.text(0.42, 0.1, x_label, va='center', fontdict=font)
        plt.subplots_adjust(
                    bottom=0.17,  
                    top=0.95, 
                    left=0.1,
                    right=0.98,
                    hspace=0.35,
                    wspace = 0.4,
                    )
    
    #plt.tight_layout()
    plt.savefig(save_path)
    #plt.show()
    plt.close()

    

if __name__ == "__main__":
    plot_red_tradeoff()
    plot_scaling(True)
    plot_scaling(False)
