# -*- coding: utf-8 -*-
import ads
import ast
import csv
import datetime
import mqtt
import os
import parsing_and_assignment
import time

path = os.path.join(os.getcwd(), 'data_files', 'TemperaturePerformanceTest.csv')
file_exists = os.path.isfile(path)

class temp_performance_test():

    def __init__(self):
        self.ads_instance = ads.ads()
        self.mqtt_instance = mqtt.mqtt()
        self.ads_instance.connect(ams_netID="X.XX.XXX.XXX.X.X", host="XXX.XXX.XXX.XXX")
        self.var_list = parsing_and_assignment.getRawADSVarListFromSymbols(self.ads_instance)
        self.mqtt_instance.connect()
        self.mqtt_instance.client.subscribe("#", qos=0)
        self.mqtt_instance.on_message = self.listen
        self.mqtt_instance.start_mqtt()

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
                fieldnames = ['Temperature Gateway', "Temperature PLC", 'timestamp']
                writer = csv.DictWriter(f, fieldnames=fieldnames)
                if not file_exists:
                    writer.writeheader()  # file doesn't exist yet, write a header
                msg.payload = ast.literal_eval(msg.payload.decode("utf-8"))
                row={'Temperature Gateway': msg.payload['Temperature Gateway'], 
                     'Temperature PLC': float(self.ads_instance.read(var='GVL_default.el3202_ch1')/100),
                     'timestamp':str(datetime.datetime.now().isoformat())}
                print(row)
                writer.writerow(row)
        except KeyboardInterrupt():
            print('Abort')
            self.mqtt_instance.stop_mqtt()
            self.mqtt_instance.disconnect()
            self.ads_instance.disconnect()

    def block_main_thread(self):
        while True:
            try:
                pass
            except KeyboardInterrupt():
                print('Abort')
                self.mqtt_instance.stop_mqtt()
                self.mqtt_instance.disconnect()
                self.ads_instance.disconnect()

#%%
if __name__ == '__main__':
    inst = temp_performance_test()