
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
//include <ArduinoJson.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


#define _ONE_WIRE_BUS D5
#define _MQTT_HOST "mqtt.home"
#define _MQTT_HOST_PORT 1883
#define _NTP_SERVER "0.uk.pool.ntp.org"

#define _MQTT_ROOT_WATER_MAIN_FLOW_RATE "home/utility/water/flowrate"
#define _MQTT_ROOT_BOILER_TEMPERATURE "home/utility/boiler/temperature"


#define _DEBUG 1


//void ICACHE_RAM_ATTR ISRoutine ();


// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(_ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature temp_sensors(&oneWire);




WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, _NTP_SERVER);
byte statusLed    = BUILTIN_LED;
bool LED_STATE = LOW;

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = D2;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 8.0;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;
unsigned long oldTimeTemp;
WiFiClient espClient;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&espClient, _MQTT_HOST, _MQTT_HOST_PORT);


void printDebug(String msg){
  #ifdef _DEBUG
      Serial.print("[");
      Serial.print(millis());
      Serial.print("\t]\t");
      Serial.println(msg);    
  #endif
 }


/*
Insterrupt Service Routine
 */
void ICACHE_RAM_ATTR pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}


void setup()
{
  
  // Initialize a serial connection for reporting values to the host
  Serial.begin(115200);

  temp_sensors.begin(); 
  Serial.print("Temperature Sensors found: ");
  Serial.println(temp_sensors.getDeviceCount(), DEC);
  
  timeClient.begin();

  
  WiFiManager wifiManager;
  WiFi.hostname("WaterMeter");
  
  wifiManager.autoConnect("WaterMeterAP", "Qwerty123");

  otaSetup();
       
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;
  oldTimeTemp       = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)

  Serial.print("Setting up interrupts...");
  
  sensorInterrupt = digitalPinToInterrupt(sensorPin);
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

  Serial.println("done");

}

/**
 * Main program loop
 */
void loop()
{
  MQTT_connect();
  ArduinoOTA.handle(); 

  if((millis() - oldTimeTemp) > 10000)
  { 
    oldTimeTemp = millis();
    readTemperatures();

  }
   
  if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    timeClient.update();
    
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
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;

    unsigned int frac;
//
//    StaticJsonBuffer<300> jsonBuffer;
//    JsonObject& root = jsonBuffer.createObject();

//    root["flowMainR"] = flowRate;
//    root["flowMainmL"] = flowMilliLitres;
//    root["time"] = timeClient.getFormattedTime();
   
  #ifdef _DEBUG
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print(".");             // Print the decimal point
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    Serial.print("L/min");
    // Print the number of litres flowed in this second
    Serial.print("  Current Liquid Flowing: ");             // Output separator
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");

    // Print the cumulative total of litres flowed since starting
    Serial.print("  Output Liquid Quantity: ");             // Output separator
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
  #endif      
      

    Adafruit_MQTT_Publish mqtt_water_main = Adafruit_MQTT_Publish(&mqtt, _MQTT_ROOT_WATER_MAIN_FLOW_RATE);
    if (! mqtt_water_main.publish(flowRate)) {
      Serial.println(F("Sending MQTT Failed"));
    }

 
    LED_STATE = !LED_STATE;  
    digitalWrite(statusLed, LED_STATE);

    
    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
//    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}

void readTemperatures(){
// Serial.print(" Requesting temperatures..."); 
// temp_sensors.requestTemperatures(); // Send the command to get temperature readings 
// Serial.println("DONE"); 
///********************************************************************/
// Serial.print("Temperature is: "); 
// Serial.print(temp_sensors.getTempCByIndex(0)); // Why "byIndex"? 


    int numSensors = temp_sensors.getDeviceCount();
    temp_sensors.requestTemperatures(); // Send the command to get temperature readings 
    delay(20);
    
    for (int i=0;i<=numSensors-1;i++){
      DeviceAddress addr;
      temp_sensors.getAddress(addr, i);
      String rootPath = String(_MQTT_ROOT_BOILER_TEMPERATURE) + "/t_" + saveAddress(addr);

      char buff[rootPath.length()+1];
      rootPath.toCharArray(buff, sizeof(buff));

      Adafruit_MQTT_Publish mqtt_boiler = Adafruit_MQTT_Publish(&mqtt, buff);
     
      if (! mqtt_boiler.publish(temp_sensors.getTempCByIndex(i))) {
        Serial.println(F("Sending MQTT Failed"));
      }
      delay(50);
    }
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}


/* function to save a device address as a string*/
String saveAddress(uint8_t deviceAddress[8])
{
  String address = "";
    for (uint8_t i = 3; i < 8; i++) {
        // zero pad the address if necessary
//        if (deviceAddress[i] < 16)
//            address += 0;
        address += String(deviceAddress[i],HEX);
    }
    return address;
} //end print address function
