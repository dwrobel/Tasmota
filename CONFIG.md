Configuration/Configure MQTT
Host              -> 192.168.160.5
Port              -> 1885
Topic             -> sonoff-54
Full Topic        -> %prefix%/%topic%/

CConfiguration/Config Module:
D4 GPIO2         -> Relay2i (30
D5 GPIO14 Sensor -> DS18x20 (4)
D0 GPIO16        -> Relay1i (29)

Console:
Var1 = Outflow
Var2 = Boiler
Rule1 on DS18B20-1#Temperature do Var1 %value% endon on DS18B20-2#Temperature do Var2 %value% endon on Time#Minute do if (((Var2 < 30) AND (Var1 < 30) AND (Mem1 >= 0)) OR (Mem1 > 0)) POWER1 ON elseif ((Var2 > 35) OR (Var1 > 35) OR (Mem1 < 0)) POWER1 OFF endif endon
Rule1 1
Rule2 on tele-DS18B20-3#Temperature<5 do POWER2 ON endon on tele-DS18B20-3#Temperature>10 do POWER2 OFF endon
Rule2 1
Rule3 on System#Boot do Subscribe BoilerHeaterEvent, evnt/sonoff-54/BoilerHeaterMode, Mem1  endon on Event#BoilerHeaterEvent do Mem1 = %value% endon
Rule3 1

Rule3 based on: https://github.com/arendst/Tasmota/wiki/Subscribe-&-Unsubscribe
mosquitto_pub -h piwnica -t evnt/sonoff-54/BoilerHeaterMode -m {"Mem1":"1"}
stat/sonoff-54/RESULT = {"Mem1":"1.000"}
