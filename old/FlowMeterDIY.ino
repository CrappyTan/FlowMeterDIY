
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


#define ONE_WIRE_BUS D5

#define _MQTT_ROOT_BOILER_ROOT "home/utility/boiler"


// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature temp_sensors(&oneWire);

#define _NTP_SERVER "0.uk.pool.ntp.org"
#define _MQTT_HOST "mqtt.home"
#define _MQTT_HOST_PORT 1883

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, _NTP_SERVER);

/*
Liquid flow rate sensor -DIYhacking.com Arvind Sanjeev

Measure the liquid/water flow rate using this code. 
Connect Vcc and Gnd of sensor to arduino, and the 
signal line to arduino digital pin 2.
 
 */

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

WiFiClient espClient;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&espClient, _MQTT_HOST, _MQTT_HOST_PORT);

//Adafruit_MQTT_Publish mqtt_water_main= Adafruit_MQTT_Publish(&mqtt, _MQTT_ROOT_WATER_MAIN_1);
//Adafruit_MQTT_Publish mqtt_boiler = Adafruit_MQTT_Publish(&mqtt, _MQTT_ROOT_BOILER_1);


void setup()
{
  
  // Initialize a serial connection for reporting values to the host
  Serial.begin(115200);

  temp_sensors.begin(); 
  Serial.print("Temperature Sensors found: ");
  Serial.println(temp_sensors.getDeviceCount(), DEC);
  
  timeClient.begin();

  
 //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("WaterMeterAP", "Qwerty123");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

       
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

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  sensorInterrupt = digitalPinToInterrupt(sensorPin);
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}


/**
 * Main program loop
 */
void loop()
{
  // Ensure the connection to the MQTT server is alive (this will make the first
    // connection and automatically reconnect when disconnected).  See the MQTT_connect
    // function definition further below.

    timeClient.update();

    
    MQTT_connect();


    
    sendTemps(mqtt);
//    sendFlow();
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


/*
Insterrupt Service Routine
 */
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
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


void sendTemps(Adafruit_MQTT_Client mqtt){

    timeClient.update();

    int numSensors = temp_sensors.getDeviceCount();
    temp_sensors.requestTemperatures(); // Send the command to get temperature readings 
    delay(20);
    
    for (int i=0;i<=numSensors-1;i++){
      DeviceAddress addr;
      temp_sensors.getAddress(addr, i);
      String c = "t_" + saveAddress(addr);
      double temp = temp_sensors.getTempCByIndex(i);

      String path = _MQTT_ROOT_BOILER_ROOT;// + "/temperature/" + c;
      
      Adafruit_MQTT_Publish mqtt_send = Adafruit_MQTT_Publish(&mqtt, path);

      if (! mqtt_send.publish(temp)) {
        Serial.println(F("Sending MQTT Failed"));
        digitalWrite(statusLed, LOW);
        delay(100);
        digitalWrite(statusLed, HIGH);
      }
      
      delay(20);
    }
}

