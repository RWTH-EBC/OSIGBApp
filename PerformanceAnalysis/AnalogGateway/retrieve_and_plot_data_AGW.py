# -*- coding: utf-8 -*-
import csv
import datetime
from matplotlib import pyplot as plt
import numpy as np
import os
import pandas as pd
import statistics


path_CO  = os.path.join(os.getcwd(), 'data_files', 'AnalogPerformanceTest-CO.csv')
path_CI  = os.path.join(os.getcwd(), 'data_files', 'AnalogPerformanceTest-CI.csv')

path_VO  = os.path.join(os.getcwd(), 'data_files', 'AnalogPerformanceTest-VO.csv')
path_VI  = os.path.join(os.getcwd(), 'data_files', 'AnalogPerformanceTest-VI.csv')
# file_exists = os.path.isfile(path)

# data =csv.reader(open(path))
# temp = [row for row in data]
currentCO = pd.read_csv(path_CO.replace('\\', '/'))
currentGW = pd.read_csv(path_CI)

voltageVO = pd.read_csv(path_VO)
voltageVI = pd.read_csv(path_VI)

# print(list(currentCO.columns.values))
print(currentCO.head())

# test = pd.read_csv(path)
# print(list(test.columns.values))

def calcDeviation(x,y):
    if x > y:
        res = x-y
    else:
        res = y-x
    return res

plt.close()
plt.plot(np.array(currentCO['Current Gateway'], dtype=np.float64), label='Current Gateway')
plt.plot(np.array(currentCO['Current PLC'], dtype=np.float64), label='Current PLC')
plt.legend()
plt.show()

CurrentValues=range(0,21,1)
# print(CurrentValues)
df_selection = {}
labels=['Output', 'Input']
sections=['PLC', 'GW']
for section in sections:
    df_selection[section] = {}
for i in CurrentValues:
    df_selection['PLC'][i] = currentCO[currentCO['Current Gateway']==i]
    df_selection['PLC'][i] = df_selection['PLC'][i][3:]
    df_selection['GW'][i] = currentGW[currentGW['Current PLC']==i]
    df_selection['GW'][i] = df_selection['GW'][i][3:]

num = 0
print(df_selection['GW'][num])
print(df_selection['PLC'][num])

plt.close()
plt.figure(figsize=(10, 10))
plt.subplots_adjust(hspace=0.5)
plt.rcParams.update({'font.size': 15})
widths=0.8
ub = 0.3
lb = 0.25
stepticks = 0.1

currentval = {}
currentval['minPLC'] = []
currentval['maxPLC'] = []
currentval['minGW'] = []
currentval['maxGW'] = []
meancleanout=[]
meancleanin=[]

for n, current in enumerate(range(0,21,2)):
    
    ax = plt.subplot(3,4, n + 1)
   
    ax.boxplot([df_selection['PLC'][current]['Current PLC'].astype(float), 
                df_selection['GW'][current]['Current Gateway'].astype(float)],
                    labels=labels, widths=widths)
    ax.axhline(y=current, color='grey', linestyle='dashed')
    center = current
    ax.yaxis.set_ticks(np.arange(center-lb, center+ub, step=stepticks))

    currentval[current]= {}
    currentval['maxPLC'].append(calcDeviation(df_selection['PLC'][current]['Current PLC'].max(),current))
    currentval['minPLC'].append(calcDeviation(df_selection['PLC'][current]['Current PLC'].min(),current))
    currentval['maxGW'].append(calcDeviation(df_selection['GW'][current]['Current Gateway'].max(),current))
    currentval['minGW'].append(calcDeviation(df_selection['GW'][current]['Current Gateway'].min(),current))

    currentval[current]['min'] = df_selection['PLC'][current]['Current PLC'].min()
    currentval[current]['max'] = df_selection['PLC'][current]['Current PLC'].max()
    currentval[current]['mean'] = df_selection['PLC'][current]['Current PLC'].mean()
    currentval[current]['meancleanOut'] = current - df_selection['PLC'][current]['Current PLC'].mean()
    currentval[current]['meancleanIn'] = current - df_selection['GW'][current]['Current Gateway'].mean()
    meancleanout.append(currentval[current]['meancleanOut'])
    meancleanin.append(currentval[current]['meancleanIn'])

plt.suptitle("Analog Gateway Performance: Current",fontsize=16, y=1)
plt.tight_layout()
plt.savefig('Analog_Performance_Test-Current.pdf',
            dpi=200)

# print(meancleanout)
meandeviationIn = statistics.mean(meancleanin)
meandeviationOut = statistics.mean(meancleanout)
print('Mean deviation: \n Input = %f mA\n Output = %f mA' %(meandeviationIn, meandeviationOut) )
print('Deviation Input \n Max = %f mA\n Min = %f mA' %(np.max(currentval['maxGW']), np.min(currentval['minGW'])))
print('Deviation Output \n Max = %f mA\n Min = %f mA'%(np.max(currentval['maxPLC']), np.min(currentval['minPLC'])))

VoltageValues=np.arange(0,11,1)
# print(VoltageValues)
df_selection = {}
for section in sections:
    df_selection[section] = {}
for i in VoltageValues:
    df_selection['PLC'][i] = voltageVO[voltageVO['Voltage Gateway']==i]
    df_selection['PLC'][i] =df_selection['PLC'][i][5:-5]
    df_selection['GW'][i] = voltageVI[voltageVI['Voltage PLC']==i]
    df_selection['GW'][i] =df_selection['GW'][i][5:-5]

num = 1
print(df_selection['PLC'][num])
print(df_selection['GW'][num])

plt.close()

plt.figure(figsize=(10, 10))
# plt.subplots_adjust(hspace=0.2, wspace=0.45)
plt.rcParams.update({'font.size': 15})
widths=0.8

voltageval={}
voltageval['minPLC'] = []
voltageval['maxPLC'] = []
voltageval['minGW'] = []
voltageval['maxGW'] = []
meancleanout=[]
meancleanin=[]

ub = 0.13
lb = 0.06
stepticks = 0.03

for n, voltage in enumerate(VoltageValues):
    
    ax = plt.subplot(3,4, n + 1)
   
    ax.boxplot([df_selection['PLC'][voltage]['Voltage PLC'].astype(float), 
                df_selection['GW'][voltage]['Voltage Gateway'].astype(float)],
                    labels=labels, widths=widths)
    ax.axhline(y=voltage, color='grey', linestyle='dashed')
    center = voltage
    ax.yaxis.set_ticks(np.arange(center-lb, center+ub, step=stepticks))

    voltageval[voltage] = {}
    voltageval['maxPLC'].append(calcDeviation(df_selection['PLC'][voltage]['Voltage PLC'].max(),voltage))
    voltageval['minPLC'].append(calcDeviation(df_selection['PLC'][voltage]['Voltage PLC'].min(),voltage))
    voltageval['maxGW'].append(calcDeviation(df_selection['GW'][voltage]['Voltage Gateway'].max(),voltage))
    voltageval['minGW'].append(calcDeviation(df_selection['GW'][voltage]['Voltage Gateway'].min(),voltage))

    voltageval[voltage]['meancleanOut'] = voltage - df_selection['PLC'][voltage]['Voltage PLC'].mean()
    voltageval[voltage]['meancleanIn'] = voltage - df_selection['GW'][voltage]['Voltage Gateway'].mean()
    meancleanout.append(voltageval[voltage]['meancleanOut'])
    meancleanin.append(voltageval[voltage]['meancleanIn'])
    

plt.suptitle("Analog Gateway Performance: Voltage", fontsize=16, y=1)
plt.tight_layout()
plt.savefig('Analog_Performance_Test-Voltage.pdf',
            dpi=200)

meandeviationIn = statistics.mean(meancleanin)
meandeviationOut = statistics.mean(meancleanout)
print('Mean deviation: \n Input = %f V\n Output = %f V' %(meandeviationIn, meandeviationOut) ) 
print('Deviation Input \n Max = %f V\n Min = %f V' %(np.max(voltageval['maxGW']), np.min(voltageval['minGW'])))
print('Deviation Output \n Max = %f V\n Min = %f V'%(np.max(voltageval['maxPLC']), np.min(voltageval['minPLC'])))