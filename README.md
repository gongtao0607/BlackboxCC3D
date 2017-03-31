# BlackboxCC3D

The BlackboxCC3D is a firmware running on CC3D/ATOM platfrom to serve as a flight data recorder (Black box). The firmware implements the (digital) signal pass-through between "Output ports" and "Receiver port" (the original naming from CC3D), so that no Y-cable is needed to monitor the signal. The BlackboxCC3D can be extremely helpful to tune the parameters on 3 axis gyros

The BlackboxCC3D currently records following signals:
* 5 PWM signals + 1 RPM signal:
The 6 RX-Ports/Output-ports can be used to record servo PWM signal (duty width), or RPM signal (cycle width)
* Orientation:
The Quaternion computed by MPU6050 DMP module is recorded. The quaternion can be later decoded to orientation (Yaw/Pitch/Roll), or 3 axis integral (the sensor reading of a FBL gyro)
* DSM2 satellite signal:
Can be used to record transmitter signals.

The BlackboxCC3D send sampled data to Main Port (USART1). By connecting to an OpenLog module, all the data can be stored to an Micro-SD card. The data can be decoded and analyzed/visualized by a python script.

# Connection
## PWM/RPM signals:
```
(FBL GYRO)  ((RX Port)  (Output Port))  (Servo/ESC/Sensor)
    CH1   ->   S1 IN -->>   S1 OUT    ->    Servo 1
    CH2   ->   S2 IN -->>   S2 OUT    ->    Servo 2
    CH3   ->   S3 IN -->>   S3 OUT    ->    Servo 3
    CH4   ->   S4 IN -->>   S4 OUT    ->    Servo 4
    CH5   ->   S5 IN -->>   S5 OUT    ->    ESC
    RPM   <-   S6 IN <<--   S6 OUT    <-    RPM Sensor
```

*Note that the pass-through direction for S6 is reversed, since the RPM signal is usually send from a male type connector (RPM sensor), to a female type connector (gyro RPM sensor port). Also remember only S1 IN and S1-6 OUTs have power connectors.*

## DSM2 satellite:
The BlackboxCC3D can either share the satellite with FBL gyro, or use a dedicated one.
* Shared:
Connect only the RX wire of Main/Flexi Port to a satellite signal. (Make a joint cable). No other wires are required if the BlackboxCC3D is already connect to the FBL gyro. (If not, connect the ground wire)
```
Satellite ------- (-) FBL Gyro
          ------- (+)
          ---|--- (S)
             |
        BlackboxCC3D
```
* Dedicated:
You need to make a voltage regulator cable since the satellite runs at 3.3V

The satellite signal can either run to USART1 (Main Port), which will be shared by the OpenLog (since we only TX to OpenLog), or run to USART3 (Flexi Port). Running to Main Port is easier for a shared connection wiring, while the the Flexi Port is easier for a dedicated connection wiring.

## OpenLog:
The OpenLog module need to connect to the Main Port for data storage. 3 wires (GND, VCC, TX) should connect to the OpenLog (GND, VCC, *RX*). If Main Port is shared by OpenLog and DSM2 satellite, only one baud rate can run (115200, since the satellite runs on that)
