/*
 * PD-01RGB-WIFI1 Software version v1.0
 * JC Design 
 */
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <MQTTClient.h>
#include <HSBColor.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/*******  Change the settings to match your setup   ******/
/*******  Don't forget to change the OTA password in the OTA_init() function   ******/

const char* ssid = "Network SSID";            //Your network SSID
const char* password = "Network Password";    //Your network password

const char* server = "Broker IP";           //Your MQTT IP address
const char* mqttDeviceID = "Unique ID";     //The unique MQTT ID for this device

char* outTopic1 = "Out Topic Temperature Sensor";   //Topic for outgoing temperature value i.e.: Home/Sensor/Temperature
char* outTopic2 = "Out Topic Touch Button";         //Topic for sending a touch toggle

char* subscribeTopic1 = "Subscripbe Topic RGB LEDs";    //Topic for incoming RGB value i.e.: Home/Light/RGB
char* subscribeTopic2 = "Subscripbe Topic OTA";         //Topic to enable OTA upload i.e.: Home/OTA/RGBcontroller

IPAddress ip(192,168,0,21);                   //the desired static IP Address
IPAddress gateway(192,168,0,1);               //set gateway to match your network
IPAddress subnet(255,255,255,0);              //set subnet mask to match your network

#define TIME_BETWEEN_READING  60              //Time in seconds between each temperature reading, 60 sec = 1 min

#define USE_BUTTON          //Comment this line if you DON'T want to use the touch button.

/*******  Don't change anything after this line   ******/

WiFiClient net;
MQTTClient client;

/********************************/
#ifdef USE_BUTTON
  #define BUTTON_PIN 5
  bool button_state = false;
#endif

#define temp_offset -3                //Temperature offset value
float temp; //Stores temperature value
String s_temp;
char messTemp[10];
OneWire ds(4);     //DS18b20 on gpio4
DallasTemperature dsTemp(&ds);

bool offline = false;
int counter = 0;
long last_client_millis = 0;

#define WIFI_DELAY 2000
#define CLIENT_DELAY  2000      //time in milliseconds

struct LedPin {
  byte red;
  byte green;
  byte blue;
};
LedPin ledPin = {13, 12, 14};

int LedValue[3] = {0, 0, 0};
bool update_req = false;
bool start = true;
unsigned long Lasttime = 0;

void messageReceived(String &topic, String &payload) {
  String msgTopic = topic;
  String msgString = payload;
  Serial.print(msgTopic);
  Serial.print(": ");
  Serial.println(msgString);

  if(msgTopic == subscribeTopic1)
  {
    if(msgString == "ON")
    {
      analogWrite(ledPin.red, 1023);
      analogWrite(ledPin.green, 1023);
      analogWrite(ledPin.blue, 1023);
      Serial.println("1023,1023,1023");
    }
    else if(msgString == "OFF")
    {
      analogWrite(ledPin.red, 0);
      analogWrite(ledPin.green, 0);
      analogWrite(ledPin.blue, 0);
      Serial.println("0,0,0");
    }
    else
    {
      int firstComma = msgString.indexOf(',') + 1;                  //find first comma in string
      int secondComma = msgString.indexOf(',', firstComma + 1);     //find second comma in string
      int thirdComma = msgString.indexOf(',', secondComma + 1);     //find third comma in string
      String hue = msgString.substring(0, (firstComma - 1));
      String saturation = msgString.substring(firstComma, secondComma);
      String brightness = msgString.substring((secondComma+1), thirdComma);
    
      Serial.print(hue); Serial.print(",");
      Serial.print(saturation); Serial.print(",");
      Serial.println(brightness);
    
      H2R_HSBtoRGB(hue.toInt(), saturation.toInt(), brightness.toInt(), LedValue);

      LedValue[0] = (LedValue[0]*4)+3;
      LedValue[1] = (LedValue[1]*4)+3;
      LedValue[2] = (LedValue[2]*4)+3;
      
      if(LedValue[0] < 100)
        LedValue[0] = 0;
      else if(LedValue[0] > 980)
        LedValue[0] = 1023;
      if(LedValue[1] < 100)
        LedValue[1] = 0;
      else if(LedValue[1] > 980)
        LedValue[1] = 1023;
      if(LedValue[2] < 100)
        LedValue[2] = 0;
      else if(LedValue[2] > 980)
        LedValue[2] = 1023;

      Serial.print(LedValue[0]); Serial.print(",");
      Serial.print(LedValue[1]); Serial.print(",");
      Serial.println(LedValue[2]);
      
      analogWrite(ledPin.red, LedValue[0]);
      analogWrite(ledPin.green, LedValue[1]);
      analogWrite(ledPin.blue, LedValue[2]);
    }
  }

  if(msgTopic == subscribeTopic2)
  {
    if(msgString == "ON")
    {
      update_req = true;
    }
    else if(msgString == "OFF")
    {
      update_req = false;
    }
  }
}

void connect() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(500);
  WiFi.config(ip, gateway, subnet);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.begin(server, net);
  client.onMessage(messageReceived);
  delay(250);
}

void OTA_init() {
  //For more info, look at BasicOTA_Test
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(mqttDeviceID);

  // No authentication by default
  ArduinoOTA.setPassword("OTA password");

  ArduinoOTA.begin();
}

void wifi_manager() {
  if(WiFi.status() != WL_CONNECTED || !client.connected()) {
    Serial.print("wifi: "); Serial.println(WiFi.status());      //Keep the serial, otherwise it doesn't work (too fast)
    Serial.print("Counter: ");  Serial.println(counter);
    if(!offline) {
      offline = true;
    }
    if((counter > WIFI_DELAY || counter == 0) && WiFi.status() != WL_CONNECTED) {
      connect();
      counter = 1;
    }
    counter++;
  }
  if(offline && WiFi.status() == WL_CONNECTED && ((millis() - last_client_millis) > CLIENT_DELAY)) {
    Serial.println("check client..........................");
    if(client.connect(mqttDeviceID)) {
      offline = false;
      counter = 0;
      OTA_init();
      client.subscribe(subscribeTopic1);
      client.subscribe(subscribeTopic2);
      Serial.println("Connected!");
    }
    last_client_millis = millis();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("JC Design\nPD-01RGB-WIFI1\nStart program...");
  pinMode(ledPin.red, OUTPUT);
  pinMode(ledPin.green, OUTPUT);
  pinMode(ledPin.blue, OUTPUT);
  pinMode(5, INPUT);
  #ifdef USE_BUTTON
    pinMode(BUTTON_PIN, INPUT);
  #endif
  
  analogWrite(ledPin.red, 0);
  analogWrite(ledPin.green, 0);
  analogWrite(ledPin.blue, 0);

  delay(100);
  connect();
}

void loop() {
  wifi_manager();
  
  client.loop();
  delay(10);

  if(update_req) {
    ArduinoOTA.handle();
  }

  #ifdef USE_BUTTON
    if(!button_state && digitalRead(BUTTON_PIN) == HIGH) {
      button_state = true;
      client.publish(outTopic2, "TOGGLE");
      Serial.println("Button Pressed");
    }
    else if(digitalRead(BUTTON_PIN) == LOW) {
      button_state = false;
    }
  #endif
  
  if(!update_req && ((!start && ((millis() - Lasttime) > (TIME_BETWEEN_READING*1000))) || (start && millis() > 40000))) {
    if(start) {
      dsTemp.requestTemperatures(); 
      delay(1000); 
    }
    dsTemp.requestTemperatures();
    delay(20);
    temp = dsTemp.getTempCByIndex(0) + temp_offset;
    String s_temp = String(temp);
    s_temp.toCharArray(messTemp, s_temp.length()+1);
    client.publish(outTopic1, messTemp);
    start = false;
    Lasttime = millis();
    Serial.println(temp);
  }
}
