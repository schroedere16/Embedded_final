# Final Project: Ultrasonic Parking Sensor
The overall operation of this design revolves around detecting distance using an ultrasonic sensor. The main components responsible for the device's operation are the MSP4305529 microcontroller and the HC-SR04 Ultrasonic sensor. The HC-SR04 uses ultrasonic waves to detect distance by transmitting a sound wave from one terminal and receiving the reflected wave back in the other. This allows for a fairly accurate distance reading for objects located in front of the sensor for up to 2 meters. Using the output of this device, the ADC on the microcontroller is able to convert the reading to a distance measurement (cm). Based on the distance detected, the five indicator LEDs illuminate accordingly so that the user can gauge the distance.

There are five indicator LEDs that are used to indicate how close an object is with relation to the sensor. Each LED is assigned a distance value. Once the object in front of the sensor is equal to or less than the value, the respective LED lights up. UART was also utilized for this project. The UART transmit buffer displayed the distance readings in real time, which adjusted based on the distance detected. The receive buffer was also utilized in setting new distance values. For example, if the user wanted to change the distance thresholds for each LED, this could be done by sending a package of bytes through UART.

# Authors
Nick Scamardi, Eric Schroeder & Nick Setaro are the three contributors to this Project.

# Dependencies
The library dependencies for the code behind the sensor include the standard `<msp430.h>` library, `<stdint.h>` and `<intrinsics.h>`.

# Components
The main components used to build this milestone include:
* TI MSP430F5529 Microcontroller
* HC-SR04 Ultrasonic Sensor
* 5 LEDs (White, Blue, Green, Yellow, Red)
* Code Composer Studio 8.1.0


# Functionality of main.c
First, all of the variables used were initialized, as well as the pins on the MSP430F5529. The inital distance values were configured and are displayed below:

```c
    uint32_t D_1 = 10;
    uint32_t D_2 = 20;
    uint32_t D_3 = 30;
    uint32_t D_4 = 40;
    uint32_t D_5 = 50;
    int byte = 0;
```

Once all of the values were set, as well as the pins, the timers and UART were both configured. The BAUD rate used for this project was 9600. An infinite for loop was utilized for converting the output of the sensor to a distance reading in centimeters. This loop is shown below:

```c
    for(;;)
    {
         triggerMeasurement();

        // Wait for echo start
        __low_power_mode_3();

         lastCount = TA0CCR2;

        // Wait for echo end
        __low_power_mode_3();

        distance = TA0CCR2 - lastCount;
        distance *= 34000;
        distance >>= 14;  // division by 16384 (2 ^ 14)
```

This code is responsible for providing accurate distance calculations in centimeters based on the analog input. The rest of the code consists of using if statements to illuminate the correct light based on the distance, as well as interrupt service routines that set the transmit and receive buffers to send and receive over UART.
