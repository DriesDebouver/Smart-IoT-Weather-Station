# Smart IoT Weather Station
## Smart IoT Weather Station based on Arduino Nano 33 IoT

### This smart weather station shows:
- Current date & time
- Inside temperature
- Outside temperature
- Inside humidity
- Outside humidity
- Light intensity
- Pressure
- Amount of rain ( not really: there is a water sensor on top but as an indicaiton, don't use this outside :) )
- Sunrise time
- Sunset time

The Arduino Nano 33 IoT is connected via WiFi and get's the outside temperature and outside humidity by the OpenWeatherMap API.
The Arduino also serves as a webserver so you can see the weather station's values remotely on another device.

### Used hardware:
- Arduino Nano 33 IoT
- Liquid Crystal Display (16x2)
- DHT11 Sensor
- BMP180 Sensor
- Water Sensor
- LDR Sensor
- 2-axis analog joystick
- 5mm Green LED
- 5mm Yellow LED
- 5mm Red LED

In the end I made a small case out of MDF but the time was limited:
![Done](https://github.com/DriesDebouver/Smart-IoT-Wheather-Station/blob/master/Done.jpg)

##### Endproject for the course "Embedded Systems - Arduino" in the Internet of Things education.
