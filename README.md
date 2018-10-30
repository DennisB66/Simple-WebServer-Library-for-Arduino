# Simple WebServer Library for Arduino and ESP8266

Simple WebServer is a small but flexible library for Arduino and ESP8266 that implements a basic HTTP server that supports straightforward API calls.

The primary goal was to create a Arduino sketch that can switch a number of relays to activate garden lights via wired ethernet. The example sketches shows how the library can be applied and demonstrates the use of an API based Arduino with an Ethernet shield or ESP8266 connected to a 4 relay board (Simple_HTTP_Relay), including an extension with JSON based responses (Simple_HTTP_Relay_JSON, requires the ArduinoJSON library). A more basic example (Simple_HTTP_Blink) is also included that activates the built-in led over HTTP.

## Functionality
The functions in the class SimpleWebServer include:
```
begin()			// start server session
connect()		// open connection (incoming HTTP request from client)
disconnect()		// close connection (with client)
handleOn()		// attach callback function
handleRequest()		// route incoming requests to the proper callback
handle()		// route HTTP request to proper callback function
respond()	        // send response (to client)
sendContent()		// send response (content)
sendLine()		// send response (content) + LF
port			// return active port number
request()		// return pending HTTP request
method()	        // return pending HTTP method
pathCount()		// return number of (recognized) arguments
path()                  // return a specific path item of HTTP request
argCount()              // return number of (recognized) arguments of HTTP request
arg()		        // return value of a specific argument (by index or label) of HTTP request
```

## Library Dependencies

- https://github.com/DennisB66/Simple-Utility-Library-for-Arduino
- https://github.com/bblanchon/ArduinoJson (example only)

Version history:
```
0.7			// improved design & coding
0.8			// various enhancements
```
