# MSP432-Digital-Notch-Filter

This program removes unwanted noise (60 Hz) using a digital notch filter. A sine wave enters the MSP432's precision ADC module and exits through an external DAC after being digitally filtered. Matlab is used to calculate the 60Hz notch filter transfer function which is then implemented in C.

https://github.com/Ltran0325/MSP432-Digital-Notch-Filter/blob/main/main.c

**Demo:**

https://www.youtube.com/watch?v=BwOXYYQE5To

**Notch Filter Transfer Function:**

<a href="https://www.codecogs.com/eqnedit.php?latex=H(s)&space;=&space;\frac{s^w&space;&plus;&space;w_n^2}{s^2&space;&plus;&space;(w_n/Q)s&space;&plus;w_n^2}" target="_blank"><img src="https://latex.codecogs.com/svg.latex?H(s)&space;=&space;\frac{s^w&space;&plus;&space;w_n^2}{s^2&space;&plus;&space;(w_n/Q)s&space;&plus;w_n^2}" title="H(s) = \frac{s^w + w_n^2}{s^2 + (w_n/Q)s +w_n^2}" /></a>

<a href="https://www.codecogs.com/eqnedit.php?latex=H(z)&space;=&space;\frac{b_2z^2&space;&plus;b_1z^2&space;&plus;b_0}{z^2&space;&plus;a_1z&space;&plus;&space;a_0}" target="_blank"><img src="https://latex.codecogs.com/svg.latex?H(z)&space;=&space;\frac{b_2z^2&space;&plus;b_1z^2&space;&plus;b_0}{z^2&space;&plus;a_1z&space;&plus;&space;a_0}" title="H(z) = \frac{b_2z^2 +b_1z^2 +b_0}{z^2 +a_1z + a_0}" /></a>


**MATLAB Code:**
```MATLAB

% H(s) = (s^2 + w^2)/(s^2 + (w/Q)s + w^2)

% w = 60Hz = center frequency to reject

% Q = 3 = quality factor (increase to narrow rejection)

% w/Q = bandwidth of the rejection band

Hc = tf([1,  0, (2 * pi * 60)^2], [1, 20, (2 * pi * 60)^2]);

bode(Hc);

Hd = c2d(Hc, 1/2000)
```
**MATLAB Bode Plot:**

60Hz notch filter.

![image](https://user-images.githubusercontent.com/62213019/112414144-ccb56380-8cde-11eb-8755-160d54b66b95.png)

**Oscilloscope (Scopy):**

60 Hz is filtered.

![image](https://user-images.githubusercontent.com/62213019/112414497-61b85c80-8cdf-11eb-8b1b-fc9883f2d173.png)
