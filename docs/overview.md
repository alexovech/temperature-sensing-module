## Introduction

This project is a Monitoring system for a greenhouse. The Monitoring device contains temperature, light, humidity, and ultrasonic sensors; the analogue outputs are converted into digital measurements for the Arduino UNO R3 and then display the values on an OLED using Adafruit library.

The importance of this device is to ensure that plants have the best possible environment to grow. Any faults will be detected and quickly warn the user. An encryption was applied to the display for security. A checksum function was also added so that the total value of all values given from readings was checked to see if it matched the machine's calculations.

![](images/diagram.png)

---

## Roles of Sensors

- The Humidity sensor was to ensure the humidity was not high to damage the plants.
- The Light sensor was to ensure the light in the greenhouse was at acceptable levels.
- The Temperature sensor was to ensure the temperature was not high or too low so the plants can grow properly.
- The Motion sensor was for security so that anyone approaching the greenhouse at a certain proximity has an alarm sound off when they get too close.

---

## Temperature Sensor

The principle of working temperature sensor includes converting the measured temperature into a proportional voltage output of approximately 10 mV/°C. To better understand and validate this behaviour, a simplified model of the temperature sensor was created. However, due to modelling limitations in Multisim, the sensor behaviour could only be reproduced using operational amplifiers.

![](images/sensor.png)
