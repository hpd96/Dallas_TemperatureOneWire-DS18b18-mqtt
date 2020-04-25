

/*
rotex temp -> mqtt

hpd
*/


#define REQUIRESNEW false
#define REQUIRESALARMS false

#include <OneWire.h>
#include <DallasTemperature.h>
 

#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x00, 0x10, 0x83, 0xBE, 0xEE, 0xEF };
//IPAddress server(192,168,42,1);  // numeric IP for Google (no DNS)
 char server[] = "conil.fritz.box";    // name address for Google (using DNS)

//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)

IPAddress ip(192, 168, 42, 247);


// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

EthernetClient client;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // start serial port
  Serial.begin(19200);
  while(!Serial) { ; }
  Serial.println("    Dallas Temperature IC Control Library Demo");

  // Start up the library
    // Start up the library
  sensors.begin();
  delay(250);
  showInfo();

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
else
{
Serial.print("new IP is :");   Serial.print(""); Serial.println();
}

    // give the Ethernet shield a second to initialize:
  delay(1000);
  
}
 
void showInfo(void)
{
  uint8_t i;
  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  DeviceAddress insideThermometer, outsideThermometer;
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  //  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1"); 
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();

  printAddress(insideThermometer);

}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.println();
}


void loop(void)
{
  int i;

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("\n Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");


for(i=0; i<sensors.getDeviceCount(); i++)
{
  Serial.print("Temperature for Device ");
  Serial.print( i+1, DEC );
  Serial.print(" is: ");
  Serial.print(sensors.getTempCByIndex(i));
}  
  Serial.print("\n");

  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.print("GET /q?");
    for(i=0; i<sensors.getDeviceCount(); i++)
    {
      if ( i>0 ) { client.print("&"); }
      client.print("temp");
      client.print( i+1, DEC );
      client.print("=");
      client.print(sensors.getTempCByIndex(i));
    }
    client.println(" HTTP/1.1");
    client.println("Host: hpbeef.fritz.box");
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }


  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

    
  delay(10000);


}


