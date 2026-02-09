## Purpose of calibration

The objective was to verify and confirm the accuracy of the output values, especially under non-standard conditions where the sensor may receive a greater or lesser load than expected. 
If the values do not match the simulation, a new linear relationship will be created, which will be required for the implementation to the Software module.

## Extreme Condition Testing

Initially, the sensor was placed in a low-temperature environment, which was measured using a non-contact infrared thermometer and was approximately 0 °C. 
In this experiment, any contact with the sensor that could emit additional heat was minimized, so the measurements were as accurate as possible. 
The result from the LM35 was 0.05 V, which was comparable to 5 °C.

![]()

In the next procedure, the tests were conducted at temperatures closer to the maximum values. 
In this case, a specific heater was used to accurately produce energy comparable to 100 °C. 
Under these results, the sensor readings showed 0.85 V, i.e. 85 °C.

![]()

Based on these results, a new linear relationship should be created, which will be used in the future to calibrate the sensor value to its ideal parameters. Excel was used to create the linear graph and obtain the linear equation. The accuracy of the temperature measurement module can be improved by making use of additional calibration points, a better operational amplifier, and a voltage reference.

![]() 
