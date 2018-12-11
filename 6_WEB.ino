void handleRoot(){
  server.sendHeader("Location", "/index.html",true);   //Redirect to our html web page
  server.send(302, "text/plane","");
}

void handleWebRequests(){
  if(loadFromSpiffs(server.uri())) { return; }
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {}

  dataFile.close();
  return true;
}

void mqttreconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      for (int i = 0; i < numberOfActors; i++) {
        actors[i].mqtt_subscribe();
        inductionCooker.mqtt_subscribe();
        yield();
      }
    }
  }
}

void mqttcallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Received MQTT");
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  } Serial.println(" ");
    char payload_msg[length];
    for (int i = 0; i < length; i++) {
       payload_msg[i] = payload[i];
    }
        
  if (inductionCooker.mqtttopic == topic) {
    Serial.println("passing mqtt to induction");
    inductionCooker.handlemqtt(payload_msg);
  }
  for (int i = 0; i < numberOfActors; i++) {
    if (actors[i].argument_actor == topic) {
      Serial.print("passing mqtt to actor ");
      Serial.println(actors[i].name_actor);
      actors[i].handlemqtt(payload_msg);
    }
    yield();
  }      
}