void handleRoot() {
  server.sendHeader("Location", "/index.html", true);  //Redirect to our html web page
  server.send(302, "text/plain", "");
}

void handleWebRequests() {
  if (loadFromSpiffs(server.uri())) {
    return;
  }
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


bool loadFromSpiffs(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/")) path += "index.html";

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";

  if (!SPIFFS.exists(path.c_str())) {
    return false;
  }
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {}
  dataFile.close();
  return true;
}

void mqttreconnect() {
  // Create MQTT Name from Chip ID
  int mqtt_chip_key = ESP.getChipId();
  char mqtt_clientid[25];
  snprintf(mqtt_clientid, 25, "ESP8266-%08X", mqtt_chip_key);
  // 10 Tries for reconnect

  // Wenn Client nicht verbunden, Verbindung herstellen
  if (!client.connected()) {
    // Delay prüfen
    if (millis() > mqttconnectlasttry + mqttconnectdelay) {
      DBG_PRINT("MQTT Trying to connect. Name:");
      DBG_PRINT(mqtt_clientid);
      for (int i = 0; i < mqttnumberoftrys; i++) {
        DBG_PRINT(".. Try #");
        DBG_PRINT(i+1);
        if (client.connect(mqtt_clientid)) {
          DBG_PRINT(".. Success. Subscribing.");
          goto Subscribe;
        }
        delay(5);
      }
      mqttconnectlasttry = millis();
      DBG_PRINT(".. Failed. Trying again in ");
      DBG_PRINT(mqttconnectdelay/1000);
      DBG_PRINTLN(" seconds");
      return;
    }   
  }

  Subscribe:
    for (int i = 0; i < numberOfActors; i++) {
      actors[i].mqtt_subscribe();
      yield();
    }
    inductionCooker.mqtt_subscribe();

}


void mqttcallback(char* topic, byte* payload, unsigned int length) {
  DBG_PRINTLN("Received MQTT");
  DBG_PRINT("Topic: ");
  DBG_PRINTLN(topic);
  DBG_PRINT("Payload: ");

  for (int i = 0; i < length; i++) {
    DBG_PRINT((char)payload[i]);
  } DBG_PRINTLN(" ");
  char payload_msg[length];
  for (int i = 0; i < length; i++) {
    payload_msg[i] = payload[i];
  }

  if (inductionCooker.mqtttopic == topic) {
    DBG_PRINTLN("passing mqtt to induction");
    inductionCooker.handlemqtt(payload_msg);
  }
  for (int i = 0; i < numberOfActors; i++) {
    if (actors[i].argument_actor == topic) {
      DBG_PRINT("passing mqtt to actor ");
      DBG_PRINTLN(actors[i].name_actor);
      actors[i].handlemqtt(payload_msg);
    }
    yield();
  }
}
