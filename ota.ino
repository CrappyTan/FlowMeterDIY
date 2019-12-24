void otaSetup(){

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  ArduinoOTA.setPassword("Crappy123");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    printDebug("Start updating " + String(type));
  });
  ArduinoOTA.onEnd([]() {
    printDebug(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    printDebug("Progress: " + String(progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    printDebug("Error[%u]: " + String(error));
    if (error == OTA_AUTH_ERROR) {
      printDebug(F("Auth Failed"));
    } else if (error == OTA_BEGIN_ERROR) {
      printDebug(F("Begin Failed"));
    } else if (error == OTA_CONNECT_ERROR) {
      printDebug(F("Connect Failed"));
    } else if (error == OTA_RECEIVE_ERROR) {
      printDebug(F("Receive Failed"));
    } else if (error == OTA_END_ERROR) {
      printDebug(F("End Failed"));
    }
  });
  ArduinoOTA.begin();


  
}
