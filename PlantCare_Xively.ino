/*

 Created on 22 Nov 2014
 by Carmelito Andrade
 for Monitoring the health of the house plant using Intel Edison

  Feel free to modify and use 
 */

#include <SPI.h>
#include <WiFi.h>
#include <stdlib.h>

#define APIKEY         "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" // replace your xively api key here
#define FEEDID         XXXXXXXXX                    // replace your feed ID
#define USERAGENT      "PlantTracker_using_IntelEdison"     // user agent is the project name

char ssid[] = "WifirouterName";      //  your network SSID (name) 
char pass[] = "WifirouterPassword";   // your network password

int status = WL_IDLE_STATUS;
//char tempString[10] ;
// initialize the library instance:
WiFiClient client;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(216,52,233,121);      // numeric IP for api.pachube.com
char server[] = "api.xively.com";   // name address for pachube API

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 20*1000;  //delay between updates to pachube.com

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 

  String fv = WiFi.firmwareVersion();
  if( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");
  
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  } 
  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop() {
  // read the value of the Light Sensor
  int lightSensorReading = analogRead(A2);   
  // convert the data to a String to send it:

  String dataString = "Light_Value_from_Photocell,";
  dataString += lightSensorReading;

 // Read values from the Soil moisture sensor
  int SoilMosture = analogRead(A1);
  dataString += "\nHygro_Soil_Sensor,";
  dataString += SoilMosture;
  
  //Read temperature value and convert to Faranite
  int temperature = analogRead(A0);
  
   float voltage = temperature * 5.0;
   voltage /= 1024.0; 
   float temperatureC = (voltage - 0.5) * 100 ; //Temperature in Centigrade
  // now convert to Fahrenheit
   float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
   //convert the temperatureF from float to string
    //dtostrf(temperatureF, 8, 2, tempString);
    String stringVal ="";
   stringVal+=String(int(temperatureF))+ "."+String(getDecimal(temperatureF)); //Refer to the decimal value below, workaround for dtostrf
   dataString += "\nTemperature,";
   dataString += stringVal;

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data: 
  if(!client.connected() || (millis() - lastConnectionTime > postingInterval)) {
    sendData(dataString);
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

// this method makes a HTTP connection to the server:
void sendData(String thisData) {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.print("PUT /v2/feeds/");
    client.print(FEEDID);
    client.println(".csv HTTP/1.1");
    client.println("Host: api.xively.com");
    client.print("X-ApiKey: ");
    client.println(APIKEY);
    client.print("User-Agent: ");
    client.println(USERAGENT);
    client.print("Content-Length: ");
    client.println(thisData.length());

    // last pieces of the HTTP PUT request:
    client.println("Content-Type: text/csv");
    client.println("Connection: close");
    client.println();

    // here's the actual content of the PUT request:
    client.println(thisData);
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  // note the time that the connection was made or attempted:
  lastConnectionTime = millis();
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

//Create to workaround the error 'dtostrf' was not declared in this scope
long getDecimal(float val)
{
 int intPart = int(val);
 long decPart = 1000*(val-intPart); 
 if(decPart>0)return(decPart);           
 else if(decPart<0)return((-1)*decPart); 
 else if(decPart=0)return(00);         
}
