/*
 * PD-01RGB-WIFI1 Software version v1.0
 * JC Design 
 */
#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#include <MQTTClient.h>
#include <HSBColor.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//Uncomment to enable debug messages via serial port
#define SERIAL_DEBUG

//define your default values here, if there are different values in config.json, they are overwritten.
//length should be max size + 1
char mqtt_server_ip[40] = "10.0.1.10";
char mqtt_port[6] = "1883";
char mqtt_device_id[40] = "PD-01RGB-WIFI1";

char temperature_topic[40];
char time_between_temp_read[5] = "60";
char button_topic[40];
char button_check[2];
char rgb_topic[40];

//default custom static IP
char static_ip[16] = "10.0.1.11";
char static_gw[16] = "10.0.1.1";
char static_sn[16] = "255.255.255.0";

WiFiClient net;
MQTTClient client;

// Wifi manager variables
#define startConfigPin 0          //Choose a compatible pin, When low will reset the wifi manager config
bool shouldSaveConfig = true;
#define builtin_led 2
#define AUTOCONNECT true
#define CONFIGPORTAL false

// Touch button variables
#define BUTTON_PIN 5
bool enable_button = true;
char custom_button_html[27];
bool button_state = false;
bool button_config = false;

// Temperature sensor variables
#define TEMP_PIN 4
#define temp_offset -3            //Temperature offset value
float temp;                       //Stores temperature value
String s_temp;
char messTemp[10];
OneWire ds(TEMP_PIN);                    //DS18b20 on gpio4
DallasTemperature dsTemp(&ds);
bool start = true;
unsigned long Lasttime = 0;

// bool offline = false;
// int counter = 0;
// long last_client_millis = 0;

// #define WIFI_DELAY 2000
// #define CLIENT_DELAY  2000      //time in milliseconds

// LED output variables
struct LedPin {
  byte red;
  byte green;
  byte blue;
};
LedPin ledPin = {13, 12, 14};
int LedValue[3] = {0, 0, 0};

// Wifi handling variables
bool mqtt_connected = false;
unsigned long last_client_connect = -1;        
unsigned long client_connect_delay = 1000;      //delay between checking connection, in milliseconds

/********************************/
void saveConfigCallback();
void loadConfig();
void saveConfig();
void handleWifiManager(bool autoconnect);
void messageReceived(String &topic, String &payload);
void reconnect();

void setup() {
  #ifdef SERIAL_DEBUG
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nJC Design");
    Serial.println("PD-01RGB-WIFI1");
    Serial.println("Start program...");
  #endif
  pinMode(ledPin.red, OUTPUT);
  pinMode(ledPin.green, OUTPUT);
  pinMode(ledPin.blue, OUTPUT);
  pinMode(builtin_led, OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  
  analogWrite(ledPin.red, 0);
  analogWrite(ledPin.green, 0);
  analogWrite(ledPin.blue, 0);

  //Load config file parameters
  loadConfig();

  //WifiManager handler
  handleWifiManager(AUTOCONNECT);

  //Save config parameters into config file
  saveConfig();

  if(enable_button)
    pinMode(BUTTON_PIN, INPUT);
  
  delay(100);
  client.begin(mqtt_server_ip, net);
  client.onMessage(messageReceived);
}

void loop() {
  // wifi_handler();

  if(!client.connected()) {
    mqtt_connected = false;
    reconnect();
  } 
  
  if(mqtt_connected) {
    client.loop();
    delay(10);

    //Button reading
    if(enable_button) {
      if(!button_state && digitalRead(BUTTON_PIN) == HIGH) {
        button_state = true;
        client.publish(button_topic, "TOGGLE");
        #ifdef SERIAL_DEBUG
          Serial.println("Button Pressed");
        #endif
      }
      else if(digitalRead(BUTTON_PIN) == LOW) {
        button_state = false;
      }
    }
    
    //Temp reading
    if((!start && (int(millis() - Lasttime) > (atoi(time_between_temp_read)*1000))) || (start && millis() > 40000)) {
      if(start) {
        dsTemp.requestTemperatures(); 
        delay(1000); 
      }
      dsTemp.requestTemperatures();
      delay(20);
      temp = dsTemp.getTempCByIndex(0) + temp_offset;
      String s_temp = String(temp);
      s_temp.toCharArray(messTemp, s_temp.length()+1);
      client.publish(temperature_topic, messTemp);
      start = false;
      Lasttime = millis();
      #ifdef SERIAL_DEBUG
        Serial.println(temp);
      #endif
    }
  }

  //if gpio0 pressed go to AP mode for config
  if(digitalRead(startConfigPin) == LOW) {
    #ifdef SERIAL_DEBUG
      Serial.println("Enter On Demand AP...");
    #endif

    //WifiManager handler
    handleWifiManager(CONFIGPORTAL);

    //Save config parameters into config file
    saveConfig();

    #ifdef SERIAL_DEBUG
      Serial.println("Exit On Demand AP...");
    #endif
  }
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  #ifdef SERIAL_DEBUG
    Serial.println("Should save config");
  #endif
  shouldSaveConfig = true;
  //save the custom parameters to FS
//  if(button_config) {
//    #ifdef SERIAL_DEBUG
//      Serial.println("saving config");
//    #endif
//    DynamicJsonDocument json(1024);
//
//    json["ip"] = WiFi.localIP().toString();
//    json["gateway"] = WiFi.gatewayIP().toString();
//    json["subnet"] = WiFi.subnetMask().toString();
//
//    json["mqtt_server_ip"] = mqtt_server_ip;
//    json["mqtt_port"] = mqtt_port;
//    json["mqtt_device_id"] = mqtt_device_id;
//    json["temperature_topic"] = temperature_topic;
//    json["time_between_temp_read"] = time_between_temp_read;
//    json["button_check"] = button_check;
//    json["button_topic"] = button_topic;
//    json["rgb_topic"] = rgb_topic;
//
//    File configFile = SPIFFS.open("/config.json", "w");
//    if (!configFile) {
//      #ifdef SERIAL_DEBUG
//        Serial.println("failed to open config file for writing");
//      #endif
//    }
//
//    serializeJson(json, Serial);
//    serializeJson(json, configFile);
//
//    configFile.close();
//    button_config = false;
//    //end save
//  }
}

void loadConfig() {
  #ifdef SERIAL_DEBUG
    Serial.println("mounting FS...");
  #endif

  if(SPIFFS.begin()) {
    #ifdef SERIAL_DEBUG
      Serial.println("mounted file system");
    #endif

    if(SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      #ifdef SERIAL_DEBUG
        Serial.println("reading config file");
      #endif
      File configFile = SPIFFS.open("/config.json", "r");
      if(configFile) {
        #ifdef SERIAL_DEBUG
          Serial.println("opened config file");
        #endif
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buffer(new char[size]);

        configFile.readBytes(buffer.get(), size);

        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buffer.get());
        serializeJson(json, Serial);
        if(!deserializeError) {
          #ifdef SERIAL_DEBUG
            Serial.println("\nparsed json");
          #endif

          //IP settings
          if(json["ip"]) {
            #ifdef SERIAL_DEBUG
              Serial.println("setting custom ip from config");
            #endif

            strcpy(static_ip, json["ip"]);
            strcpy(static_gw, json["gateway"]);
            strcpy(static_sn, json["subnet"]);
          } 
          else {
            #ifdef SERIAL_DEBUG
              Serial.println("no custom ip in config");
            #endif
          }

          //MQTT settings
          strcpy(mqtt_server_ip,          json["mqtt_server_ip"]);
          strcpy(mqtt_port,               json["mqtt_port"]);
          strcpy(mqtt_device_id,          json["mqtt_device_id"]);
          strcpy(temperature_topic,       json["temperature_topic"]);
          strcpy(time_between_temp_read,  json["time_between_temp_read"]);
          strcpy(button_check,            json["button_check"]);
          strcpy(button_topic,            json["button_topic"]);
          strcpy(rgb_topic,               json["rgb_topic"]);
        } 
        else {
          #ifdef SERIAL_DEBUG
            Serial.println("failed to load json config");
          #endif
        }
      }
    }
  } 
  else {
    #ifdef SERIAL_DEBUG
      Serial.println("failed to mount FS");
    #endif
  }

  #ifdef SERIAL_DEBUG
    Serial.println(static_ip);
    Serial.println(mqtt_server_ip);
    Serial.println(mqtt_port);
  #endif
}

void saveConfig() {
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    #ifdef SERIAL_DEBUG
      Serial.println("saving config");
    #endif
    DynamicJsonDocument json(1024);
    json["ip"] = WiFi.localIP().toString();
    json["gateway"] = WiFi.gatewayIP().toString();
    json["subnet"] = WiFi.subnetMask().toString();

    json["mqtt_server_ip"] = mqtt_server_ip;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_device_id"] = mqtt_device_id;
    json["temperature_topic"] = temperature_topic;
    json["time_between_temp_read"] = time_between_temp_read;
    json["button_check"] = button_check;
    json["button_topic"] = button_topic;
    json["rgb_topic"] = rgb_topic;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      #ifdef SERIAL_DEBUG
        Serial.println("failed to open config file for writing");
      #endif
    }

    serializeJson(json, Serial);  Serial.println("");
    serializeJson(json, configFile);
    configFile.close();
    //end save
  }
}

void handleWifiManager(bool autoconnect) {
  digitalWrite(builtin_led, LOW);       //Drive pin low to turn on built in LED

  WiFiManager wm;

  //set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);

  // create parameters
  WiFiManagerParameter custom_mqtt_server_ip("mqtt_server_ip", "MQTT server ip (max. 60 characters)", mqtt_server_ip, 60);
  WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT port (max. 6 characters)", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_device_id("mqtt_device_id", "MQTT device id (max. 60 characters)", mqtt_device_id, 60);
  WiFiManagerParameter custom_temp_topic("temp_topic", "Temperature sensor topic (max. 60 characters)", temperature_topic, 60);
  WiFiManagerParameter custom_temp_time("temp_time", "Time between temp readings in seconds (max. 6 characters)", time_between_temp_read, 6);
  
  enable_button = atoi(button_check);
  strcpy(custom_button_html, "type=\"checkbox\"");      //reference: https://github.com/kentaylor/WiFiManager/blob/master/examples/ConfigOnSwitchFS/ConfigOnSwitchFS.ino
  if(enable_button) {
    strcat(custom_button_html, " checked");
  }
  #ifdef SERIAL_DEBUG
    Serial.print("Enable button value: ");  Serial.println(enable_button);
    Serial.print("Custom html: ");  Serial.println(custom_button_html);
  #endif
  WiFiManagerParameter custom_button_enable("touchbutton", "Enable Touch Button", "T", 2, custom_button_html, WFM_LABEL_BEFORE);
  WiFiManagerParameter custom_button_topic("button_topic", "Button topic (max. 60 characters)", button_topic, 60);
  WiFiManagerParameter custom_rgb_topic("rgb_topic", "RGB output topic (max. 60 characters)", rgb_topic, 60);

  //set static ip
  IPAddress _ip, _gw, _sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);

  wm.setSTAStaticIPConfig(_ip, _gw, _sn);

  //add parameters
  wm.addParameter(&custom_mqtt_server_ip);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_device_id);
  wm.addParameter(&custom_temp_topic);
  wm.addParameter(&custom_temp_time);
  wm.addParameter(&custom_button_enable);
  wm.addParameter(&custom_button_topic);
  wm.addParameter(&custom_rgb_topic);

  std::vector<const char *> menu = {"wifi","param","info","sep","restart","exit"};
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");

  wm.setTimeout(300);

  //Start wifimanager portal
  if(autoconnect && !wm.autoConnect("PD-01RGB-WIFI1", "JCDesign")) {
    #ifdef SERIAL_DEBUG
      Serial.println("Failed to connect and hit timeout");
    #endif
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }
  else if(!autoconnect && !wm.startConfigPortal("PD-01RGB-WIFI1", "JCDesign")) {
    #ifdef SERIAL_DEBUG
      Serial.println("Exit config portal");
    #endif
  }
  else {
    #ifdef SERIAL_DEBUG
      Serial.println("Connected to WiFi network!");
    #endif
  }

  strcpy(mqtt_server_ip, custom_mqtt_server_ip.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_device_id, custom_mqtt_device_id.getValue());
  strcpy(temperature_topic, custom_temp_topic.getValue());
  strcpy(time_between_temp_read, custom_temp_time.getValue());
  
  enable_button = (strncmp(custom_button_enable.getValue(), "T", 1) == 0);
  itoa(enable_button, button_check, 10);
  #ifdef SERIAL_DEBUG
    Serial.print("button enable: ");
    Serial.print(enable_button);
    Serial.print(", char value: ");
    Serial.println(button_check);
  #endif

  strcpy(button_topic, custom_button_topic.getValue());
  strcpy(rgb_topic, custom_rgb_topic.getValue());

  digitalWrite(builtin_led, HIGH);      //Drive pin high to turn off built in LED
}

void messageReceived(String &topic, String &payload) {
  String msgTopic = topic;
  String msgString = payload;
  #ifdef SERIAL_DEBUG
    Serial.print(msgTopic);
    Serial.print(": ");
    Serial.println(msgString);
  #endif

  if(msgTopic == rgb_topic) {
    if(msgString == "ON") {
      analogWrite(ledPin.red, 1023);
      analogWrite(ledPin.green, 1023);
      analogWrite(ledPin.blue, 1023);
      #ifdef SERIAL_DEBUG
        Serial.println("1023,1023,1023");
      #endif
    }
    else if(msgString == "OFF") {
      analogWrite(ledPin.red, 0);
      analogWrite(ledPin.green, 0);
      analogWrite(ledPin.blue, 0);
      #ifdef SERIAL_DEBUG
        Serial.println("0,0,0");
      #endif
    }
    else {
      int firstComma = msgString.indexOf(',') + 1;                  //find first comma in string
      int secondComma = msgString.indexOf(',', firstComma + 1);     //find second comma in string
      int thirdComma = msgString.indexOf(',', secondComma + 1);     //find third comma in string
      String hue = msgString.substring(0, (firstComma - 1));
      String saturation = msgString.substring(firstComma, secondComma);
      String brightness = msgString.substring((secondComma+1), thirdComma);

      #ifdef SERIAL_DEBUG
        Serial.print(hue); Serial.print(",");
        Serial.print(saturation); Serial.print(",");
        Serial.println(brightness);
      #endif
    
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

      #ifdef SERIAL_DEBUG
        Serial.print(LedValue[0]); Serial.print(",");
        Serial.print(LedValue[1]); Serial.print(",");
        Serial.println(LedValue[2]);
      #endif
      
      analogWrite(ledPin.red, LedValue[0]);
      analogWrite(ledPin.green, LedValue[1]);
      analogWrite(ledPin.blue, LedValue[2]);
    }
  }
}

// void wifi_handler() {
//   if(WiFi.status() != WL_CONNECTED || !client.connected()) {
//     #ifdef SERIAL_DEBUG
//       Serial.print("wifi: "); Serial.println(WiFi.status());      //Keep the serial, otherwise it doesn't work (too fast)
//       Serial.print("Counter: ");  Serial.println(counter);
//     #endif
//     if(!offline) {
//       offline = true;
//     }
//     if((counter > WIFI_DELAY || counter == 0) && WiFi.status() != WL_CONNECTED) {
//       connect();
//       counter = 1;
//     }
//     counter++;
//   }
//   if(offline && WiFi.status() == WL_CONNECTED && ((millis() - last_client_millis) > CLIENT_DELAY)) {
//     Serial.println("check client..........................");
//     if(client.connect(mqttDeviceID)) {
//       offline = false;
//       counter = 0;
//       OTA_init();
//       client.subscribe(subscribeTopic1);
//       client.subscribe(subscribeTopic2);
//       Serial.println("Connected!");
//     }
//     last_client_millis = millis();
//   }
// }

void reconnect() {
  // Loop until we're reconnected
  if(!client.connected() && ((millis() - last_client_connect) > client_connect_delay)) {
    #ifdef SERIAL_DEBUG
      Serial.print("\nAttempting MQTT connection...");
    #endif
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if(client.connect(mqtt_device_id)) {
      client.subscribe(rgb_topic);
      mqtt_connected = true;
      #ifdef SERIAL_DEBUG
        Serial.println("   Connected!");
      #endif
    } 
    last_client_connect = millis();
  }
}
