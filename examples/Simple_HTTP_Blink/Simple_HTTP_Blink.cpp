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

#define SERVER_NAME "NetBlink-01"                           // host name
#define SERVER_PORT 80                                      // host port

const char* ssid     = "xxxxxxxx";                          // replace with proper ssid
const char* password = "xxxxxxxx";                          // replace with proper password

byte server_mac[]     = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
byte server_ip4[]     = { 192, 168, 1, 69 };                // lan ip (e.g. "192.168.1.60")
byte server_gateway[] = { 192, 168, 1, 1 };                 // router gateway
byte server_subnet[]  = { 255, 255, 255, 0 };               // subnet mask

SimpleWebServer server( SERVER_PORT);                       // ardiuno  server

#define LED_ON  "on"                                        // command to switch blinking on
#define LED_OFF "off"                                       // command to switch blinking off

bool ledStatus;                                             // ledStatus on startup

void handleBlink_GET();                                     // callback for API GET handling
void handleBlink_PUT();                                     // callback for API PUT handling

void setup() {
  BEGIN( 115200) LF;                                             // activate Serial out
  PRINT( F( "# ================")) LF;                        // show header on server console
  PRINT( F( "# -  HTTP Blink  -")) LF;
  PRINT( F( "# ================")) LF;
  LABEL( F( "# Built-in led = "), LED_BUILTIN)   LF;

#if defined(ESP8266)                                        // ESP8266 = connect via WiFi
  WiFi.hostname( SERVER_NAME);                              // set host name
  WiFi.config( server_ip4, server_gateway, server_subnet);  // set fixed IP address
  WiFi.begin( ssid, password);                              // open WiFi connection
  while ( WiFi.status() != WL_CONNECTED) delay(500);        // wait for  connection

  LABEL( F( "# Connected to "), ssid);
  LABEL( F( " / IP = "), WiFi.localIP()) LF;
  PRINT( F( "#")) LF;
#else                                                       // Arduino = connect via Ethernet
//Ethernet.hostName( SERVER_NAME);                          // not supported (yet)
  ETHERNET_RESET( 11U);                                     // Leonardo ETH reset
  Ethernet.begin( server_mac, server_ip4, server_gateway, server_subnet);
                                                            // open ethernet connection
  LABEL( F( "# Connected to "), Ethernet.localIP()) LF;
#endif

  server.begin();                                           // start webserver
  server.handleOn( handleBlink_GET, "blink", HTTP_GET);     // set callback for GET on "blink"
  server.handleOn( handleBlink_PUT, "blink", HTTP_PUT);     // set function for PUT on "blink"

  pinMode( LED_BUILTIN, INPUT);                             // set LED output mode
  ledStatus = digitalRead( LED_BUILTIN);

  PRINT( F( "# Ready for HTTP requests")) LF;
}

void loop() {
  server.handle();                                          // call webserver handler
  yield();                                                  // time for system tasks
}

// handle on "blink" GET commands
void handleBlink_GET()
{
  if (( server.pathCount() > 1) || ( server.argsCount() > 0)) return;
                                                            // check on proper API cal
  pinMode( LED_BUILTIN, INPUT);                             // set LED output mode

  if ( ledStatus != digitalRead( LED_BUILTIN)) {
    server.response( returnCode = 200, "text/plain", "Led = on\n" );
                                                            // response to client
    PRINT( F( "# Led = on" )) LF;                           // response to console
  } else {
    server.response( returnCode = 200, "text/plain", "Led = off\n");
                                                            // response to client
    PRINT( F( "# Led = off")) LF;                           // response to server console
  }
}

// handle on "blink" PUT commands
void handleBlink_PUT()
{
  if (( server.pathCount() > 1) || ( server.argsCount() > 2)) return;
                                                            // check on proper API call
  pinMode( LED_BUILTIN, OUTPUT);                            // set LED output mode

  if ( server.arg( "state", LED_ON )) {
    digitalWrite( LED_BUILTIN, !ledStatus);                 // switch led on
    server.response( returnCode = 200, "text/plain", "Led switched on");
                                                            // respond to client
    PRINT( F( "# Led switched on" )) LF;                    // respond to server console
  }

  if ( server.arg( "state", LED_OFF)) {
    digitalWrite( LED_BUILTIN,  ledStatus);                 // switch led off
    server.response( returnCode = 200, "text/plain", "Led switched off");
                                                            // respond to client
    PRINT( F( "# Led switched off")) LF;                    // respond to server console
  }
}
