#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

String ssid="", ssid_ant="", password="", password_ant="";

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void connectOtherNetwork() {
  byte intentos=0;
  ssid=server.arg("networkID");
  password=server.arg("psswrd");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    intentos++;
    if (intentos>60){
      Serial.println("problemas de conexion");
      if(ssid_ant!=""){
        WiFi.begin(ssid_ant, password_ant);
      }else{
        WiFi.softAP("espNet");
      }
      break;
    }
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    ssid_ant=ssid;
    password_ant=password;
  }
}

void networkSelection() {
  int scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/false);
  String html=R"(<!DOCTYPE html>
    <html lang="en">
    <head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Seleccion de red</title>
    <style>
        form{
            text-align: center;
        }
        input, select{
            display: block;
            margin: auto;
            margin-bottom: 10px;
        }
    </style>
    </head>
    <body>
    <form action="/connect" method="post">
        <select name="networkID">)";

    if (scanResult == 0) {
    html+="<h4>No networks found</h4>";
    } else if (scanResult > 0) {
      for (int8_t i = 0; i < scanResult; i++) {
        html+="<option value=\""+WiFi.SSID(i)+"\">"+WiFi.SSID(i)+" "+String(WiFi.RSSI(i))+"</option>";
      }
    }
    
    html+=R"(<input type="password" name="psswrd">
            <input type="submit" value="Connect">
            </form>
            </body>
            </html>)";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP("espNet"); //ssid, password
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/connect", connectOtherNetwork);
  server.on("/selection", networkSelection);
  server.begin();
  Serial.println("HTTP server started");

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("myesp8266");
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
