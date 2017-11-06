// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Web Server Library for Arduino
// File       : Simple_HTTP_Relay.ino
// Purpose    : Example to demonstrate working of SimpleWebServer library
// Repository : https://github.com/DennisB66/Simple-WebServer-Library-for-Arduino
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

#define NO_DEBUG_MODE

#define SERVER_NAME "NetRelay-01"                           // host name
#define SERVER_PORT 80                                      // host port

const char* ssid     = "xxxxxxxx";
const char* password = "xxxxxxxx";

byte server_mac[]     = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte server_ip4[]     = { 192, 168, 1, 60 };                // lan ip (e.g. "192.168.1.178")
byte server_gateway[] = { 192, 168, 1, 1 };                 // router gateway
byte server_subnet[]  = { 255, 255, 255, 0 };               // subnet mask

SimpleWebServer myServer( SERVER_PORT);                     // ardiuno  server

#define RELAY_COUNT  4                                      // number of available relays
#define RELAY_ON     0                                      // status for relay = on
#define RELAY_OFF    1                                      // status for relay = off

#define CMD_ON  "on"                                        // command to switch relay on
#define CMD_OFF "off"                                       // command to switch relay off

byte relayPin[] = { 2, 3, 6, 7 };                           // pin configuration for relay (e.g. relay pin 0 = arduino pin 2)
byte relaySet[ RELAY_COUNT];                                // relay status (values indicated by RELAY_ON / RELAY_OFF)

void handleRelay_GET();
void handleRelay_PUT();
void ConfigRelay( void);
int  UpdateRelay( const char*);
int  UpdateRelay( int, const char*);
int  RelayPrint ( const char*, bool);
int  RelayPrint ( int  , bool);
bool Relay2Json ( const char*, const char*, unsigned int);
bool Relay2Json ( int, const char*, unsigned int);

void setup() {
  BEGIN( 9600) LF;                                          // open serial communications

  PRINT( F( "==============================================")) LF;
  PRINT( F( "-  Arduino Network Relay with JSON response  -")) LF;
  PRINT( F( "==============================================")) LF;

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
  delay( 100);
}

// handle GET on "relay" commands
void handleRelay_GET()
{
  if (( server.pathCount() > 2) || ( server.argsCount() > 1)) return;

  char  content[ 256]  = "";                                // char buffer for json
  bool  json           = false;

  char* idx  = server.path( 1);                             // get relay id
  char* cmd  = server.arg( "state");                        // get relay command

  #ifdef DEBUG_MODE
  VALUE( F( "# dev"   ), F( "relay"));
  VALUE( F( ", idx"   ), idx ? idx : "NULL");
  VALUE( F( ", state" ), cmd ? cmd : "NULL") LF;
  #endif

  if ( idx) {
    code = ( RelayPrint( atoi( idx), false) == 0) ? 200 : 400;
                                                            // show status for relay = idx
    json =   Relay2Json( atoi( idx), content, sizeof( content));
  } else {
    code = ( RelayPrint(       cmd,  false) == 0) ? 200 : 400;
                                                            // show arrays with status = cmd
    json =   Relay2Json(       cmd, content, sizeof( content));
  }

  if ( json) {
    server.response( code, "application/json", content);    // return response (headers + code + json)
  } else {
    server.response( code);                                 // return response (headers + code)
  }

}

// handle PUT on "relay" commands
void handleRelay_PUT()
{
  if (( server.pathCount() > 2) || ( server.argsCount() > 1)) return;

  char  content[ 256]  = "";               // char buffer for json
  bool  json           = false;

  char* idx  = server.path( 1);                             // get relay id
  char* cmd  = server.arg( "state");                        // get relay command

  if ( cmd == NULL) return;

  #ifdef DEBUG_MODE
  VALUE( F( "# dev"   ), F( "relay"));
  VALUE( F( ", idx"   ), idx ? idx : "NULL");
  VALUE( F( ", state" ), cmd ? cmd : "NULL") LF;
  #endif

  if ( idx) {
    UpdateRelay( atoi( idx), cmd);                            // set state = cmd for relay = idx

    code = ( RelayPrint( atoi( idx), cmd)  == 0) ? 200 : 400; // return status for relay = idx
    json =   Relay2Json( atoi( idx), content, sizeof( content));
  } else {
    UpdateRelay( cmd);                                         // set state = cmd for all relays

    code = ( RelayPrint( cmd, false)        == 0) ? 200 : 400; // return status for all relays
    json =   Relay2Json( cmd, content, sizeof( content));
  }

  if ( json) {
    server.response( code, "application/json", content);         // return response (headers + code + json)
  } else {
    server.response( code);                                      // return response (headers + code)
  }

}

// initialize pins with relays inactive
void ConfigRelay( void)
{
  for (int i = 0; i < RELAY_COUNT; i++) {
     UpdateRelay( i, CMD_OFF);                                       // 1st: set pin for relay i to low
                                                                     // (to avoid an immediate switch on)
     pinMode( relayPin[ i], OUTPUT);                                 // 2nd: set pin for relay i as output
  }
}

// set all relays with value cmd
int UpdateRelay( const char* cmd)
{
  int error = 0;

  for( int i = 0; i < RELAY_COUNT; i++) {
    error = UpdateRelay( i, cmd) ? 102 : error;
  }

  return error;
}

// set relay idx with value cmd
int UpdateRelay( int idx, const char* cmd)
{
  bool valid = false;

  if (( idx >= 0) & ( idx < RELAY_COUNT)) {                          // check if relay exists
    if ( strCmp( cmd, CMD_ON )) { valid = true; relaySet[ idx] = RELAY_ON;  }
    if ( strCmp( cmd, CMD_OFF)) { valid = true; relaySet[ idx] = RELAY_OFF; }

    if ( valid) {
      digitalWrite( relayPin[ idx], relaySet[ idx]);                 // write cmd to relay idx
    } else {
      ATTR( Serial, "# error: cmd not defined for relay ", idx);
      return 102;                                                    // invalid command
    }
  } else {
    return 101;                                                      // invalid relay
  }

  return 0;
}

// print status for all relays (triggered = update)
int RelayPrint( const char* cmd, bool triggered)
{
  for( int i = 0; i < RELAY_COUNT; i++) {
    if (( cmd == NULL) ||
       ( strCmp( cmd, CMD_ON ) && ( relaySet[ i] == RELAY_ON )) ||
       ( strCmp( cmd, CMD_OFF) && ( relaySet[ i] == RELAY_OFF))) {
      RelayPrint( i, triggered);
    }
  }

  return 0;
}

// print status for relays idx (triggered = update)
int RelayPrint( int idx, bool triggered)
{
  if (( idx >= 0) & ( idx < RELAY_COUNT)) {                          // check if relay exists
    ATTR_( Serial, F( "# relay "), idx);
    ATTR_( Serial, F( " on pin "), relayPin[ idx]);
    LINE_( Serial, triggered ? " switched " : " = ");
    LINE ( Serial, relaySet[ idx] == RELAY_ON ? CMD_ON : CMD_OFF);

    return 0;
  } else {
    ATTR_( Serial, "# error: relay ", idx); LINE ( Serial, " not defined");
    return 101;
  }
}

bool Relay2Json( const char* cmd, const char* content, unsigned int size)
{
  StaticJsonBuffer<200> jsonBuffer;                   // create buffer
  JsonArray& root     = jsonBuffer.createArray();     // create array

  for( int i = 0; i < RELAY_COUNT; i++) {
    if (( cmd == NULL) ||
       ( strCmp( cmd, CMD_ON ) && ( relaySet[ i] == RELAY_ON )) ||
       ( strCmp( cmd, CMD_OFF) && ( relaySet[ i] == RELAY_OFF))) {
      JsonObject& item = root.createNestedObject();   // create object

      item[ "relay"] = i;                             // write object
      item[ "state"] = ( relaySet[ i] == RELAY_ON ? CMD_ON : CMD_OFF);
    }
  }

  root.printTo( content, size);                       // write to char buffer

  return true;                                        // success = json created
}

bool Relay2Json( int idx, const char* content, unsigned int size)
{
  if (( idx >= 0) & ( idx < RELAY_COUNT)) {           // check if relay exists
    StaticJsonBuffer<200> jsonBuffer;                 // create buffer
    JsonArray& root     = jsonBuffer.createArray();   // create array
    JsonObject& item    = root.createNestedObject();  // create object

    item[ "relay"] = idx;                             // write object
    item[ "state"] = ( relaySet[ idx] == RELAY_ON ? CMD_ON : CMD_OFF);

    root.printTo( content, size);                     // write to char buffer
    return true;                                      // success = json created
  } else {

    strcpy( content, "");                             // clear char buffer
    return false;                                     // failure = no json
  }
}
