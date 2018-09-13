// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Web Server Library for Arduino
// File       : Simple_HTTP_Relay.ino
// Purpose    : Example to demonstrate working of SimpleWebServer library
// Repository : https://github.com/DennisB66/Simple-Web-Server-Library-for-Arduino
//
// This example works for an Arduino Uno with an Ethernet Shield and a 4 slot Relay Board
// where the relays can be controlled via API calls over HTTP
//
// Test working with (replace with proper IP address if changed):
// curl -i -X GET "http://192.168.1.60"                        -> return HTTP identify
// curl -i -X GET "http://192.168.1.60/relays"                 -> show status of all relays
// curl -i -X GET "http://192.168.1.60/relays?state=on"        -> show relays with status on
// curl -i -X GET "http://192.168.1.60/relays/3"               -> show status of relay 3
// curl -i -X GET "http://192.168.1.60/relays/4"               -> show (invalid relay)
// curl -i -X PUT "http://192.168.1.60/relays?state=on"        -> switch all relays on
// curl -i -X PUT "http://192.168.1.60/relays?state=off"       -> switch all relays off
// curl -i -X PUT "http://192.168.1.60/relays/3?state=on"      -> switch relay 3 on
// curl -i -X PUT "http://192.168.1.60/relays/3?state=off"     -> switch relay 3 off
// curl -i -X PUT "http://192.168.1.60/relays/3?state=blink"   -> error (invalid value)

#include "SimpleWebServer.h"
#include "SimpleUtils.h"
#include "ArduinoJson.h"

#define SERVER_NAME "NetRelay-01"                           // host name
#define SERVER_PORT 80                                      // host port

const char* ssid     = "Kajtus06";                          // replace with proper ssid
const char* password = "Polska06";                          // replace with proper password

byte server_mac[]     = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // mac address
byte server_ip4[]     = { 192, 168, 1, 60 };                // lan ip (e.g. "192.168.1.60")
byte server_gateway[] = { 192, 168, 1, 1 };                 // router gateway
byte server_subnet[]  = { 255, 255, 255, 0 };               // subnet mask

SimpleWebServer server( SERVER_PORT);                       // ardiuno  server

#define RELAY_COUNT  4                                      // number of available relays
#define RELAY_ANY    0
#define RELAY_ON     1                                      // status for relay = on
#define RELAY_OFF    2                                      // status for relay = off

#define CMD_ON  "on"                                        // command to switch relay on
#define CMD_OFF "off"                                       // command to switch relay off

uint8_t relayPin[] = { D2, D3, D4, D5 };                    // pin configuration for relay (e.g. relay pin 0 = arduino pin 2)
uint8_t relaySet[ RELAY_COUNT];                             // relay status (values indicated by RELAY_ON / RELAY_OFF)

void handleRelay_GET();
void handleRelay_PUT();
void configRelay( void);
void updateRelay( int);
void updateRelay( int, int);
bool relay2Json ( char*, unsigned int, int);
bool relay2Json ( char*, unsigned int, int, int);

void setup() {
  BEGIN( 115200) LF;                                          // open serial communications

  PRINT( F( "===================")) LF;                     // show header
  PRINT( F( "-  Network Relay  -")) LF;
  PRINT( F( "===================")) LF;

#if defined(ESP8266)                                        // ESP8266 = connect via WiFi
  WiFi.hostname( SERVER_NAME);                              // set host name
  WiFi.config( server_ip4, server_gateway, server_subnet);  // set fixed IP address
  WiFi.begin( ssid, password);                              // open WiFi connection
  while ( WiFi.status() != WL_CONNECTED) delay(500);        // wait for  connection

  LABEL( F( "# connected to "), ssid);
  LABEL( F( " / IP = "), WiFi.localIP()) LF;
  PRINT( F( "#")) LF;
#else                                                       // Arduino = connect via Ethernet
//Ethernet.hostName( SERVER_NAME);                          // not supported (yet)
  ETHERNET_RESET( 11U);                                     // Leonardo ETH reset
  Ethernet.begin( server_mac, server_ip4, server_gateway, server_subnet);
                                                            // open ethernet connection
  LABEL( F( "# connected to "), Ethernet.localIP()) LF;
  PRINT( F( "#")) LF;
#endif

  server.begin();                                           // starting webserver
  server.handleOn( handleRelay_GET, "relays", HTTP_GET);    // set function for "/relays"
  server.handleOn( handleRelay_PUT, "relays", HTTP_PUT);    // set function for "/relays"

  PRINT( F( "# initializing relay")) LF;
  configRelay();                                            // prepare relays (defauls = all off)
  PRINT( F( "# ready for requests")) LF;
  PRINT( F( "#")) LF;
}

void loop() {
  server.handle();
  delay( 100);                                              // check every second for a new request
}

// handle GET on "relay" commands
void handleRelay_GET()
{
  if (( server.pathCount() <  1) || ( server.pathCount() > 2)) return;
  if (( server.argsCount() <  0) || ( server.argsCount() > 1)) return;
  if (( server.pathCount() == 2) && ( server.argsCount() > 0)) return;

  char response[256]; strClr( response);
  int  relay = server.path( 1) ? atoi( server.path( 1)) : -1;
  int  state = RELAY_ANY;

  state = server.arg( "state", CMD_ON  ) ? RELAY_ON  : state;
  state = server.arg( "state", CMD_OFF ) ? RELAY_OFF : state;

  if ( server.path( 1)) {                                   // show status for relay = idx
    relay2Json( response, sizeof( response), relay, state);
    server.response( returnCode = 200, "text/plain", response);
  } else {                                                  // show arrays with status = cmd
    relay2Json( response, sizeof( response),        state);
    server.response( returnCode = 200, "text/plain", response);
  }
}

// handle PUT on "relay" commands
void handleRelay_PUT()
{
  if (( server.pathCount() < 1) || ( server.pathCount() > 2)) return;
  if (( server.argsCount() < 1) || ( server.argsCount() > 1)) return;

  int relay = server.path( 1) ? atoi( server.path( 1)) : -1;
  int state = RELAY_ANY;

  state = server.arg( "state", CMD_ON  ) ? RELAY_ON  : state;
  state = server.arg( "state", CMD_OFF ) ? RELAY_OFF : state;

  if ( server.path( 1)) {                                   // set state = cmd for relay = idx
    updateRelay( relay, state);
    server.response( returnCode = 200);
  } else {                                                  // set state = cmd for relay = idx
    updateRelay( state);
     returnCode = 200;
    server.response( returnCode = 200);
  }
}

// initialize pins with relays inactive
void configRelay()
{
  for( int i = 0; i < RELAY_COUNT; i++) {                   // to avoid an immediate switch on:
     digitalWrite( relayPin[ i], LOW   );                   // 1st: set pin for relay i to low
     pinMode     ( relayPin[ i], OUTPUT);                   // 2nd: set pin for relay i as output

     relaySet[i] = RELAY_OFF;                               // set initial relay state
  }
}

// set all relays with value cmd
void updateRelay( int state)
{
  for( int i = 0; i < RELAY_COUNT; i++) {
    updateRelay( i, state);
  }
}

// set relay idx with value cmd
void updateRelay( int i, int state)
{
  if (( i >= 0) && ( i < RELAY_COUNT)) {                       // check if relay exists
    digitalWrite( relayPin[ i], relaySet[ i] = state);        // write cmd to relay idx
  }
}


// print status for all relays
bool relay2Json( char* content, unsigned int size, int state)
{
  StaticJsonBuffer<200> jsonBuffer;                         // create buffer
  JsonArray& root     = jsonBuffer.createArray();           // create array

  for( int i = 0; i < RELAY_COUNT; i++) {
    if (( state == RELAY_ANY) || ( state == relaySet[i])) {
      JsonObject& item = root.createNestedObject();         // create object
      item[ "relay"] = i;                                   // write object
      item[ "state"] = ( relaySet[ i] == RELAY_ON ? CMD_ON : CMD_OFF);
    }
  }

  root.printTo( content, size);                             // write to char buffer

  return true;                                              // success = json created
}

// print status for relay item i
bool relay2Json( char* content, unsigned int size, int i, int state)
{
  if (( i >= 0) & ( i < RELAY_COUNT)) {                     // check if relay exists
    StaticJsonBuffer<200> jsonBuffer;                       // create buffer
    JsonArray&  root    = jsonBuffer.createArray();         // create array
    JsonObject& item    = root.createNestedObject();        // create object

    item[ "relay"] = i;                                     // write object
    item[ "state"] = ( relaySet[ i] == RELAY_ON ? CMD_ON : CMD_OFF);

    root.printTo( content, size);                           // write to char buffer
  } else {
    strClr( content);                                       // clear char buffer
  }
}
