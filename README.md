# Greenhouse Monitoring System – Temperature Sensing Module

Team of 4 students.  
My role: temperature sensing module designer.  
Designed and implemented an LM35-based temperature sensing module with analog signal conditioning, calibration, and hardware-level error detection.  
The project consists of two stages: a standalone temperature sensing module and system integration with enclosure.

---

## Overview

- **Full project description:** [Overview](docs/overview.md)
- **Skills Developed:** Designing operational amplifier circuit, sensor calibration and parameter identification, embedded system implementation in Arduino, 3D CAD modelling.
- **Features:** OpAmp-based gain and offset correction, Arduino ADC inverse mapping, sensor error detection, custom 3D-printed enclosure.
- **Technology:** Arduino, LM35 temperature sensor, LM741 operational amplifier, Resistors, OLED display, LEDs.
- **Simulation:** Analog part designed in [Multisim](); Enclosure model designed in [Autodesk Fusion 360]().
- **Language:** C/C++ (Arduino).

---

# Stage 1 - Temperature Sensor

## Problem Statement

The LM35 temperature sensor provides a low-level output (~10 mV/°C).  
The Arduino ADC operates over a much higher voltage range:from 3 V to 5 V.  
Such a small sensor voltage results in poor ADC resolution and reduced measurement accuracy.  
To obtain valid and reliable temperature readings, the signal must be shifted and amplified into a suitable voltage range.  
This is achieved using LM741 operational amplifiers and resistive networks.
