# -*- coding: utf-8 -*-
import os
import sys
from pathlib import Path
from tkinter.tix import REAL
path_file = os.path.normpath(os.path.dirname(__file__) + os.sep + os.pardir)
sys.path.insert(1,path_file)

import ads
import pyads
import ast
import csv
import datetime
import mqtt
import parsing_and_assignment
import time


path = os.path.join(os.path.dirname(__file__), 'data_files', 'AnalogPerformanceTest.csv')
file_exists = os.path.isfile(path)
sleep_time = 3          # time in seconds for changing value

class analog_performance_test():

    def __init__(self):
        self.ads_instance = ads.ads()
        # self.mqtt_instance = mqtt.mqtt()
        self.ads_instance.connect(ams_netID="X.XX.XXX.XXX.X.X", host="XXX.XXX.XXX.XXX")
        self.var_list = parsing_and_assignment.getRawADSVarListFromSymbols(self.ads_instance)
        # self.mqtt_instance.connect()
        # self.mqtt_instance.client.subscribe("#", qos=0)
        # self.mqtt_instance.on_message = self.listen
        # self.mqtt_instance.start_mqtt()
        # self.mode = 'voltage_input'
        self.mode = 'idle'
        self.file_exists = file_exists
        self.ads_instance.write(var='MAIN.Vo_Kai', val=7.532, typ=pyads.PLCTYPE_REAL)
        # pub,sub = parsing_and_assignment.getADSvarsFromSymbols(self.ads_instance)
        # print(pub)
        # print(sub)
#%%
if __name__ == '__main__':
    inst = analog_performance_test()

    # inst.voltage_input()
    inst.voltage_output()

# Change file name and run script again
    # inst.current_input()
    # inst.current_output()
    
    # inst.block_main_thread()

