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
Rule1 on DS18B20-1#Temperature do Var1 %value% endon on DS18B20-2#Temperature do Var2 %value% endon on Time#Minute do if ((Var1 < 35) AND (Var2 < 30)) POWER1 ON elseif ((Var1 > 35) OR (Var2 > 35)) POWER1 OFF endif endon
Rule1 1
Rule2 on tele-DS18B20-3#Temperature<5 do POWER2 ON endon on tele-DS18B20-3#Temperature>10 do POWER2 OFF endon
Rule2 1
