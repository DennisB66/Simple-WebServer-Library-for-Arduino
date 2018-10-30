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
// curl -i -X GET "http://192.168.1.68"                        -> return HTTP identify
// curl -i -X GET "http://192.168.1.68/blink"                  -> show blinking status
// curl -i -X PUT "http://192.168.1.68/blink?state=on"         -> switch blinking on
// curl -i -X PUT "http://192.168.1.68/blink?state=off"        -> switch blinking off

#include "SimpleWebServer.h"
#include "SimpleUtils.h"

#define SERVER_NAME "NetBlink-01"                           // host name
#define SERVER_PORT 80                                      // host port

#include "MySecrets.h"                                      // Wifi Settings (change in MySecrets.h)
const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;

byte server_mac[]     = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
byte server_ip4[]     = { 192, 168,   1,  69 };             // lan ip (e.g. "192.168.1.68")
byte server_gateway[] = { 192, 168,   1,   1 };             // router gateway
byte server_subnet[]  = { 255, 255, 255,   0 };             // subnet mask

SimpleWebServer server( SERVER_PORT);                       // ardiuno  server

#define LED_DEFAULT 2
#define LED_ON      1                                       // value to switch led on
#define LED_OFF     0                                       // value to switch led off

#define CMD_ON  "on"                                        // command to switch led on
#define CMD_OFF "off"                                       // command to switch led off

bool ledStatus;                                             // ledStatus on startup

void handleBlink_GET();                                     // callback for API GET handling
void handleBlink_PUT();                                     // callback for API PUT handling

void setup() {
  BEGIN( 115200) LF;                                        // activate Serial out

  PRINT( F( "# -----------------------")) LF;               // show header
  PRINT( F( "# -  Simple HTTP Blink  -")) LF;
  PRINT( F( "# -  V0.8  (DennisB66)  -")) LF;
  PRINT( F( "# -----------------------")) LF;
  PRINT( F( "#")) LF;
  LABEL( F( "# Built-in led = "), LED_DEFAULT) LF;

#if defined(ESP8266)                                        // ESP8266 = connect via WiFi
  WiFi.hostname( SERVER_NAME);                              // set host name
  WiFi.config( server_ip4, server_gateway, server_subnet);  // set fixed IP address
  WiFi.begin( ssid, pass);                                  // open WiFi connection
  while ( WiFi.status() != WL_CONNECTED) delay(500);        // wait for  connection

  LABEL( F( "# Connected to "), ssid);
  LABEL( F( " / IP = "), WiFi.localIP()) LF;
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

  PRINT( F( "# ready for HTTP requests")) LF;
  PRINT( F( "#")) LF;
}

void loop() {
  server.handle(); yield();                                 // call webserver handler
}

// handle on "blink" GET commands
void handleBlink_GET()
{                                                           // check on path / args boundaries
  if (( server.pathCount() > 1) || ( server.argsCount() > 0)) return;

  bool state = digitalRead( 2);

  server.respond( returnCode = 200, "text/plain");          // response to client
  server.sendLine( F( "led = "), state ? CMD_ON : CMD_OFF);

  LABEL( F( "# Led = "), state) LF;                         // response to console
}

// handle on "blink" PUT commands
void handleBlink_PUT()
{                                                           // check on path / args boundaries
  if (( server.pathCount() > 1) || ( server.argsCount() > 1)) return;

  pinMode( LED_DEFAULT, OUTPUT);                            // set LED output mode

  if ( server.arg( "state", CMD_ON )) {
    digitalWrite( LED_DEFAULT, LED_ON);                     // switch led on
    server.respond( returnCode = 200, "text/plain");        // response to client
    server.sendLine( F( "led switched "), CMD_ON );
                                                            // respond to client
    LABEL( F( "# Led switched "), CMD_ON ) LF;              // respond to server console
  }

  if ( server.arg( "state", CMD_OFF)) {
    digitalWrite( LED_DEFAULT, LED_OFF);                    // switch led off
    server.respond( returnCode = 200, "text/plain");        // response to client
    server.sendLine( F( "led switched "), CMD_OFF);

    LABEL( F( "# Led switched "), CMD_OFF) LF;              // respond to server console
  }
}
