Incubator:
Configuration/Config Module:
D6 GPIO12        -> SI7021
D7 GPIO13        -> Relay_i [1]
D5 GPIO14        -> DS18x20

Configuration/Configure MQTT:
Host              -> piwnica
Port              -> 1883
Topic             -> incubator
Full Topic        -> %prefix%/%topic%/

Configure Logging:
Telemetry period  -> 10

Console:
SetOption36 0
# Based on: https://tasmota.github.io/docs/Commands/#setoption65
SetOption65 1

# Time settings
        H W M D h T
TimeDST 0,0,3,7,2,120
TimeSTD 0,0,10,7,3,60
Timezone 99

Reset 99

WebButton1 "Heater stop"

Rule1 1
Rule1 on tele-DS18B20#Temperature<37.8 do POWER1 OFF endon on tele-DS18B20#Temperature>37.7 do POWER1 ON endon



Garage: ESP32-DevKit v1, branch: dw-14.2.0.3-development-ds18x20-ext-20240901
Configuration/Config Module:                                Garage cable                      Basement cable
TX GPIO1         -> ModBr Tx
RX GPIO3         -> Modbr Rx
IO GPIO25        -> DS18x20   [1]
IO GPIO14        -> DS18x20   [2]
 [0] 28FFBA13761801F0 - freezer outside
 [1] 28FFA7156C1803F5 - freezer inside
IO GPIO14        -> DS18x20   [2]
 [2] 28FFC252761801CC - hot water
 [3] 28FF810134180145 - cold water
IO GPIO26        -> Relay_i   [3] - Control garage gate     White-Green
IO GPIO27        -> Relay_i   [4] - Main circulating pump
IO GPIO32        -> Relay_i   [1] - Opening main gate       Orange                            White-Green
AO GPIO33        -> Relay_i   [2] - Closing main gate       Green
IO GPIO12        -> Relay     [5] - DS18B20 Vdd
                                                                                              +3V3 White-Green
                                                                                              GND  Green
                                                            White-Orange: Relays' common
                                                            White-Brown & Brown: +12V
                                                            White-Blue & Blue: GND

Configuration/Configure MQTT
Host              -> piwnica
Port              -> 1883
Topic             -> garage
Full Topic        -> %prefix%/%topic%/

Console:
# Temperature sensor resolution
TempRes 1

# Enable Kalman filter mean over teleperiod for JSON temperature for DS18x20 sensors
SetOption126 1

# Reset counters at TelePeriod time
SetOption79 1

WebButton1 Open main gate
WebButton2 Close main gate
WebButton3 Control garage gate
WebButton4 Main circulating pump
WebButton5 DS18B20 Vdd

TelePeriod 10

# Boot loop defaults restoration control.
SetOption36 0
# Based on: https://tasmota.github.io/docs/Commands/#setoption65
SetOption65 1

# Time settings
        H W M D h T
TimeDST 0,0,3,7,2,120
TimeSTD 0,0,10,7,3,60
Timezone 99

Reset 99


Boiler: esp8266-1 8.3.1.6

Configuration/Config Module:
D4 GPIO2         -> Relay2i (30)
D5 GPIO14 Sensor -> DS18x20 (4)
D0 GPIO16        -> Relay1i (29)

Console:
Var1 = Boiler
Var2 = Outflow
Rule1 on DS18B20-1#Temperature do Var1 %value% endon on DS18B20-2#Temperature do Var2 %value% endon on Time#Minute do if (((Var2 < 30) AND (Var1 < 30) AND (Mem1 >= 0)) OR (Mem1 > 0)) POWER1 ON elseif ((Var2 > 35) OR (Var1 > 35) OR (Mem1 < 0)) POWER1 OFF endif endon
Rule1 1
Rule2 on tele-DS18B20-3#Temperature<5 do POWER2 ON endon on tele-DS18B20-3#Temperature>10 do POWER2 OFF endon
Rule2 1
Rule3 on mqtt#connected do Subscribe BoilerHeaterEvent, evnt/sonoff-54/BoilerHeaterMode, Mem1 endon on Event#BoilerHeaterEvent do Mem1 = %value% endon
Rule3 1
SetOption36 0
SetOption65 1
WebButton1 Heater
WebButton2 Anti freezing

Rule3 based on: https://github.com/arendst/Tasmota/wiki/Subscribe-&-Unsubscribe
mosquitto_pub -h piwnica -t evnt/sonoff-54/BoilerHeaterMode -m {"Mem1":"1"}
stat/sonoff-54/RESULT = {"Mem1":"1.000"}


Kurnik: 12.2.0.2
Configuration/Configure MQTT
Host              -> piwnica
Port              -> 1883
Topic             -> kurnik
Full Topic        -> %prefix%/%topic%/

D3 GPIO0  -> Switch_n [2] <-- Close Window
D4 GPIO2  -> DS18x20  [1] <-- Temp. sensors
D2 GPIO4  -> Relay_i  [5] ==> Lamp
D1 GPIO5  -> Switch_n [1] <-- Open Window
D6 GPIO12 -> Relay_i  [3] ==> Open Window
D7 GPIO13 -> Relay_i  [4] ==> Close Window
D0 GPIO16 -> Relay_i  [1]

Latitude 50.188476
Longitude 19.1143973

Configure Timers -> Enable Timers
Timer1 {"Arm":1,"Mode":1,"Time":"00:00","Window":0,"Days":"1111111","Repeat":1,"Output":1,"Action":3}
Timer2 {"Arm":1,"Mode":2,"Time":"01:00","Window":0,"Days":"1111111","Repeat":1,"Output":2,"Action":3}

# Timers: https://tasmota.github.io/docs/#/Timers
# 
# Timer 1 open
# Timer 2 close
# Relay 1  openning (POWER1) =1 pressed
# Relay 2 clossing   (POWER2) =1 pressed
# Relay 3 OpenWindow (POWER3)
# Relay 4 CloseWindow (POWER4)
# on

mem5 1

Rule1
on Power1#state do var1 %value% endon
on Power2#state do var2 %value% endon
on Power3#state do var3 %value% endon
on Power4#state do var4 %value% endon

rule1 1


Rule2
on Power1#state do
 if (Var1==1 AND Var2==1)
  var9=1;
  Power3 0;
  Power4 0;
 elseif (Var1==1 AND Var2==0)
  var9=2;
  Power3 1;
  Power4 0;
  Power5 0;
 else
  var9=3
 endif
endon

on Power2#state do
  if (Var1==1 AND Var2==1)
    var9=4;
    Power3 0;
    Power4 0
  elseif (Var1==0 AND Var2==1)
    var9=5;
    Power3 0;
    Power4 1;
    Power5 %mem5%;
  else
    var9=6
  endif
endon

Rule2 1


Rule3
on Clock#Timer=1 do
 if ((Var2==0) AND (Var3==0))
  Power3 1;
  Power4 0;
  Power5 0;
 endif
endon

on Clock#Timer=2 do
 if ((Var1==0) AND (Var4==0))
  Power3 0;
  Power4 1;
  Power5 %mem5%;
 endif
endon

on Time#Minute do
 Publish stat/%topic%/SUN {"SunRise":"%sunrise%","SunSet":"%sunset%"}
endon

Rule3 1


# Button's labels
WebButton1 Open
WebButton2 Close
WebButton3 Opening
WebButton4 Closing
WebButton5 Lamp

SetOption36 0
# Based on: https://tasmota.github.io/docs/Commands/#setoption65
SetOption65 1

# Time settings
        H W M D h T
TimeDST 0,0,3,7,2,120
TimeSTD 0,0,10,7,3,60
Timezone 99

Reset 99

heat-exchanger: dw-14.2.0.3-development-ds18x20-ext-20240901
Address 1: Heat Pump
Address 2: L3F1946-P (DTS-1496-4P)

Configuration/Config Module:
TX GPIO1         -> ModBrTx
RX GPIO3         -> ModBr Rx
D1 GPIO5         -> Flowrate [1]
D5 GPIO14        -> DS18x20  [1]

Configuration/Configure MQTT:
Host              -> piwnica
Port              -> 1883
Topic             -> heat_exchanger
Full Topic        -> %prefix%/%topic%/

Console:

Rule1 1
Rule1 on System#Boot do
  ModbusTCPStart 502;
  ModbusBaudrate 9600;
  ModbusSerialConfig 8N1;
endon

TelePeriod 5

SetOption36 0
# Based on: https://tasmota.github.io/docs/Commands/#setoption65
SetOption65 1

# Temperature sensor resolution
TempRes 1

# Enable Kalman filter mean over teleperiod for JSON temperature for DS18x20 sensors
SetOption126 1

# Time settings
        H W M D h T
TimeDST 0,0,3,7,2,120
TimeSTD 0,0,10,7,3,60
Timezone 99

Reset 99

# Console Test: ModbusSend {"deviceAddress":1, "functionCode":3, "startAddress":0, "type":"raw","count":2}
# Set R01 Temp: ModbusSend {"deviceAddress":1, "functionCode":16, "startAddress":1158, "type":"int16","count":1, "Values":[422]}
# Debug: SSerialSend5 01 03 00 00 00 06 c5 c8
# Debug: socat -u -x /dev/ttyUSB1,raw,b9600,cs8,ospeed=b9600,ispeed=b9600 -

Air-X:

To disable deepsleep over MQTT use mosquitto_pub with: -t "cmnd/air-x/DeepsleepTime" -r -m "0"
    To remove retained message use mosquitto_pub with: -t "cmnd/air-x/DeepsleepTime" -r -n"

ESP32-D0WD
Host              -> piwnica
Port              -> 1883
Topic             -> air-x
Full Topic        -> %prefix%/%topic%/

Configuration/Config Module:
PIC16F                       Pin
ESP32-DevKit(1)           Pin
IO GPIO13 -> Relay     [5] 13 12
IO GPIO14 -> Relay     [4] 14 11
IO GPIO16 -> DeepSleep
IO GPIO25 -> Relay     [1] 25 13
IO GPIO26 -> Relay     [2] 26  9
IO GPIO27 -> Relay     [3] 27 10
IO GPIO12 -> Counter   [1] 12  6
IA GPIO36 -> ADC Range [1] VP 17

Configuration/Configure Other:
Device Name: Wind Turbine Air-X RevE4.3

ADC:
10500 @11.01V
12600 @13.01V
12000 @12.50V
13088 @13.29V
15380 @14.36V

Console:
# Based on: https://tasmota.github.io/docs/ADC/
adcparam1 6, 0, 4096, 0, 16500

Rule1 1
Rule1
  on analog#range1>14700 do backlog power1 1; power2 1; power3 1; power4 1; power5 1; endon
  on analog#range1<14000 do backlog power1 0; power2 0; power3 0; power4 0; power5 0; endon
  on analog#range1<13000 do backlog DeepsleepTime 3600; endon
  on System#Boot do backlog DeepsleepTime 0; endon

# Button's labels
WebButton1 LED(13)
WebButton2 EN0(9)
WebButton3 EN1(10)
WebButton4 EN2(11)
WebButton5 EN3(12)

SetOption36 0
# Based on: https://tasmota.github.io/docs/Commands/#setoption65
SetOption65 1

# Time settings
        H W M D h T
TimeDST 0,0,3,7,2,120
TimeSTD 0,0,10,7,3,60
Timezone 99

Reset 99



https://docs.espressif.com/projects/esp-idf/en/stable/esp32/hw-reference/esp32/get-started-devkitc.html
http://www.lcdwiki.com/res/MSP1443/1.44inch_SPI_Module_MSP1443_User_Manual_EN.pdf
https://tasmota.github.io/docs/Displays/#universal-display-driver
https://github.com/arendst/Tasmota/discussions/17605

GPIO5  SPI CS   [1]   SS
GPIO13 SPI DC   [1]   A0
GPIO18 SPI CLK  [1]   SCK
GPIO23 SPI MOSI [1]   SDA
GPIO25 DS18x20  [1]
GPIO32 Output Hi      RESET
GPIO33 Output Hi      LED
GPIO39 Option A [3]
