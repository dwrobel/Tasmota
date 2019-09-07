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
Rule1 on tele-DS18B20-2#Temperature<10 do POWER1 ON endon on tele-DS18B20-2#Temperature>40 do POWER1 OFF endon
Rule1 1
Rule2 on tele-DS18B20-3#Temperature<19 do POWER2 ON endon on tele-DS18B20-3#Temperature>20 do POWER2 OFF endon
Rule2 1
