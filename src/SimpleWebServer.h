// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino / ESP8266
// Library    : Simple WebServer Library for Arduino & ESP8266
// File       : SimpleWebServer.h
// Purpose    : Arduino library to create a simple webserver responding to API calls
// Repository : https://github.com/DennisB66/Simple-WebServer-Library-for-Arduino

#ifndef _SIMPLE_WEB_SERVER_H
#define _SIMPLE_WEB_SERVER_H

#include <Arduino.h>

#if   defined(__AVR__)
#include <SPI.h>
#include <Ethernet.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "SimpleTask.h"
#include "SimpleHttp.h"

#define NO_DEBUG_SIMPLE_WEBSERVER                          // use DEBUG_SIMPLE_WEBSERVER

#define HTTP_BUFFER_SIZE  200
#define HTTP_VERS_SIZE      4
#define HTTP_PATH_SIZE     92
#define MAX_PATHCOUNT       4
#define MAX_ARGSCOUNT       4

extern int returnCode;

class SimpleWebServerTask : public SimpleTask               // single callback task
{
public:
  SimpleWebServerTask( TaskFunc, const char*, HTTPMethod = HTTP_ANY);
                                                            // create callbacl task (callback, device, method)
 ~SimpleWebServerTask();

  const char* device();                                     // return targeted device
  HTTPMethod  method();                                     // return targeted HTTP method

protected:
  char*      _device;                                       // targeted device for this task
  HTTPMethod _method;                                       // targeted method for this task
};

class SimpleWebServer : public SimpleTaskList               // webserver with multiple callback tasks
{
public:
  SimpleWebServer( int = 80);                               // create webserver (port)
  void begin();                                             // start webserver
  int  port();                                              // return port number

  bool  connect();                                          // check on incoming connection (HTTP request)
  void  disconnect();                                       // close connection
  void  response( int = 200);                               // send response (code = 200 OK)
  void  response( int, const char*);                        // send response (code, content type)
  void  response( int, const char*, const char*);           // send response (code, content type, content)
  void  response( int, const char*, __FlashStringHelper*);  // send response (code, content type, FLASH content

  void handleOn( TaskFunc, const char*, HTTPMethod);        // attach callback function (callback, device, method)
  void handle();                                            // route incoming requests to the proper callback

  char*       request();                                    // return HTTP request
  HTTPMethod  method();                                     // return HTTP method
  bool        method( HTTPMethod);                          // true = active method equals given method
  int         pathCount();                                  // return number of (recognized) arguments
  const char* path( uint8_t);                               // return n-th part of request path
  bool        path( uint8_t, const char*);                  // true = n-th part equals string
  int         argsCount();                                  // return number of (recognized) arguments
  const char* arg( const char*);                            // return value of argument with a specfic label
  bool        arg( const char*, const char*);               // true = argument with label=value exists

protected:
  int             _port;                                    // port number

#if   defined(__AVR__)
  EthernetServer  _server;                                  // server object (Ethernet based)
  EthernetClient  _client;                                  // client object (Ethernet based)
#elif defined(ESP8266)
  WiFiServer      _server;                                  // server object (WiFi based)
  WiFiClient      _client;                                  // client object (WiFi based)
#endif

  char            _buffer[HTTP_BUFFER_SIZE];                // buffer for HTTP request
  HTTPMethod      _method;                                  // method of HTTP request
  char*           _version;                                 // vesion of hTTP request

  typedef char*   pathItem;                                 // pathItem object (/...)
  struct          argument {                                // argument object (?...)
    char* label;                                            // label of parameter
    char* value;                                            // value of parameter
  };

  int             _pathCount;                               // number of path items
  int             _argsCount;                               // number of arguments
  pathItem        _path[MAX_PATHCOUNT];                     // path item list
  argument        _args[MAX_ARGSCOUNT];                     // argument list

  bool            _header;

  bool _parseRequest();                                     // break down HTTP request
  void _sendHeader( int, size_t = 0, const char* = NULL);   // send response header (code, content type)
  void _sendContent( const char*);                          // send response content (content)
  void _sendContent( const __FlashStringHelper*);           // send response content (FLASH content)
  void _clientStop();                                       // stop client session
};

#endif // SIMPLEWebSERVER_H
