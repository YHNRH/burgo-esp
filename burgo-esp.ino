#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "secrets.h"



uint32_t Id = -1;

WiFiServer server(80);
WiFiUDP udp;

int udpServerPort = 41235;
#define RELAY 0
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
  }
  
  ArduinoOTA.begin();
  
  server.begin();
  while (!udp.begin(udpServerPort) ) {
    yield();
  }
}

void loop(){
  ArduinoOTA.handle();
  Id = ESP.getChipId();
  discoverResponder();

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
  String obj = "{\"Id\": ";
  obj += Id;
  obj += ", \"status\": \"";
  if (value == LOW)
    obj += "ON";
  else if (value == HIGH)
    obj += "OFF";
  else
    obj += "UNDEFINED";
  obj += "\"}";
  return obj;
}

void discoverResponder() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
      char incomingPacket[255] = ""; 
      int len = udp.read(incomingPacket, 255);
      if (len > 0) {
        if (String(incomingPacket) == "discover") {
          incomingPacket[len] = 0;
          udp.beginPacket(udp.remoteIP(),udp.remotePort());
          udp.printf(status().c_str());
          udp.endPacket();
        }
      }
    }
}
