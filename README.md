# Simple WebServer Library for Arduino and ESP8266

Simple webServer is a small but flexible library for Arduino and ESP8266 that implements a basic HTTP server that supports straightforward API calls.

The primary goal was to create a Arduino sketch that can switch a number of relays to activate garden lights via wired ethernet. The example sketches shows how the library can be applied and demonstrates the use of an API based Arduino with an Ethernet shield or ESP8266 connected to a 4 relay board (Simple_HTTP_Relay), including an extension with JSON based responses (Simple_HTTP_Relay_JSON, requires the ArduinoJSON library). A more basic example (Simple_HTTP_Blink) is also included that activates the built-in led over HTTP.

## Functionality
The functions in the class SimpleWebServer include:
```
begin()			// start server session
getPort			// return active port number
connect()		// check on incoming connection (HTTP request from client)
connect()		// close connection (with client)
response()	        // send response to client
handleOn()		// attach callback function
handle()		// route HTTP request to proper callback function
getRequest()		// return pending HTTP request
getMethod()	        // return pending HTTP method
path()                  // return a specific path item of HTTP request
argCount()              // return number of (recognized) arguments of HTTP request
argLabel() 	        // return label of n-th argument of HTTP request
arg()		        // return value of a specific argument (by index or label) of HTTP request
```

## Library Dependencies

- https://github.com/DennisB66/Simple-Utility-Library-for-Arduino
- https://github.com/bblanchon/ArduinoJson (example only)
