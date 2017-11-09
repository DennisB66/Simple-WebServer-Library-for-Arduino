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

#define NO_DEBUG_MODE

#define SERVER_NAME "NetRelay-01"                           // host name
#define SERVER_PORT 80                                      // host port

const char* ssid     = "xxxxxxxx";                          // repce with proper ssid
const char* password = "xxxxxxxx";                          // repce with proper password

byte server_mac[]     = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // mac address
byte server_ip4[]     = { 192, 168, 1, 60 };                // lan ip (e.g. "192.168.1.60")
byte server_gateway[] = { 192, 168, 1, 1 };                 // router gateway
byte server_subnet[]  = { 255, 255, 255, 0 };               // subnet mask

SimpleWebServer server( SERVER_PORT);                       // ardiuno  server

#define RELAY_COUNT  4                                      // number of available relays
#define RELAY_ON     0                                      // status for relay = on
#define RELAY_OFF    1                                      // status for relay = off

#define CMD_ON  "on"                                        // command to switch relay on
#define CMD_OFF "off"                                       // command to switch relay off

byte relayPin[] = { D2, D3, D4, D5 };                       // pin configuration for relay (e.g. relay pin 0 = arduino pin 2)
byte relaySet[ RELAY_COUNT];                                // relay status (values indicated by RELAY_ON / RELAY_OFF)

void handleRelay_GET();
void handleRelay_PUT();
void configRelay( void);
int  updateRelay( const char*);
int  updateRelay( int, const char*);
int  relayPrint ( const char*, bool);
int  relayPrint ( int  , bool);

void setup() {
  BEGIN( 9600) LF;                                          // open serial communications

  PRINT( F( "===================")) LF;                     // show header
  PRINT( F( "-  Network Relay  -")) LF;
  PRINT( F( "===================")) LF;

#if defined(ESP8266)                                        // ESP8266 = connect via WiFi
  WiFi.hostname( SERVER_NAME);                              // set host name
  WiFi.config( server_ip4, server_gateway, server_subnet);  // set fixed IP address
  WiFi.begin( ssid, password);                              // open WiFi connection
  while ( WiFi.status() != WL_CONNECTED) delay(500);        // wait for  connection

  LABEL( F( "# WiFi connected to "), ssid);
  LABEL( F( " / IP = "), WiFi.localIP()) LF;
  PRINT( F( "#")) LF;
#else                                                       // Arduino = connect via Ethernet
//Ethernet.hostName( SERVER_NAME);                          // not supported (yet)
  ETHERNET_RESET( 11U);                                     // Leonardo ETH reset
  Ethernet.begin( server_mac, server_ip4, server_gateway, server_subnet);
                                                            // open ethernet connection
  LABEL( F( "# Ethernet connected to "), Ethernet.localIP()) LF;
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
  if (( server.pathCount() > 2) || ( server.argsCount() > 1)) return;

  char* idx  = server.path( 1);                             // get relay id
  char* cmd  = server.arg( "state");                        // get relay command

  #ifdef DEBUG_MODE
  VALUE( F( "# dev"   ), F( "relay"));
  VALUE( F( ", idx"   ), idx ? idx : "NULL");
  VALUE( F( ", state" ), cmd ? cmd : "NULL") LF;
  #endif

  if ( idx) {                                               // show status for relay = idx
    errorCode = ( relayPrint( atoi( idx), false) == 0) ? 200 : 400;
  } else {                                                  // show arrays with status = cmd
    errorCode = ( relayPrint(       cmd , false) == 0) ? 200 : 400;
  }
}

// handle PUT on "relay" commands
void handleRelay_PUT()
{
  if (( server.pathCount() > 2) || ( server.argsCount() > 1)) return;

  char* idx  = server.path( 1);                             // get relay id
  char* cmd  = server.arg( "state");                        // get relay command

  if ( cmd == NULL) return;

  #ifdef DEBUG_MODE
  VALUE( F( "# dev"   ), F( "relay"));
  VALUE( F( ", idx"   ), idx ? idx : "NULL");
  VALUE( F( ", state" ), cmd ? cmd : "NULL") LF;
  #endif

  if ( idx) {                                               // set state = cmd for relay = idx
    errorCode = ( updateRelay( atoi( idx),  cmd) == 0) ? 200 : 400;
    relayPrint( atoi( idx), true );                         // return status for relay = idx
  } else {                                                  // set state = cmd for relay = idx
    errorCode = ( updateRelay(              cmd) == 0) ? 200 : 400;
    relayPrint(       cmd , true );                         // show arrays with status = cmd
  }
}

// initialize pins with relays inactive
void configRelay( void)
{
  for (int i = 0; i < RELAY_COUNT; i++) {
     updateRelay( i, CMD_OFF);                              // 1st: set pin for relay i to low
                                                            // (to avoid an immediate switch on)
     pinMode( relayPin[ i], OUTPUT);                        // 2nd: set pin for relay i as output
  }
}

// set all relays with value cmd
int updateRelay( const char* cmd)
{
  int error = 0;

  for( int i = 0; i < RELAY_COUNT; i++) {
    error = updateRelay( i, cmd) ? 102 : error;
  }

  return error;
}

// set relay idx with value cmd
int updateRelay( int idx, const char* cmd)
{
  bool valid = false;

  if (( idx >= 0) & ( idx < RELAY_COUNT)) {                 // check if relay exists
    if ( strCmp( cmd, CMD_ON )) { valid = true; relaySet[ idx] = RELAY_ON ; }
    if ( strCmp( cmd, CMD_OFF)) { valid = true; relaySet[ idx] = RELAY_OFF; }

    if ( valid) {
      digitalWrite( relayPin[ idx], relaySet[ idx]);        // write cmd to relay idx
    } else {
      ERROR( F( "cmd not defined for relay"));
      return 102;                                           // invalid command
    }
  } else {
    return 101;                                             // invalid relay
  }

  return 0;
}

// print status for all relays (triggered = update)
int relayPrint( const char* cmd, bool triggered)
{
  for( int i = 0; i < RELAY_COUNT; i++) {
    if (( cmd == NULL) ||
       ( strCmp( cmd, CMD_ON ) && ( relaySet[ i] == RELAY_ON )) ||
       ( strCmp( cmd, CMD_OFF) && ( relaySet[ i] == RELAY_OFF))) {
      relayPrint( i, triggered);
    }
  }

  return 0;
}

// print status for relays idx (triggered = update)
int relayPrint( int idx, bool triggered)
{
  if (( idx >= 0) & ( idx < RELAY_COUNT)) {
    LABEL( F( "# relay "), idx);
    LABEL( F( " on pin "), relayPin[ idx]);
    PRINT( triggered ? " switched " : " = ");
    PRINT( relaySet[ idx] == RELAY_ON ? CMD_ON : CMD_OFF) LF;

    return 0;
  } else {
    ERROR( F( "relay "));
    LABEL( idx, F( " not defined")) LF;
    return 101;
  }
}
