# OSIGBApp
Open-Source Internet of Things (IoT) Gateways for Building Automation Applications

This repository will be populated in course of a journal publication (see publications section down below).

<!-- openocd -f board/esp32-wrover-kit-3.3v.cfg -->

## General

The IoT gateways are designed to operate as an interface for legacy components in building automation systems and IoT-based applications, e. g. connecting to cloud services via IoT protocols communication like MQTT. 

To this end, this repository contains the relevant [schematics and PCB design](https://github.com/RWTH-EBC/OSIGBApp/tree/main/Schematics) to obtain the interfacing hardware allowing to digitize and convert signals from sensors or to actuators. 
Here, the following signals are supported:
- Analog communication (0-10 V, 0-20 mA)
- Resistive measurements of passive RTD-probes (2,3,4-wire)
- Digital communication via RS-485/EIA-485 (for instance used in serial Modbus RTU or BACnet MS/TP communication)
Core component of the gateways is the ESP32 microcontroller.
Hence, the firmware was developed using the Espressif IoT Development Framework [ESP-IDF](https://github.com/espressif/esp-idf), which is the official development framework provided by the manufacturer for the [ESP32](https://espressif.com/en/products/hardware/esp32/overview) chip.
Accordingly, the developed [firmware](https://github.com/RWTH-EBC/OSIGBApp/tree/main/Firmware) is provided as well in this repository. 
In order to ease use and operation of the gateways, we further provide a [documentation](https://github.com/RWTH-EBC/OSIGBApp/tree/main/Documentation). 
Finally, a [performance analysis](https://github.com/RWTH-EBC/OSIGBApp/tree/main/PerformanceAnalysis) has been conducted comparing the measurement performance with a conversion via a PLC.

## Documentation:
- The user Manual "how to operate the gateways" can be found [here](Documentation/manual.pdf). 
- For using the software, [ESP-IDF](https://github.com/espressif/esp-idf) is needed. We also recommend to use [VScode](https://code.visualstudio.com/) with the ESP-IDF extension for programming.
- Furthermore, tips for using the Menuconfig with this software can be found in the Wiki of the repository.
- All functions in the code are commented and a documentation is generated with [Doxygen](https://github.com/doxygen/doxygen). The result can be found in the [DOC folder](https://github.com/RWTH-EBC/OSIGBApp/tree/main/Firmware/doc). You have to clone the repository and open doc/doxygen/html/index.html to view the complete documentation.

- Documentation on how to build the Aio Configurator webpage can be found here: [AIO Configurator](Firmware/doc/aio_configurator.md)
  - The IoT-AIO Configurator is a web page that uses HTML forms to configure the IoT Wireless Adapter.
    The source code and documentation can be obtained here:
      - [Configurator Request](Firmware/doc/markdown/aio_configurator_request.html)
      - [Configurator Response](Firmware/doc/markdown/aio_configurator_response.html)
     
- Further developement Ideas, by Carlo Guarnieri can be found here: [Developments](Firmware/doc/developments.md)

## Terms of use
All content provided in this repository is released under the [MIT License](https://github.com/RWTH-EBC/OSIGBApp/blob/main/LICENSE). 
If any of the content provided is used, please give credit by referencing the related publication, as decribed in the subsequent section.

## Publications and referencing
<!--The IoT-Gateways have been developed in several research projects between 2018 and 2021. A core development for the current state has been conducted during the Master Thesis of Kai Droste in 2020. -->
We kindly ask all academic publications employing components of the IoT-Gateways to preferrably cite the following paper as soon as it is available:

<!--  - Kai Droste "[Development of a cloud controlled building automation and comparison with the current state of the art](https://git-ce.rwth-aachen.de/ebc/projects/ebc0449_bmwi_nextgenbat_ga/iot-development/iot-development.dissemination/theses/development-of-a-cloud-controlled-building-automation-and-comparison-with-the-current-state-of-the-art)", 2020 Masterthesis   -->
- **M. H. Schraven, K. Droste, C. Guarnieri Caló Carducci, D. Müller, A. Monti, "Open-Source Internet of Things Gateways for Building Automation Applications", Unpublished research**
Or refer to the previous development stages as follows:
- M.H. Schraven, C. Guarnieri Calò Carducci, M.A. Baranski, D. Mueller, A. Monti, “Designing a Development Board for Research on IoT Appli-cations in Building Automation Systems,” 36th International Symposiumon  Automation  and  Robotics  in  Construction  (ISARC  2019),  Banff,Canada, 2019, pp. 82-90, [DOI: 10.22260/ISARC2019/0012](https://doi.org/10.22260/ISARC2019/0012)
- C. Guarnieri Calò Carducci, A. Monti, M. H. Schraven, M. Schumacher and D. Mueller, “Enabling ESP32-based IoT Applications in Building Automation Systems,” 2019 II Workshop on Metrology for Industry 4.0 and IoT (MetroInd4.0&IoT), Naples, Italy, 2019, pp. 306-311, [DOI: 10.1109/METROI4.2019.8792852](https://doi.org/10.1109/METROI4.2019.8792852)

# Further software related to IoT-AIO

## IoT-AIO Configurator

# Contact
[![EONERC EBC Logo](Firmware/doc/eonerc_logo.png)](http://www.ebc.eonerc.rwth-aachen.de)

- [Kai Droste](mailto:kai.droste@eonerc.rwth-aachen.de)
- [Markus Schraven](mailto:mschrave@eonerc.rwth-aachen.de)
- [Carlo Guarnieri](mailto:cguarnieri@eonerc.rwth-aachen.de)

[EON Energy Research Center (EONERC)](http://www.eonerc.rwth-aachen.de)  
[Institute for Automation of Complex Power Systems (ACS)](http://www.acs.eonerc.rwth-aachen.de)  
[RWTH University Aachen, Germany](http://www.rwth-aachen.de)  
