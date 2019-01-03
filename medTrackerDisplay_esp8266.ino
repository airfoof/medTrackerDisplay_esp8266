#include <EasyButton.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "SSD1306Wire.h"
#include "images.h"

SSD1306Wire display(0x3c, D2, D1);
int button1Pin = D4;
int button2Pin = D3;
EasyButton button1(button1Pin);
EasyButton button2(button2Pin);

// Update these with values suitable for your network.
const char* mqtt_server = "192.168.1.10";
const char* clientId = "medtracker-esp8266";
const char* drugTimeTopic = "medtracker/drugtime";
const char* medsNotTakenTopic = "medtracker/medsnottaken";
const char* medsTakenTopic = "medtracker/medstaken";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(String topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if(topic == drugTimeTopic){
      if(message == "on"){
        //displayText("DrugTime ON");
        displayDrugTimeImage();
      }else if(message == "off"){
        displayDrugsTakenImage();
        delay(1000);
        clearDisplay();
      }
  }
  if(topic == medsNotTakenTopic){
      displayMedsNotTakenImage(message);
  }
}

void displayText(String text) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 22, text);
  display.display();
}

void displayDrugTimeImage() {
  display.clear();
  display.drawXbm(0, 0, pill_small_icon_width, pill_small_icon_height, pill_small_icon_bits);
  display.display();
}

void displayDrugsTakenImage() {
  display.clear();
  display.drawXbm(0, 0, check_circle_width, check_circle_height, check_circle_bits);
  display.display();
}
void displayMedsNotTakenImage(String priority) {
  display.clear();
  if(priority == "1"){
    display.drawXbm(0, 0, alarm_icon_width, alarm_icon_height, alarm_icon_bits);
  }else if(priority == "2"){
    display.drawXbm(0, 0, alert_icon_width, alert_icon_height, alert_icon_bits);
  }else if(priority == "3"){
    display.drawXbm(0, 0, alert_icon_width, alert_icon_height, alert_icon_bits);
  }
  display.display();
}

void clearDisplay() {
  display.clear();
  display.display();
}

void handleButton1Press() {
  //displayText("Button1 pressed!");
  Serial.println("Button1 pressed!");

  client.publish(medsTakenTopic, clientId);
}

void handleButton2Press() {
  //displayText("Button2 pressed!");
  Serial.println("Button2 pressed!");
}

void reconnect() {
  // Loop until we're reconnected
  Serial.println("Reconnecting...");
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect(clientId)) {
      // ... and resubscribe
      client.subscribe(drugTimeTopic);
      client.subscribe(medsNotTakenTopic);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  // Initialising the UI will init the display too.
  display.init();
  display.setFont(ArialMT_Plain_10);
  
  displayText("Setting up WiFi...");
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  
  displayText("Setting up MQTT...");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  displayText("Setup Complete!!");
  delay(500);
  clearDisplay();
  
  button1.onPressed(handleButton1Press);
  button2.onPressed(handleButton2Press);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  button1.read();
  button2.read();

  client.loop();
}
