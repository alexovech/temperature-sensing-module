## Problem 1 — Operational Amplifier Topology Limitation

Due to component availability constraints, only LM741 operational amplifiers were provided for the analog front-end design.  
With this amplifier, voltage summation and offset generation can only be achieved using an inverting configuration.  
As a result, the conditioned signal polarity was inverted after the first amplification stage.  

**Solution:**  
A second operational amplifier stage was introduced to restore the original signal polarity, resulting in a two-stage amplifier architecture.

---

## Problem 2 — Operational Amplifier Power Supply Requirements

Initially, the LM741 operational amplifiers were powered from the same 5 V supply as the Arduino system.  
Under this configuration, the amplifier output exhibited incorrect gain behaviour and saturation effects.  

**Solution:**  
It was identified that the LM741 requires a higher supply voltage. Two 9 V batteries were introduced to provide a ±9 V dual supply configuration, enabling correct amplifier operation.

---

## Problem 3 — Mixed Power Supply Grounding Issue

After introducing the ±9 V supply for the operational amplifiers, the system still produced unstable and incorrect readings.  
This issue was traced to the presence of multiple power supplies (±9 V for the amplifiers and 5 V for the Arduino and sensors) without a shared ground reference.  

**Solution:**  
The ground references of the ±9 V amplifier supply and the 5 V Arduino supply were connected to establish a common ground.  
This resolved the instability and allowed consistent voltage measurements across the system.

---

## Problem 4 — ADC Conversion Scaling Error

Initial ADC-to-voltage conversion calculations assumed an ideal 5.0 V reference voltage for the Arduino ADC.   
However, measurements using a multimeter showed that the actual supply voltage was approximately 5.07 V.  
This discrepancy introduced a systematic scaling error in the computed sensor voltage and temperature values.  

**Solution:**  
The measured reference voltage value was used in the ADC conversion formula within the Arduino code, improving the accuracy of the reconstructed sensor voltage.
