#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "secrets.h"

#define RELAY 0
WiFiServer server(80);
void setup(){
  delay(2200);
  Serial.begin(115200);
  pinMode(RELAY,OUTPUT);
  digitalWrite(RELAY, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID, SECRET_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  
  setupOTA();
  
  server.begin();
}
 
void loop(){
  ArduinoOTA.handle();
  WiFiClient client = server.available();
  if (!client)                                
  {
    return;
  }
  while(!client.available())
  {
    delay(1);
  }

  routeRequest(client);
  delay(1);
  //client.stop();
}

void routeRequest(WiFiClient client){
  String request = client.readStringUntil('\r');
  client.flush();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/json");
  client.println("");
  int value = LOW;
  if (request.indexOf("/STATUS") != -1)
  {
    client.println(status());
  } else if (request.indexOf("/RELAY=ON") != -1)  
  {
    client.println(switchState(LOW));
  } else if (request.indexOf("/RELAY=OFF") != -1)  
  {
    client.println(switchState(HIGH));
  }
}

String status(){
  return getStatusJson(digitalRead(RELAY));
}

String switchState(int value){
  digitalWrite(RELAY,value);
  return getStatusJson(value);
}

String getStatusJson(int value){
  if (value == LOW)
    return "{\"status\": \"ON\"}";
  else if (value == HIGH)
    return "{\"status\": \"OFF\"}";
  else
    return "{\"status\": \"UNDEFINED\"}";
}

void setupOTA(){
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
