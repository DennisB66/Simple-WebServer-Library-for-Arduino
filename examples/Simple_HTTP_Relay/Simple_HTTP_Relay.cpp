// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Web Server Library for Arduino & ESP8266
// File       : Simple_HTTP_Relay.ino
// Purpose    : Example to demonstrate working of SimpleWebServer library
// Repository : https://github.com/DennisB66/Simple-Web-Server-Library-for-Arduino
//
// This example works for an Arduino Uno with an Ethernet Shield and a 4 slot Relay Board
// where the relays can be controlled via API calls over HTTP
//
// Test working with (replace with proper IP address if changed):
// curl -i -X GET "http://192.168.1.68"                        -> return HTTP identify
// curl -i -X GET "http://192.168.1.68/relays"                 -> show status of all relays
// curl -i -X GET "http://192.168.1.68/relays?state=on"        -> show relays with status on
// curl -i -X GET "http://192.168.1.68/relays/3"               -> show status of relay 3
// curl -i -X GET "http://192.168.1.68/relays/4"               -> show (invalid relay)
// curl -i -X PUT "http://192.168.1.68/relays?state=on"        -> switch all relays on
// curl -i -X PUT "http://192.168.1.68/relays?state=off"       -> switch all relays off
// curl -i -X PUT "http://192.168.1.68/relays/3?state=on"      -> switch relay 3 on
// curl -i -X PUT "http://192.168.1.68/relays/3?state=off"     -> switch relay 3 off
// curl -i -X PUT "http://192.168.1.68/relays/3?state=blink"   -> error (invalid value)

#include "SimpleWebServer.h"
#include "SimpleUtils.h"

#define SERVER_NAME "NetRelay-01"                           // host name
#define SERVER_PORT 80                                      // host port
#define VERBOSE_MODE

#include "MySecrets.h"                                      // Wifi Settings (change in MySecrets.h)
const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;

byte server_mac[]     = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte server_ip4[]     = { 192, 168,   1,  68 };             // lan ip (e.g. "192.168.1.68")
byte server_gateway[] = { 192, 168,   1,   1 };             // router gateway
byte server_subnet[]  = { 255, 255, 255,   0 };             // subnet mask

SimpleWebServer server( SERVER_PORT);                       // ardiuno  server

#define RELAY_COUNT  4                                      // number of available relays
#define RELAY_ANY    0                                      // status for relay = dc (don't care)
#define RELAY_ON     1                                      // status for relay = on
#define RELAY_OFF    2                                      // status for relay = off

#define CMD_ON  "on"                                        // command to switch relay on
#define CMD_OFF "off"                                       // command to switch relay off

uint8_t relayPin[] = { D2, D3, D4, D5 };                    // pin configuration for relay (e.g. relay pin 0 = arduino pin 2)
uint8_t relaySet[ RELAY_COUNT];                             // relay status (values indicated by RELAY_ON / RELAY_OFF)

void handleRelay_GET();                                     // forward declarations
void handleRelay_PUT();
void configRelay( void);
void updateRelay( int);
void updateRelay( int, int);
void relayPrint ( char*, int);
void relayPrint ( char*, int, int);

void setup() {
  BEGIN( 115200) LF;                                        // open serial communications

  PRINT( F( "# -----------------------")) LF;               // show header
	PRINT( F( "# -  Simple HTTP Relay  -")) LF;
  PRINT( F( "# -  V0.8  (DennisB66)  -")) LF;
  PRINT( F( "# -----------------------")) LF;
  PRINT( F( "#")) LF;

#if defined(ESP8266)                                        // ESP8266 = connect via WiFi
  WiFi.hostname( SERVER_NAME);                              // set host name
  WiFi.config( server_ip4, server_gateway, server_subnet);  // set fixed IP address
  WiFi.begin( ssid, pass);                                  // open WiFi connection
  while ( WiFi.status() != WL_CONNECTED) delay(500);        // wait for  connection

  LABEL( F( "# connected to "), ssid);
  LABEL( F( " / IP = "), WiFi.localIP()) LF;
#else                                                       // Arduino = connect via Ethernet
//Ethernet.hostName( SERVER_NAME);                          // not supported (yet)
  ETHERNET_RESET( 11U);                                     // Leonardo ETH reset
  Ethernet.begin( server_mac, server_ip4, server_gateway, server_subnet);
                                                            // open ethernet connection
  LABEL( F( "# connected to "), Ethernet.localIP()) LF;
#endif

  server.begin();                                           // starting webserver
  server.handleOn( handleRelay_GET, "relays", HTTP_GET);    // set function for "/relays"
  server.handleOn( handleRelay_PUT, "relays", HTTP_PUT);    // set function for "/relays"
  configRelay();                                            // prepare relays (defauls = all off)

  PRINT( F( "# ready for HTTP requests")) LF;
  PRINT( F( "#")) LF;
}

void loop() {
  server.handle();                                          // handle each new request
}

// handle GET on "relay" commands
void handleRelay_GET()
{                                                           // check on path / args boundaries
  if (( server.pathCount() <  1) || ( server.pathCount() > 2)) return;
  if (( server.argsCount() <  0) || ( server.argsCount() > 1)) return;
  if (( server.pathCount() == 2) && ( server.argsCount() > 0)) return;

  char        reply[256]; strClr( reply);                   // reply buffer
  const char* index      = server.path( 1);                 // relay index string
  uint8_t     relay      = index ? atoi( index) : 0;        // relay index value
  uint8_t     state      = RELAY_ANY;                       // relay state

  state = server.arg( "state", CMD_ON ) ? RELAY_ON : state; // get requested state = on
  state = server.arg( "state", CMD_OFF) ? RELAY_OFF: state; // get requested state = off

  if ( index) {                                             // if specific relay is speciified
    relayPrint( reply, relay, RELAY_ANY);                   // show state for specific relay
    server.respond( returnCode = 200, "text/plain", reply); // send OK + reply to client
  } else {
    relayPrint( reply, state);                              // show relays with specific state
    server.respond( returnCode = 200, "text/plain", reply); // respond result to client
  }
}

// handle PUT on "relay" commands
void handleRelay_PUT()
{                                                           // check on path / args boundaries
  if (( server.pathCount() < 1) || ( server.pathCount() > 2)) return;
  if (( server.argsCount() < 1) || ( server.argsCount() > 1)) return;

  const char* index = server.path( 1);                      // relay index string
  uint8_t     relay = index ? atoi( index) : 0;             // relay index value
  uint8_t     state = RELAY_ANY;                            // relay state

  state = server.arg( "state", CMD_ON ) ? RELAY_ON : state; // get requested state = on
  state = server.arg( "state", CMD_OFF) ? RELAY_OFF: state; // get requested state = off

  if ( index) {                                             // if specific relay is speciified
    updateRelay( relay, state);                             // set state for specfic relay
    server.respond( returnCode = 200);                      // send OK to client
  } else {
    updateRelay( state);                                    // set state for all relays
    server.respond( returnCode = 200);                      // send OK to client
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
  for( int i = 0; i < RELAY_COUNT; i++) {                   // for all relays
    updateRelay( i, state);                                 // set state of relay i
  }
}

// set state of relay i
void updateRelay( int i, int state)
{
  if (( i >= 0) && ( i < RELAY_COUNT)) {                    // check if relay is valid
    digitalWrite( relayPin[ i], relaySet[ i] = state);      // set state of relay i
  }
}

// print relay state for all relays (write buffer, requested state)
void relayPrint( char* line, int state)
{
  for( int i = 0; i < RELAY_COUNT; i++) {                   // for all relays
    if (( state == RELAY_ANY) || ( state == relaySet[ i])) {
      relayPrint( line, i, state);                          // write state of relay i
    }
  }
}

// print relay state for single relay (write buffer, relay i, requested state)
void relayPrint( char* line, int i, int state)
{
  if (( i >= 0) && ( i < RELAY_COUNT) &&                    // check if relay is valid
     (( state == RELAY_ANY) || ( state == relaySet[i]))) {  // check if state is valid
    strcat( line, "# relay "); strcat( line, dec( i , 2));  // write state of relay i
    strcat( line, " on pin "); strcat( line, dec( relayPin[i], 2));
    strcat( line, " = "     ); strcat( line, relaySet[ i] == RELAY_ON ? CMD_ON : CMD_OFF);
    strcat( line, "\r\n");
  } else {
    strcat( line, "# relay "    ); strcat( line, dec(i , 2));
    strcat( line, " not defined"); strcat( line, "\r\n");   // undefined relay
  }
}
