void setup() {
  Serial.begin(115200);

  // Sensoren Starten
  DS18B20.begin();

  // Dateisystem laden
  ESP.wdtFeed();
  if (!SPIFFS.begin())  {
    DBG_PRINTLN("SPIFFS Mount failed");
  }

  // Set device name
  snprintf(mqtt_clientid, 25, "ESP8266-%08X", mqtt_chip_key);

  // Einstellungen laden
  ESP.wdtFeed();
  loadConfig();

  // WiFi Manager
  ESP.wdtFeed();
  WiFi.hostname(mqtt_clientid);
  WiFiManagerParameter cstm_mqtthost("host", "cbpi ip", mqtthost, 16);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&cstm_mqtthost);
  wifiManager.autoConnect("MQTTDevice");
  strcpy(mqtthost, cstm_mqtthost.getValue());

  // Änderungen speichern
  ESP.wdtFeed();
  saveConfig();

  // ArduinoOTA aktivieren
  setupOTA();

  // MQTT starten
  client.setServer(mqtthost, 1883);
  client.setCallback(mqttcallback);

  // Webserver starten
  ESP.wdtFeed();
  setupServer();
}

void setupServer() {

  server.on("/", handleRoot);

  server.on("/setupActor", handleSetActor);       // Einstellen der Aktoren
  server.on("/setupSensor", handleSetSensor);     // Einstellen der Sensoren

  server.on("/reqSensors", handleRequestSensors); // Liste der Sensoren ausgeben
  server.on("/reqActors", handleRequestActors);   // Liste der Aktoren ausgeben
  server.on("/reqInduction", handleRequestInduction);

  server.on("/reqSearchSensorAdresses", handleRequestSensorAddresses);
  server.on("/reqPins", handlereqPins);

  server.on("/reqSensor", handleRequestSensor);   // Infos der Sensoren für WebConfig
  server.on("/reqActor", handleRequestActor);     // Infos der Aktoren für WebConfig
  server.on("/reqIndu", handleRequestIndu);       // Infos der Indu für WebConfig

  server.on("/setSensor", handleSetSensor);       // Sensor ändern
  server.on("/setActor", handleSetActor);         // Aktor ändern
  server.on("/setIndu", handleSetIndu);           // Indu ändern

  server.on("/delSensor", handleDelSensor);       // Sensor löschen
  server.on("/delActor", handleDelActor);         // Aktor löschen

  server.on("/reboot", rebootDevice);             // reboots the whole Device
  server.on("/mqttOff", turnMqttOff);             // Turns off MQTT completly until reboot

  server.on("/reqMiscSet", handleRequestMiscSet); // Misc menu
  server.on("/reqMisc", handleRequestMisc);       // Misc Infos für WebConfig
  server.on("/setMisc", handleSetMisc);           // Misc ändern

  // FSBrowser initialisieren
  server.on("/list", HTTP_GET, handleFileList);   // list directory
  server.on("/edit", HTTP_GET, []() {             // load editor
    if (!handleFileRead("/edit.htm")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/edit", HTTP_PUT, handleFileCreate); // create file
  server.on("/edit", HTTP_DELETE, handleFileDelete);  // delete file
  // first callback is called after the request has ended with all parsed arguments
  // second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  // server.onNotFound(handleWebRequests);           // Sonstiges

  server.begin();
}

void setupOTA() {
  DBG_PRINT("Configuring OTA device...");
  TelnetServer.begin();   //Necesary to make Arduino Software autodetect OTA device
  ArduinoOTA.onStart([]() {
    DBG_PRINTLN("OTA starting...");
  });
  ArduinoOTA.onEnd([]() {
    DBG_PRINTLN("OTA update finished!");
    DBG_PRINTLN("Rebooting...");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DBG_PRINT("OTA in progress: ");
    DBG_PRINTLN((progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) DBG_PRINTLN("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DBG_PRINTLN("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DBG_PRINTLN("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DBG_PRINTLN("Receive Failed");
    else if (error == OTA_END_ERROR) DBG_PRINTLN("End Failed");
  });
  ArduinoOTA.begin();
  DBG_PRINTLN("OTA OK");
}
