Incubator:
Configuration/Config Module:
D6 GPIO12        -> AM2301
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



Garage:
Configuration/Config Module:
D1 GPIO5         -> Relay2i (30)
D5 GPIO14 Sensor -> DS18x20 (4)
D0 GPIO16        -> Relay1i (29)

Configuration/Configure MQTT
Host              -> piwnica
Port              -> 1883
Topic             -> garage
Full Topic        -> %prefix%/%topic%/



Boiler:

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


Configuration/Configure MQTT
Host              -> piwnica
Port              -> 1883
Topic             -> kurnik
Full Topic        -> %prefix%/%topic%/

D3 GPIO0  -> Switch2n (83) <-- Close Window
D4 GPIO2  -> Relay2i  (30)
D2 GPIO4  -> Relay5i  (33) ==> Lamp
D1 GPIO5  -> Switch1n (81) <-- Open Window
D6 GPIO12 -> Relay3i  (31) ==> Open Window
D7 GPIO13 -> Relay4i  (32) ==> Close Window
D5 GPIO14 -> DS18x20   (4) <-- Temp. sensors
D0 GPIO16 -> Relay1i  (29)

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

