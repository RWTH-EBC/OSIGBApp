# -*- coding: utf-8 -*-
import csv
import datetime
from matplotlib import pyplot as plt
import numpy as np
import os
import pandas as pd

path = os.path.join(os.getcwd(), 'data_files', 'TemperaturePerformanceTest.csv')
file_exists = os.path.isfile(path)

def retrieve_and_plot():
    print(path)
    data = csv.DictReader(open(path.replace('\\', "/")))
    temp = [row for row in data]
    df = pd.DataFrame(temp)
    df_selection={}
    for i in df['timestamp']:
        if datetime.datetime.fromisoformat(i) >= datetime.datetime.fromisoformat('2022-02-16T14:35:00.000'):
            j = df.index[df['timestamp'] == i].to_list()[0]
            break
    df_selection['0'] = df[j-600:j]
    for i in df['timestamp']:
        if datetime.datetime.fromisoformat(i) >= datetime.datetime.fromisoformat('2022-02-16T15:05:00.000'):
            j = df.index[df['timestamp'] == i].to_list()[0]
            break
    df_selection['10'] = df[j-600:j]
    for i in df['timestamp']:
        if datetime.datetime.fromisoformat(i) >= datetime.datetime.fromisoformat('2022-02-16T15:35:00.000'):
            j = df.index[df['timestamp'] == i].to_list()[0]
            break
    df_selection['20'] = df[j-600:j]
    for i in df['timestamp']:
        if datetime.datetime.fromisoformat(i) >= datetime.datetime.fromisoformat('2022-02-16T16:05:00.000'):
            j = df.index[df['timestamp'] == i].to_list()[0]
            break
    df_selection['30'] = df[j-600:j]
    for i in df['timestamp']:
        if datetime.datetime.fromisoformat(i) >= datetime.datetime.fromisoformat('2022-02-16T16:35:00.000'):
            j = df.index[df['timestamp'] == i].to_list()[0]
            break
    df_selection['40'] = df[j-600:j]
    for i in df['timestamp']:
        if datetime.datetime.fromisoformat(i) >= datetime.datetime.fromisoformat('2022-02-16T17:05:00.000'):
            j = df.index[df['timestamp'] == i].to_list()[0]
            break
    df_selection['50'] = df[j-600:j]
    for i in df['timestamp']:
        if datetime.datetime.fromisoformat(i) >= datetime.datetime.fromisoformat('2022-02-16T17:30:00.000'):
            j = df.index[df['timestamp'] == i].to_list()[0]
            break
    df_selection['60'] = df[j-600:j]
    for i in df['timestamp']:
        if datetime.datetime.fromisoformat(i) >= datetime.datetime.fromisoformat('2022-02-16T17:55:00.000'):
            j = df.index[df['timestamp'] == i].to_list()[0]
            break
    df_selection['70'] = df[j-600:j]
    for i in df['timestamp']:
        if datetime.datetime.fromisoformat(i) >= datetime.datetime.fromisoformat('2022-02-16T18:20:00.000'):
            j = df.index[df['timestamp'] == i].to_list()[0]
            break
    df_selection['80'] = df[j-600:j]
    for i in df['timestamp']:
        if datetime.datetime.fromisoformat(i) >= datetime.datetime.fromisoformat('2022-02-16T18:50:00.000'):
            j = df.index[df['timestamp'] == i].to_list()[0]
            break
    df_selection['90'] = df[j-600:j]
    try:
        # first try
        plt.close()
        plt.plot(np.array(df['Temperature Gateway'], dtype=np.float64), label='Temperature Gateway')
        plt.plot(np.array(df['Temperature PLC'], dtype=np.float64), label='Temperature PLC')
        plt.legend()
        plt.show()
        # plt.savefig('TemperaturePerformance.pdf')
        # second try
        # plt.close()
        # data_plc = [df_selection[i]['Temperature PLC'].astype(float) for i in df_selection]
        # data_gw = [df_selection[i]['Temperature Gateway'].astype(float) for i in df_selection]
        # ticks = [i+' °C' for i in df_selection]
        # plt.figure()
        # bxp_plc = plt.boxplot(data_plc, positions=np.array(range(len(data_plc)))*2.0-0.3, sym='', widths=0.5)
        # bxp_gw = plt.boxplot(data_gw, positions=np.array(range(len(data_gw)))*2.0+0.3, sym='', widths=0.5)
        # plt.xticks(range(0, len(ticks) * 2, 2), ticks)
        # plt.xlim(-2, len(ticks)*2)
        # plt.tight_layout()
        # third try
        plt.close()
        labels=['PLC', 'TGW']
        widths=0.8
        ub = 0.5
        lb = 0.7
        stepticks = 0.2
        fig, ((ax1, ax2, ax3, ax4, ax5), (ax6, ax7, ax8, ax9, ax10)) = plt.subplots(nrows=2,ncols=5)
        fig.subplots_adjust(hspace=0.2, wspace=0.45)
        ax1.boxplot([df_selection['0']['Temperature PLC'].astype(float), 
                     df_selection['0']['Temperature Gateway'].astype(float)],
                    labels=labels, widths=widths)
        ax1.axhline(y=0.0, color='grey', linestyle='dashed')
        center = 0.0
        ax1.yaxis.set_ticks(np.arange(center-lb, center+0.6, step=stepticks))
        ax2.boxplot([df_selection['10']['Temperature PLC'].astype(float), 
                     df_selection['10']['Temperature Gateway'].astype(float)],
                    labels=labels, widths=widths)
        ax2.axhline(y=10.0, color='grey', linestyle='dashed')
        center = 10.0
        ax2.yaxis.set_ticks(np.arange(center-lb, center+0.6, step=stepticks))
        ax3.boxplot([df_selection['20']['Temperature PLC'].astype(float), 
                     df_selection['20']['Temperature Gateway'].astype(float)],
                    labels=labels, widths=widths)
        ax3.axhline(y=20.0, color='grey', linestyle='dashed')
        center = 20.0
        ax3.yaxis.set_ticks(np.arange(center-lb, center+0.6, step=stepticks))
        ax4.boxplot([df_selection['30']['Temperature PLC'].astype(float), 
                     df_selection['30']['Temperature Gateway'].astype(float)],
                    labels=labels, widths=widths)
        ax4.axhline(y=30.0, color='grey', linestyle='dashed')
        center = 30.0
        ax4.yaxis.set_ticks(np.arange(center-lb, center+0.6, step=stepticks))
        ax5.boxplot([df_selection['40']['Temperature PLC'].astype(float), 
                     df_selection['40']['Temperature Gateway'].astype(float)],
                    labels=labels, widths=widths)
        ax5.axhline(y=40.0, color='grey', linestyle='dashed')
        center = 40.0
        ax5.yaxis.set_ticks(np.arange(center-lb, center+ub, step=stepticks))
        ax6.boxplot([df_selection['50']['Temperature PLC'].astype(float), 
                     df_selection['50']['Temperature Gateway'].astype(float)],
                    labels=labels, widths=widths)
        ax6.axhline(y=50.0, color='grey', linestyle='dashed')
        center = 50.0
        ax6.yaxis.set_ticks(np.arange(center-lb, center+ub, step=stepticks))
        ax7.boxplot([df_selection['60']['Temperature PLC'].astype(float), 
                     df_selection['60']['Temperature Gateway'].astype(float)],
                    labels=labels, widths=widths)
        ax7.axhline(y=60.0, color='grey', linestyle='dashed')
        center = 60.0
        ax7.yaxis.set_ticks(np.arange(center-lb, center+ub, step=stepticks))
        ax8.boxplot([df_selection['70']['Temperature PLC'].astype(float), 
                     df_selection['70']['Temperature Gateway'].astype(float)],
                    labels=labels, widths=widths)
        ax8.axhline(y=70.0, color='grey', linestyle='dashed')
        center = 70.0
        ax8.yaxis.set_ticks(np.arange(center-lb, center+ub, step=stepticks))
        ax9.boxplot([df_selection['80']['Temperature PLC'].astype(float), 
                     df_selection['80']['Temperature Gateway'].astype(float)],
                    labels=labels, widths=widths)
        ax9.axhline(y=80.0, color='grey', linestyle='dashed')
        center = 80.0
        ax9.yaxis.set_ticks(np.arange(center-lb, center+ub, step=stepticks))
        ax10.boxplot([df_selection['90']['Temperature PLC'].astype(float), 
                      df_selection['90']['Temperature Gateway'].astype(float)],
                    labels=labels, widths=widths)
        ax10.axhline(y=90.0, color='grey', linestyle='dashed')
        center = 90.0
        ax10.yaxis.set_ticks(np.arange(center-lb, center+ub, step=stepticks))
        fig.set_size_inches(10, 6)
        fig.suptitle('Temperature Gateway Performance in °C')
        plt.tight_layout()
        plt.show()
        plt.savefig(os.getcwd().replace('\\', '/')+'/results_temperature_performance.pdf',
                    dpi=200)
    except KeyboardInterrupt:
        print('Aborted plotting.')
    except Exception as e:
        print(e)
    return df, df_selection

#%%
if __name__ == '__main__':
    df, df_selection = retrieve_and_plot()
    med_gw  = {i: np.median([df_selection[i]['Temperature Gateway'].astype(float)]) for i in df_selection}
    med_plc = {i: np.median([df_selection[i]['Temperature PLC'].astype(float)]) for i in df_selection}
    med_diff = np.array(list(med_gw.values())) - np.array(list(med_plc.values()))
    avg_dev = np.mean([df[498:16365]['Temperature Gateway'].astype(float) - df[498:16365]['Temperature PLC'].astype(float)])