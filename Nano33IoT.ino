/*
 * Smart IoT Weather Station
 * Made by Dries Debouver in March 2020
 */

//LCD - Digital pins 2 to 7:
#include <LiquidCrystal.h>
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

//DHT11 Sensor - Digital pin 8:
#include "DHT.h"
#define DHTPIN 8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//BMP180 - SDA analog pin A4, SCL analog pin A5:
#include <SFE_BMP180.h>
#include <Wire.h>
SFE_BMP180 bmp;
char bmpStatus;
#define ALTITUDE 6.0   //Height from sea-level of your city 

//WiFi:
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFi.h>
char ssid[] = "...";   //WiFi name (SSID)
char pass[] = "...";   //WiFi pasword
int status = WL_IDLE_STATUS;
const int GMT = 1;   //Timezone

//Real Time myClock:
#include <RTCZero.h>
RTCZero rtc;

//WiFi outside weather - OpenWeatherMap.com:
#include <ArduinoJson.h>
String apiKey = "...";   //API Key OpenWeatherMap.org
String location = "2785294";   //City code
char server[] = "api.openweathermap.org";
WiFiClient client;

//WiFi Webserver:
WiFiServer server2(80);

//Blynk App Control:
#define BLYNK_PRINT Serial
#include <BlynkSimpleWiFiNINA.h>
char auth[] = "...";   //Blynk App authentication code

//Variables:
double tempDht;                   //Temperature DHT11 Sensor
int humiDht;                      //Humidity DHT11 Sensor
double tempBmp;                   //Temperature BMP180 Sensor
double presBmp;                   //Pressure BMP180 Sensor
double presBmpSea;                //Pressure relative at sea-level BMP180 Sensor
const int lightLdrPin = A0;       //Lightintensity (LDR) Sensor Pin
int lightLdr;                     //Lightintensity value
const int waterSenPin = A1;       //Water Sensor Signal Pin
int waterSen;                     //Water Sensor value
const int joystickVrxPin = A2;    //Joystick VrX Pin
int joystickX;                    //Joystick X value
const int joystickVryPin = A3;    //Joystick VrY Pin
int joystickY;                    //Joystick Y value
const int joystickSigPin = 9;     //Joystick Signaal Pin
int joystickS;                    //Joystick Signal/Switch value
int screen = 0;                   //Screen to change cases from LCD screen
int greenLedPin = 10;             //Green LED Digital pin
int yellowLedPin = 11;            //Yellow LED Digital pin
int redLedPin = 12;               //Red LED Digital pin
unsigned long sunriseTime;        //Sunrise time
unsigned long sunsetTime;         //Sunset time



void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  dht.begin();
  bmp.begin();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting up...");

  //Joystick button:
  pinMode(joystickSigPin, INPUT);
  digitalWrite(joystickSigPin, HIGH);

  //Status LED'S:
  pinMode(greenLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  //Green LED Blinking:
  for (int i = 0; i < 10; i++) {
    digitalWrite(greenLedPin, HIGH);
    delay(200);
    digitalWrite(greenLedPin, LOW);
    delay(200);
  }

  //WiFi:
  //check if the WiFi module works:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    digitalWrite(redLedPin, HIGH);
    delay(5000);
    digitalWrite(redLedPin, LOW);
    // don't continue:
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
    digitalWrite(redLedPin, HIGH);
    delay(5000);
    digitalWrite(redLedPin, LOW);
  }

  //attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting");
    lcd.setCursor(0, 1);
    lcd.print("to WiFi...");
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }

  //Start webserver:
  server2.begin();

  //WiFi is connected:
  printWiFiStatus();
  digitalWrite(greenLedPin, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected!");
  delay(2000);

  //Real Time myClock:
  rtc.begin();
  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));
  if (numberOfTries == maxTries) {
    Serial.print("RTC: NTP unreachable!!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC: Server RIP");
    while (1);
  } else {
    Serial.print("Epoch received: ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);
  }

  //Blynk:
  Blynk.begin(auth, ssid, pass);
}



void loop() {
  delay(500);   //Delay everyhting half a second
  Serial.println("\n***** ARDUINO WEATHER STATION *****");

  //Webserver:
  // listen for incoming clients
  WiFiClient client2 = server2.available();
  if (client2) {
    Serial.println("new client!");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client2.connected()) {
      if (client2.available()) {
        char c = client2.read();
        Serial.write(c);
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client2.println("HTTP/1.1 200 OK");
          client2.println("Content-Type: text/html");
          client2.println("Connection: close");  // the connection will be closed after completion of the response
          client2.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client2.println();
          client2.println("<!DOCTYPE HTML>");
          client2.println("<html>");
          //Text to print on the website:
          client2.print("Arduino Smart IoT Weather Station:");
          client2.print("<br />");
          client2.print("<br />");
          double tempDht2;
          tempDht2 = dht.readTemperature();
          client2.print("Inside temperatue: ");
          client2.print(tempDht2);
          client2.print(" C");
          client2.print("<br />");
          int humiDht2;
          humiDht2 = dht.readHumidity();
          client2.print("Inside humidity: ");
          client2.print(humiDht2);
          client2.print(" %");
          client2.print("<br />");
          client2.print("Lightintensity: ");
          client2.print(analogRead(lightLdrPin));
          client2.print(" (0 to 1023: low value = dark, high value = bright)");
          client2.print("<br />");
          client2.print("Rainsensor: ");
          client2.print(analogRead(waterSenPin));
          client2.print(" (0 to 1023: low value = dry, high value = wet)");
          //Tot hier
          client2.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client2.stop();
    Serial.println("client2 disonnected");
  }

  //Joystick values:
  joystickX = analogRead(joystickVrxPin);
  joystickY = analogRead(joystickVryPin);
  joystickS = digitalRead(joystickSigPin);

  //Joystick cordinates:
  Serial.println("\n- Joystick: -");
  Serial.print("X: ");
  Serial.println(joystickX);
  Serial.print("Y: ");
  Serial.println(joystickY);
  Serial.print("Signal: ");
  Serial.print(joystickS);
  Serial.println(" (1 = Not pressed, 0 = Pressed)");

  //Temperature:
  bmpStatus = bmp.startTemperature();
  delay(bmpStatus);
  bmpStatus = bmp.getTemperature(tempBmp);

  //Humidity:
  humiDht = dht.readHumidity();

  //Different screen on the LCD:
  if (joystickX > 1000 || joystickX < 50 || joystickY < 50 || joystickY > 1000) { //If joystick moves left, right, up or down
    if (joystickX > 1000 || joystickY > 1000) { //Joystick to the right or down:
      screen++; //Next screen.
      digitalWrite(yellowLedPin, HIGH);
      delay(500);
      digitalWrite(yellowLedPin, LOW);
    }
    if (joystickX < 50 || joystickY < 50) { //Joystick to the left or up:
      screen--; //Previous screen.
      digitalWrite(yellowLedPin, HIGH);
      delay(500);
      digitalWrite(yellowLedPin, LOW);
    }
  }

  if (screen > 9 || screen < 0) { //If "screen" amount is bigger then the amount of possible screens or low then 0:
    screen = 0; //Reset the screen
  }

  switch (screen) {
    case 0:
      myClock();
      break;
    case 1:
      insideTemp();
      break;
    case 2:
      insideHumi();
      break;
    case 3:
      insidePress();
      break;
    case 4:
      insideLight();
      break;
    case 5:
      rainSensor();
      break;
    case 6:
      outsideTemp();
      break;
    case 7:
      outsideHumi();
      break;
    case 8:
      sunrise();
      break;
    case 9:
      sunset();
      break;
    default: //The code will never do this but it's good practice
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("- WEATHER: -");
      break;
  }
  Serial.print("\n***** Screen: ");
  Serial.print(screen);
  Serial.println(" *****");

  //Blynk:
  Blynk.run();
  Blynk.virtualWrite(V0, tempBmp);
  Blynk.virtualWrite(V1, humiDht);
}



//FUNCTIONS:
void myClock() {
  //Print on Serial Monitor:
  Serial.println("\n- WiFi RTC: -");
  printDate(); //Standard RTC function, is down below
  printTime(); //Standard RTC function, is down below
  //Print on LCD:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(rtc.getDay());
  lcd.print("/");
  lcd.print(rtc.getMonth());
  lcd.print("/");
  lcd.print(2000 + rtc.getYear());
  lcd.setCursor(0, 1);
  lcd.print(rtc.getHours() + GMT);
  lcd.print(":");
  lcd.print(rtc.getMinutes());
  lcd.print(":");
  lcd.print(rtc.getSeconds());
}

void insideTemp() {
  //Print on Serial Monitor:
  Serial.println("\n- BMP180 Sensor: -");
  Serial.print("Inside Temperature: ");
  Serial.print(tempBmp);
  Serial.println(" °C");
  //Print on LCD:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inside temp:");
  lcd.setCursor(0, 1);
  lcd.print(tempBmp);
  lcd.print(" ");
  lcd.print((char)223); // " ° " degrees symbol
  lcd.print("C");
}

void insideHumi() {
  //DHT11 Sensor:
  tempDht = dht.readTemperature();
  //Print on Serial monitor:
  Serial.println("\n- DHT11 Sensor: -");
  Serial.print("Inside Temperature: ");
  Serial.print(tempDht);
  Serial.println(" °C");
  Serial.print("Inside Humidity: ");
  Serial.print(humiDht);
  Serial.println(" %");
  //Print on LCD:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humidity:");
  lcd.setCursor(0, 1);
  lcd.print(humiDht);
  lcd.print(" %");
}

void insidePress() {
  //BMP180 Sensor:
  bmpStatus = bmp.startTemperature();
  if (bmpStatus != 0) {
    delay(bmpStatus);
    bmpStatus = bmp.getTemperature(tempBmp);
    if (bmpStatus != 0) {
      bmpStatus = bmp.startPressure(3);
      if (bmpStatus != 0) {
        delay(bmpStatus);
        bmpStatus = bmp.getPressure(presBmp, tempBmp);
        if (bmpStatus != 0) {
          presBmpSea = bmp.sealevel(presBmp, ALTITUDE);
          //Print on Serial Monitor:
          Serial.print("Pressure: ");
          Serial.print(presBmp);
          Serial.println(" mb / hPa");
          Serial.print("Height: ");
          Serial.print(ALTITUDE, 0);
          Serial.println(" meter");
          Serial.print("Relative (sea-level) pressure: ");
          Serial.print(presBmpSea, 2);
          Serial.print(" mb, ");
          Serial.print(presBmpSea * 0.0295333727, 2);
          Serial.println(" inHg");
          //Print on LCD:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Pressure:");
          lcd.setCursor(0, 1);
          lcd.print(presBmp);
          lcd.print(" mb / hPa");
        }
      }
    }
  }
}

void insideLight() {
  //Lightintensity LDR Sensor
  lightLdr = analogRead(lightLdrPin);
  //Print on Serial Monitor:
  Serial.println("\n- LDR Sensor: -");
  Serial.print(lightLdr);
  if (lightLdr < 800) {
    Serial.println(" = Dark");
  } else {
    Serial.println(" = Bright");
  }
  //Print on LCD:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lightintensity:");
  lcd.setCursor(0, 1);
  lcd.print(lightLdr);
  if (lightLdr < 800) {
    lcd.print(" = Dark");
  } else {
    lcd.print(" = Bright");
  }
}

void rainSensor() {
  //Water Sensor:
  waterSen = analogRead(waterSenPin);
  //Print on Serial Monitor:
  Serial.println("\n- Water Sensor: -");
  Serial.print("Signal: ");
  Serial.print(waterSen);
  if (waterSen < 100) {
    Serial.println(" = Dry");
  } else if (waterSen > 100 && waterSen < 500) {
    Serial.println(" = Light rain");
  } else {
    Serial.println(" = Heavy rain");
  }
  //Print on LCD:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rain sensor:");
  lcd.setCursor(0, 1);
  lcd.print(waterSen);
  if (waterSen < 100) {
    lcd.print(" = Dry");
  } else if (waterSen > 100 && waterSen < 500) {
    lcd.print(" = Light rain");
  } else {
    lcd.print(" = Heavy rain");
  }
}

void outsideTemp() {
  Serial.println("\n- Outside Temperature via WiFi: -");
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request:
    client.print("GET /data/2.5/forecast?");
    client.print("id=" + location);
    client.print("&appid=" + apiKey);
    client.print("&cnt=1"); //amount of lists to get from server, 8 per day max
    client.println("&units=metric");
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Unable to connect");
    digitalWrite(redLedPin, HIGH);
    delay(5000);
    digitalWrite(redLedPin, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No connection");
    lcd.setCursor(0, 1);
    lcd.print("with server");
  }
  delay(2000);
  String line = "";
  while (client.connected()) {
    line = client.readStringUntil('\n');
    StaticJsonDocument<10000> doc;
    DeserializationError error = deserializeJson(doc, line);
    if (error) {
      Serial.print("deserializeJson() failed with code ");
      Serial.println(error.c_str());
      digitalWrite(redLedPin, HIGH);
      delay(5000);
      digitalWrite(redLedPin, LOW);
      return;
    }
    //Data from JSON Tree - Outside Temperature:
    String tempWifi = doc["list"][0]["main"]["temp"];
    //Print on Serial Monitor:
    Serial.print("Outside Temperature: ");
    Serial.print(tempWifi);
    Serial.println(" °C");
    //Print on LCD:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Outside Temp:");
    lcd.setCursor(0, 1);
    lcd.print(tempWifi);
    lcd.print(" ");
    lcd.print((char)223);
    lcd.print("C");
  }
}

void outsideHumi() {
  Serial.println("\n- Outside Humidity via WiFi: -");
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request:
    client.print("GET /data/2.5/forecast?");
    client.print("id=" + location);
    client.print("&appid=" + apiKey);
    client.print("&cnt=1");
    client.println("&units=metric");
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Unable to connect");
    digitalWrite(redLedPin, HIGH);
    delay(5000);
    digitalWrite(redLedPin, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No connection");
    lcd.setCursor(0, 1);
    lcd.print("with server");
  }
  delay(2000);
  String line = "";
  while (client.connected()) {
    line = client.readStringUntil('\n');
    StaticJsonDocument<10000> doc;
    DeserializationError error = deserializeJson(doc, line);
    if (error) {
      Serial.print("deserializeJson() failed with code ");
      Serial.println(error.c_str());
      digitalWrite(redLedPin, HIGH);
      delay(5000);
      digitalWrite(redLedPin, LOW);
      return;
    }
    //Data from JSON Tree - Outside Humidity:
    String humiWifi = doc["list"][0]["main"]["humidity"];
    //Print on Serial Monitor:
    Serial.print("Vochtigheid buiten: ");
    Serial.print(humiWifi);
    Serial.println(" %");
    //Print on LCD:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Outside Humidity:");
    lcd.setCursor(0, 1);
    lcd.print(humiWifi);
    lcd.print(" %");
  }
}

void sunrise() {
  Serial.println("\n- Sunrise via WiFi: -");
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request:
    client.print("GET /data/2.5/forecast?");
    client.print("id=" + location);
    client.print("&appid=" + apiKey);
    client.print("&cnt=1");
    client.println("&units=metric");
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Unable to connect");
    digitalWrite(redLedPin, HIGH);
    delay(5000);
    digitalWrite(redLedPin, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No connection");
    lcd.setCursor(0, 1);
    lcd.print("with server");
  }
  delay(2000);
  String line = "";
  while (client.connected()) {
    line = client.readStringUntil('\n');
    StaticJsonDocument<10000> doc;
    DeserializationError error = deserializeJson(doc, line);
    if (error) {
      Serial.print("deserializeJson() failed with code ");
      Serial.println(error.c_str());
      digitalWrite(redLedPin, HIGH);
      delay(5000);
      digitalWrite(redLedPin, LOW);
      return;
    }
    //Data from JSON Tree - Sunrise:
    String sunriseJson = doc["city"]["sunrise"];
    //Unix time to time:
    sunriseTime = sunriseJson.toInt();
    rtc.setEpoch(sunriseTime);
    //Print on Serial Monitor:
    Serial.print("Sunrise: ");
    Serial.print(rtc.getHours() + GMT);
    Serial.print(":");
    Serial.print(rtc.getMinutes());
    //Print on LCD:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sunrise:");
    lcd.setCursor(0, 1);
    lcd.print(rtc.getHours() + GMT);
    lcd.print(":");
    lcd.print(rtc.getMinutes());
  }
}

void sunset() {
  Serial.println("\n- Sunset via WiFi: -");
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request:
    client.print("GET /data/2.5/forecast?");
    client.print("id=" + location);
    client.print("&appid=" + apiKey);
    client.print("&cnt=1");
    client.println("&units=metric");
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Unable to connect");
    digitalWrite(redLedPin, HIGH);
    delay(5000);
    digitalWrite(redLedPin, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No connection");
    lcd.setCursor(0, 1);
    lcd.print("with server");
  }
  delay(2000);
  String line = "";
  while (client.connected()) {
    line = client.readStringUntil('\n');
    StaticJsonDocument<10000> doc;
    DeserializationError error = deserializeJson(doc, line);
    if (error) {
      Serial.print("deserializeJson() failed with code ");
      Serial.println(error.c_str());
      digitalWrite(redLedPin, HIGH);
      delay(5000);
      digitalWrite(redLedPin, LOW);
      return;
    }
    //Data from JSON Tree - Sunset:
    String sunsetJson = doc["city"]["sunset"];
    //Unix time to time:
    sunsetTime = sunsetJson.toInt();
    rtc.setEpoch(sunsetTime);
    //Print on Serial Monitor:
    Serial.print("Sunset: ");
    Serial.print(rtc.getHours() + GMT);
    Serial.print(":");
    Serial.print(rtc.getMinutes());
    //Print on LCD:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sunset:");
    lcd.setCursor(0, 1);
    lcd.print(rtc.getHours() + GMT);
    lcd.print(":");
    lcd.print(rtc.getMinutes());
  }
}

void printDate() {
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(2000 + rtc.getYear());
  Serial.println();
}

void printTime() {
  print2digits(rtc.getHours() + GMT);
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.print(":");
  print2digits(rtc.getSeconds());
  Serial.println();
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
