// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple Web Server Library for Arduino
// File       : Simple_HTTP_Blink.ino
// Purpose    : Example to demonstrate working of SimpleWebServer library
// Repository : https://github.com/DennisB66/Simple-Web-Server-Library-for-Arduino
//
// This example works for an Arduino Uno with an Ethernet Shield or an ESP8266
//
// Test working with (replace with proper IP address if changed):
// curl -i -X GET "http://192.168.1.60"                        -> return HTTP identify
// curl -i -X GET "http://192.168.1.60/blink"                  -> show blinking status
// curl -i -X PUT "http://192.168.1.60/blink?state=on"         -> switch blinking on
// curl -i -X PUT "http://192.168.1.60/blink?state=off"        -> switch blinking off

#include "SimpleWebServer.h"
#include "SimpleUtils.h"

#define NO_DEBUG_MODE

#define SERVER_NAME "NetBlink-01"                           // host name
#define SERVER_PORT 80                                      // host port

const char* ssid     = "xxxxxxxx";                          // repce with proper ssid
const char* password = "xxxxxxxx";                          // repce with proper password

byte server_mac[]     = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte server_ip4[]     = { 192, 168, 1, 60 };                // lan ip (e.g. "192.168.1.60")
byte server_gateway[] = { 192, 168, 1, 1 };                 // router gateway
byte server_subnet[]  = { 255, 255, 255, 0 };               // subnet mask

SimpleWebServer server( SERVER_PORT);                       // ardiuno  server

#if defined(ESP8266)
#define LED      2                                          // built-in LED on ESP8266
#else
#define LED     13                                          // built-in LED on Arduino UNO
#endif

#define LED_ON  "on"                                        // command to switch blinking on
#define LED_OFF "off"                                       // command to switch blinking off

bool ledStatus = false;                                     // true = blinking activated

void handleBlink_GET();                                     // callback for API GET handling
void handleBlink_PUT();                                     // callback for API PUT handling

void setup() {
  BEGIN( 9600); LF;                                         // activate Serial out

  PRINT( F( "# ----------------------")) LF;                // show header on server console
  PRINT( F( "# - ESP8266 HTTP Blink -")) LF;
  PRINT( F( "# ----------------------")) LF;
  LABEL( F( "# Built-in led = "), LED)   LF;

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

  server.begin();                                           // start webserver
  server.handleOn( handleBlink_GET, "blink", HTTP_GET);     // set callback for GET on "blink"
  server.handleOn( handleBlink_PUT, "blink", HTTP_PUT);     // set function for PUT on "blink"

  pinMode( LED, OUTPUT);                                    // set LED output mode
  digitalWrite( LED, !ledStatus);                           // switch led on

  PRINT( F( "# ready for HTTP requests")) LF;
}

void loop() {
  server.handle();                                          // call webserver handler
  delay(100);                                               // time for system tasks
}

// handle GET on "blink" commands
void handleBlink_GET()
{
  if (( server.pathCount() > 1) || ( server.argsCount() > 0)) return;
                                                            // check on proper API call
  #ifdef DEBUG_MODE
  VALUE( F( "# dev"   ), F( "relay"));
  VALUE( F( ", idx"   ), idx ? idx : "NULL");
  VALUE( F( ", state" ), cmd ? cmd : "NULL") LF;
  #endif

  if ( ledStatus) {
    server.response( errorCode = 200, "text/plain", F( "Led = on\n" ));
                                                            // response to client
    PRINT( F( "# Led = on" )) LF;             // response to console
  } else {
    server.response( errorCode = 200, "text/plain", F( "Led = off\n"));
                                                            // response to client
    PRINT( F( "# Led = off")) LF;             // response to server console
  }
}

// execute PUT commands
void handleBlink_PUT()
{
  char* cmd  = server.arg( "state");                        // get relay command

  if (( server.pathCount() > 1) || ( server.argsCount() > 1) || ( cmd == NULL)) return;
                                                            // check on proper API call
  #ifdef DEBUG_MODE
  VALUE( F( "# dev"   ), F( "relay"));
  VALUE( F( ", idx"   ), idx ? idx : F( "NULL"));
  VALUE( F( ", state" ), cmd ? cmd : F( "NULL")) LF;
  #endif

  if ( strcmp( cmd, LED_ON) == 0) {
    digitalWrite( LED, !( ledStatus = true ));              // switch led on
    server.response( errorCode = 200, "text/plain", F( "Led switched on\n"));
                                                            // respond to client
    PRINT( F( "# Led switched on")) LF;                     // respond to server console
  }

  if ( strcmp( cmd, LED_OFF) == 0) {
    digitalWrite( LED, !( ledStatus = false));              // switch led off
    server.response( errorCode = 200, "text/plain", F( "Led switched off\n"));
                                                            // respond to client
    PRINT( F( "# Led switched off")) LF;                    // respond to server console
  }
}
