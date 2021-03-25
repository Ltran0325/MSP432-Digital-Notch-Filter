# MSP432-Digital-Notch-Filter

This program removes unwanted noise (60 Hz) using a digital notch filter. A sine wave enters the MSP432's precision ADC module and exits through an external DAC after being digitally filtered. Matlab is used to calculate the 60Hz notch filter transfer function which is then implemented in C.

**Demo:**

https://www.youtube.com/watch?v=BwOXYYQE5To

**Matlab Code:**

Hc = tf([1, 0, (2*pi*60)^2], [1,20, (2*pi*60)^2]);

bode(Hc);

Hd = c2d(Hc, 1/2000)

**Matlab Bode Plot:**

![image](https://user-images.githubusercontent.com/62213019/112414144-ccb56380-8cde-11eb-8755-160d54b66b95.png)

**Oscilloscope (Scopy):**

![image](https://user-images.githubusercontent.com/62213019/112414497-61b85c80-8cdf-11eb-8b1b-fc9883f2d173.png)
