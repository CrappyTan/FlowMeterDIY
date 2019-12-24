/*
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
//#include <ArduinoJson.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define _MQTT_ROOT_WATER_MAIN_1 "home/utility/water"


void sendFlow(){
  // Disable the interrupt while calculating flow rate and sending the value to
    // the host

    
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;

    //unsigned int frac;

//    StaticJsonBuffer<150> jsonBuffer;
//    JsonObject& root = jsonBuffer.createObject();
//
//    root["flowMainR"] = flowRate;
//    root["flowMainmL"] = flowMilliLitres;
//    root["time"] = timeClient.getFormattedTime();
//   
    
//    // Print the flow rate for this second in litres / minute
//    Serial.print("Flow rate: ");
//    Serial.print(int(flowRate));  // Print the integer part of the variable
//    Serial.print(".");             // Print the decimal point
//    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
//    frac = (flowRate - int(flowRate)) * 10;
//    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
//    Serial.print("L/min");
//    // Print the number of litres flowed in this second
//    Serial.print("  Current Liquid Flowing: ");             // Output separator
//    Serial.print(flowMilliLitres);
//    Serial.print("mL/Sec");
//
//    // Print the cumulative total of litres flowed since starting
//    Serial.print("  Output Liquid Quantity: ");             // Output separator
//    Serial.print(totalMilliLitres);
//    Serial.println("mL"); 
      
//    root.printTo(Serial);
//    Serial.println();
//    char json[150];
//    root.printTo(json, sizeof(json));

    if (! mqtt_water_main.publish("")) {
      Serial.println(F("Sending MQTT Failed"));
      digitalWrite(statusLed, LOW);
      delay(100);
      digitalWrite(statusLed, HIGH);
    }else{
       LED_STATE = !LED_STATE;  
       digitalWrite(statusLed, LED_STATE);
    }

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
//    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }

*/
