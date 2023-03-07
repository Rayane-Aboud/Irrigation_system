#include "thingProperties.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"

/*
  float humidity;
  float temperature;
  float water_level;
  float soil_moisture;

  are automatically generated by initProperties()
*/



/*temperature and humidity */
#define DHTPIN D9     
#define DHTTYPE DHT11   // DHT 11


/*water level*/ 
#define TRIGPIN D10
#define ECHOPIN D11
#define HEIGHT_RESERVE 10

/*soil moisture*/
#define SM A0//define soil moisture and motor

/*water pump*/
#define WP D8









/*
  constants
*/
//
String server = "http://maker.ifttt.com";
String eventName = "signal";

String IFTTT_Key = xxxx-xxxx-xxxx-xxxx;
String IFTTTUrl = "http://maker.ifttt.com/trigger/signal/with/key/"+IFTTT_Key;



//variables for temperature and humidity sensor
DHT dht(DHTPIN, DHTTYPE);


//variables for ultra sonic sensor
float duration, cm;











void setup() {
  // Initialize serial console
  Serial.begin(9600);
  delay(1500);

  // init arduino cloud variables and environnement 
  initProperties();

  // Connect the arduino chip to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*init temperature and humidity pin*/
  pinMode(DHTPIN, INPUT);
  dht.begin();
  
  
  /*init ultra sonic sensor pins*/ 
  pinMode(TRIGPIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHOPIN, INPUT); // Sets the echoPin as an Input
  
  
  /*init soil moisture pin*/
  pinMode(SM,INPUT);
  
  /*init water pump pin*/
  pinMode(WP,OUTPUT);
  
  
  
  /*
     some developer stuff
  */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}




//function to send data to the data sheet
void sendDataToSheet(void)
{
  //link to the IFTTT server name + parameters to send to sheet
  String url = server + "/trigger/" + eventName + "/with/key/" + IFTTT_Key + "?value1=" + String((float)temperature) + "&value2=" + String((float)humidity) + "&value3=" + String((float)water_level);
  Serial.println();
  Serial.print(" URL : ");
  Serial.println(url);
  
  //Start to send data to IFTTT using an http GET request
  HTTPClient http;
  Serial.print(" HTTP request status : ");
  Serial.print("[HTTP] begin...\n");
  //send the http request to the IFTTT server
  http.begin(url); 
  
  Serial.print("[HTTP] GET...\n");
  // waiting for the response from the IFTTT server
  int httpCode = http.GET();
  
  if (httpCode > 0) {//case request successful
  
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else { //case error
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();//close the request
}






void loop() {
  //updating the dashboard
  ArduinoCloud.update();
  
  // Wait a few seconds between measurements.
  delay(2000);
  Serial.println("---------------------------------------------------------------------------");
  
  
  /*mesuring temperature and humidity*/
  // Reading temperature or humidity takes about 250 milliseconds!
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperature = dht.readTemperature();

  //writing mesurements to the serial console 
  Serial.print(F("  ->Humidity : "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature);
  Serial.println(F("C \n"));






  /*mesuring water level using ultra sonic sensor*/
  
  //mechanism:
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  
  
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  duration = pulseIn(ECHOPIN, HIGH);
  // Convert the time into a distance
  water_level = (duration / 2) / 29.1;   // Divide by 29.1 or multiply by 0.0343
  
  
  //writing mesurements to the serial console 
  Serial.print(F("  ->Water level : "));
  Serial.print("");
  Serial.print(water_level);
  Serial.print("cm");
  Serial.println();



  /*mesuring soil moisture*/
  /*
    analogRead(SM): return a value between 0 and 1023 
    the more the soil is moist lower the return value is 
    we use the formula below to convert it into a percentage
  */
  soil_moisture = 100.0 - (float)(analogRead(SM)) / 1024.0 * 100.0;
  
  //writing mesurements to the serial console
  Serial.print(F("  -> Soil moisture : "));
  Serial.print(soil_moisture);
  Serial.println("% \n");
  
  /*controling the water pump based on the soil moisture*/
  if(soil_moisture<50)
    digitalWrite(WP,HIGH);
  else{
    digitalWrite(WP,LOW);
  }
  
  //sending measurments to datasheet
  sendDataToSheet();

  //Serial.println("---------------------------------------------------------------------------");

}
