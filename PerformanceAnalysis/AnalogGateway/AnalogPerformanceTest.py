# -*- coding: utf-8 -*-
import os
import sys
from pathlib import Path
path_file = os.path.normpath(os.path.dirname(__file__) + os.sep + os.pardir)
#import fmu_handler
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
sleep_time = 200          # time in seconds for changing value

class analog_performance_test():

    def __init__(self):
        self.ads_instance = ads.ads()
        self.mqtt_instance = mqtt.mqtt()
        self.ads_instance.connect(ams_netID="X.XX.XXX.XXX.X.X", host="XXX.XXX.XXX.XXX")
        self.var_list = parsing_and_assignment.getRawADSVarListFromSymbols(self.ads_instance)
        self.mqtt_instance.connect()
        self.mqtt_instance.client.subscribe("#", qos=0)
        self.mqtt_instance.on_message = self.listen
        self.mqtt_instance.start_mqtt()
        # self.mode = 'voltage_input'
        self.mode = 'idle'
        self.file_exists = file_exists
        self.gwValue = None

    def listen(self, client=None, userdata=None, msg=None):
        try:
            with open(path, 'a', newline='') as f:
                # fieldnames = ['name', "value", 'timestamp']
                # writer = csv.DictWriter(f, fieldnames=fieldnames)
                # if not file_exists:
                #     writer.writeheader()  # file doesn't exist yet, write a header
                # msg.payload = ast.literal_eval(msg.payload.decode("utf-8"))
                # gw_temp={'name':'Temperature Gateway', 
                #          'value': msg.payload['Temperature Gateway'],
                #          'timestamp':str(datetime.datetime.now().isoformat())}
                # print(gw_temp)
                # writer.writerow(gw_temp)
                # plc_temp={'name':'Temperature PLC', 
                #           'value': float(self.ads_instance.read(var='GVL_default.el3202_ch1')/100),
                #           'timestamp':str(datetime.datetime.now().isoformat())}
                # print(plc_temp)
                # writer.writerow(plc_temp)
                if self.mode == "voltage_input" or self.mode == "voltage_output" or self.mode == "idle":
                    fieldnames = ['Voltage Gateway', "Voltage PLC", 'timestamp', 'test sequence']
                else:
                    fieldnames = ['Current Gateway', "Current PLC", 'timestamp', 'test sequence']

                writer = csv.DictWriter(f, fieldnames=fieldnames)
                if not self.file_exists:
                    writer.writeheader()  # file doesn't exist yet, write a header
                    print('wrote header')
                    self.file_exists = True
                msg.payload = ast.literal_eval(msg.payload.decode("utf-8"))

                plcValue = "idle"
                if self.mode == 'idle':
                    plcValue = float(self.ads_instance.read(var='MAIN.Vi_Kai'))
                elif self.mode == 'voltage_input':
                    plcValue = float(self.ads_instance.read(var='MAIN.Vo_read'))
                elif self.mode == 'voltage_output':
                    plcValue = float(self.ads_instance.read(var='MAIN.Vi_Kai'))            
                elif self.mode == 'current_input':
                    plcValue = float(self.ads_instance.read(var='MAIN.Io_read'))
                elif self.mode == 'current_output':
                    plcValue = float(self.ads_instance.read(var='MAIN.Ii_Kai'))
                

                if self.mode == "voltage_input" or self.mode == "idle":
                    row={
                        'Voltage Gateway': msg.payload['Analog_Input'], 
                        'Voltage PLC': plcValue,
                        'timestamp':str(datetime.datetime.now().isoformat()),
                        'test sequence': self.mode
                        }

                elif self.mode == "voltage_output":
                    row={
                        'Voltage Gateway': self.gwValue, 
                        'Voltage PLC': plcValue,
                        'timestamp':str(datetime.datetime.now().isoformat()),
                        'test sequence': self.mode
                        }

                elif self.mode == "current_input":
                    row={
                        'Current Gateway': msg.payload['Analog_Input'], 
                        'Current PLC': plcValue,
                        'timestamp':str(datetime.datetime.now().isoformat()),
                        'test sequence': self.mode
                        }

                elif self.mode == "current_output":
                    row={
                        'Current Gateway': self.gwValue, 
                        'Current PLC': plcValue,
                        'timestamp':str(datetime.datetime.now().isoformat()),
                        'test sequence': self.mode
                        }
                    
                else:
                    print('wrong mode selected')
                    row={
                        'Current Gateway': msg.payload['Analog_Input'], 
                        'Current PLC': plcValue,
                        'timestamp':str(datetime.datetime.now().isoformat()),
                        'test sequence': self.mode
                        }
                    

                print(row)
                writer.writerow(row)
        except: 
            pass
        # except KeyboardInterrupt():
        #     print('Abort')
        #     self.mqtt_instance.stop_mqtt()
        #     self.mqtt_instance.disconnect()
        #     # self.ads_instance.disconnect()

    def block_main_thread(self):
        while True:
            try:
                pass
            except KeyboardInterrupt():
                print('Abort')
                self.mqtt_instance.stop_mqtt()
                self.mqtt_instance.disconnect()
                # self.ads_instance.disconnect()
    
    # Perform Voltage input sequence
    def voltage_input(self):
        self.mode = 'voltage_input'
        v = 0
        while v <= 10:
            self.ads_instance.write(var='MAIN.Vo_Kai', val=v, typ=pyads.PLCTYPE_REAL)
            print('Set voltage on PLC to: %s V', v)
            v += 0.5
            time.sleep(sleep_time)

    def voltage_output(self):
        self.mode = 'voltage_output'
        v = 0
        TestVoltage = [1, 10]
        for v in TestVoltage:
        # while v <= 10:
            message = '{"Analog_Output":" %.2f"}' %v
            self.gwValue = v
            self.mqtt_instance.publish(message,'/cmd')
            print('send message: %s', message)
            v += 0.5
            time.sleep(sleep_time)

    def current_input(self):
        self.mode = 'current_input'
        i = 0
        while i <= 20:
            self.ads_instance.write(var='Main.Io_Kai', val=i, typ=pyads.PLCTYPE_REAL)
            print('Set Current on PLC to: %s mA', i)
            i += 1
            time.sleep(sleep_time)

    def current_output(self):
        self.mode = 'current_output'
        i = 0
        while i <= 20:
            message = '{"Analog_Output":" %.2f"}' %i
            self.gwValue = i
            self.mqtt_instance.publish(message,'/cmd')
            print('send message: %s', message)
            i += 1
            time.sleep(sleep_time)

#%%
if __name__ == '__main__':
    inst = analog_performance_test()

    # inst.voltage_input()
    inst.voltage_output()

# Change file name and run script again
    # inst.current_input()
    # inst.current_output()
    
    # inst.block_main_thread()

