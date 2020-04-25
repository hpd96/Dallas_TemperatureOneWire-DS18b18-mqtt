#include <PubSubClient.h>


#include<stdlib.h>

/*
rotex temp -> mqtt

hpd
*/


#define REQUIRESNEW false
#define REQUIRESALARMS false

#define PLAUSI_MIN 8.0
#define PLAUSI_MAX 100.0

#define MQTT_ITEM "rotex/fbh/tempX"

#define sensor1_addr "28D0241E0700006A"
#define sensor2_addr "2843081D070000D3"

#include <OneWire.h>
#include <DallasTemperature.h>
 

#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x00, 0x10, 0x83, 0xBE, 0xEE, 0xEF };

char server[] = "hwr-pi.fritz.box";    // name address for MQTT broker (using DNS)

IPAddress ip(192, 168, 42, 247);


// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("rotex/fbh/info","hello world");
      // ... and resubscribe
      client.subscribe("rotex/fbh/in");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // start serial port
  Serial.begin(57600);
  while(!Serial) { ; }
  Serial.println("    Dallas Temperature One Wire DS18b18 -> MQTT");

  client.setServer(server, 1883);
  client.setCallback(callback);
  
  // Start up the library
  sensors.begin();
  delay(250);
  showInfo();

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  else
  {
    Serial.print("DHCP IP is: "); Serial.println(Ethernet.localIP());
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
  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1"); 
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();

  printAddress(insideThermometer);
  printAddress(outsideThermometer);

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
  char sBUFFER[20];
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("\n Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

    char mqtt_item[] = MQTT_ITEM;
    
    for(i=0; i<sensors.getDeviceCount(); i++)
    {
      Serial.print("Temperature for Device ");
      Serial.print( i+1, DEC );
      Serial.print(" is: ");
      Serial.print(sensors.getTempCByIndex(i));
      Serial.print("\n");
      if ( (sensors.getTempCByIndex(i) >= PLAUSI_MIN) && (sensors.getTempCByIndex(i) <= PLAUSI_MAX) )
      {
        mqtt_item[ strlen(mqtt_item)-1 ]= i + '0' + 1; // int to char
        client.publish( mqtt_item, dtostrf(sensors.getTempCByIndex(i), 3, 1, sBUFFER) );
      }
      else
      { Serial.print( i+1, DEC); Serial.print(" out of range!"); }
    }
    delay(30000); // wait 30sec
}


