void rebootDevice() {
  server.send(200, "text/plain", "rebooting...");
  unsigned long last = 0;
  if (millis() > last + 1000)
  {
    // get rid off delay
  }
  // delay(1000);
  // CAUTION: known (library) issue: only works if you (hardware)button-reset once after flashing the device
  ESP.restart();
}

// TODO: Implement
void turnMqttOff() {
  for (int i = 0; i < numberOfActors; i++) {
    if (actors[i].isOn) {
      DBG_PRINT("Set actor ");
      DBG_PRINT(i);
      DBG_PRINTLN(" off due to MQTT disable");
      actors[i].isOn = false;
      actors[i].Update();
      //actors[i].publishmqtt();  // not yet ready on plugin side
    }
  }
  // Stop induction
  if (inductionCooker.isInduon) {
    DBG_PRINTLN("Set induction off due to MQTT disabled");
    inductionCooker.isInduon = false;
    inductionCooker.Update();
    //inductionCooker.publishmqtt(); // not yet ready on plugin side
  }
  mqttCommunication = false;
  server.send(200, "text/plain", "CAUTION! I don't work yet: turned off, please reboot to turn on again...");
}
