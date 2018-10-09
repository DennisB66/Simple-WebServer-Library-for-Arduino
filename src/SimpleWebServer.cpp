// Copyright  : Dennis Buis (2017)
// License    : MIT
// Platform   : Arduino
// Library    : Simple WebServer Library for Arduino & ESP8266
// File       : SimpleWebServer.cpp
// Purpose    : Arduino library to create a simple webserver responding to API calls
// Repository : https://github.com/DennisB66/Simple-WebServer-Library-for-Arduino

#include <Arduino.h>
#include "SimpleWebServer.h"
#include "SimpleUtils.h"

#define SERVER_METH_INIT  0                                 // state engine method read loop
#define SERVER_METH_LOOP  1                                 // state engine method read loop
#define SERVER_METH_DONE  2                                 // state engine method read done
#define SERVER_PATH_INIT  3                                 // state engine path read loop
#define SERVER_PATH_LOOP  4                                 // state engine path read done
#define SERVER_PATH_DONE  5                                 // state engine path read done
#define SERVER_ARGS_INIT  6                                 // state engine argumemt read init
#define SERVER_ARGS_LOOP  7                                 // state engine argumemt read loop
#define SERVER_ARGS_NEXT  8                                 // state engine argumemt next
#define SERVER_HTTP_INIT  9                                 // state engine http version read loop
#define SERVER_HTTP_LOOP 10                                 // state engine http version read done
#define SERVER_HTTP_DONE 11                                 // state engine http version read done
#define HTTP_REQUEST_ERR 12                                 // state engine invalid request

int returnCode = 400;                                       // HTTP response code (default = ERROR)

// create Webserver task (for a specfic method)
SimpleWebServerTask::SimpleWebServerTask( TaskFunc func, const char* device, HTTPMethod method)
: SimpleTask( func)
, _device( NULL)
, _method( method)
{
  if ( device) {                                            // create device string
    _device = (char*)  malloc( sizeof( char) * ( strlen( device) + 1));
    strcpy( _device, device);                               // copy device value
  }
}

// remove Webserver task
SimpleWebServerTask::~SimpleWebServerTask()
{
  if ( _device) free( _device);                             // destroy device string
}

// return targeted device
const char* SimpleWebServerTask::device()
{
  return _device;                                           // return device
}

// return targeted method
HTTPMethod SimpleWebServerTask::method()
{
  return _method;                                           // return method
}

// create server instance (default port = 80)
SimpleWebServer::SimpleWebServer( int port)
: SimpleTaskList()
, _port  ( port)
, _server( port)
{
}

// start webserver
void SimpleWebServer::begin()
{
  _server.begin();                                          // ethernet server begin
}

// return server port number
word SimpleWebServer::port()
{
  return _port;                                             // return server port number
}

// check on incoming connection (true = HTTP request received)
bool SimpleWebServer::connect()
{
  _client = _server.available();

  if ( _client) {
    if ( _client.connected()) {                             // check on active client session
      strClr( _buffer);                                     // reset buffer
      while ( !_client.available()) ;
      while (  _client.available()) {                       // check on available client data
        addChr( _buffer, _client.read(), HTTP_BUFFER_SIZE); // add char to buffer (ignore /r)
      }
    }

#ifdef SIMPLE_WEBSERVER_DEBUG
    PRINT( "#####") LF;
    PRINT( _buffer);                                        // print full HTTP request"
    PRINT( "#####") LF;
#endif

    if ( strlen( _buffer)) return _parseRequest();          // return breakdown HTTP request result
  }

  return false;                                             // failure: invalid HTTP request
}

// close client connection
void SimpleWebServer::disconnect()
{
  _clientStop();                                            // stop client session
}

// attach callback function (callback, device, method)
void  SimpleWebServer::handleOn( TaskFunc func, const char* name, HTTPMethod method)
{
  SimpleWebServerTask* task = new SimpleWebServerTask( func, name, method);
                                                            // create new webserver task
  _attach( task);                                           // attach task to list
}

// route incoming requests to the proper callback function
void SimpleWebServer::handleRequest()
{
  SimpleWebServerTask* task = (SimpleWebServerTask*) _rootTask;
                                                            // first task entry in task list
  while ( task != NULL) {                                   // whlle task entry is valid
    if ( method( task->method()) && path( 0, task->device())) (*task->func())();
                                                            // execute callback function
    task = (SimpleWebServerTask*) task->next();             // next task entry
  }
}

// main E2E loop (from connect to disconnect)
void SimpleWebServer::handle()
{
  returnCode = 400;                                         // default return code = error

  if ( connect()) {                                         // if new request available from client
    if (( _pathCount == 1) && ( _argsCount == 0) && ( path( 0, ""))) {
      returnCode = 200;                                     // HTTP identify
    } else {
      handleRequest();                                      // handle request
    }

    respond( returnCode);                                   // send response
    disconnect();                                           // close client session

    yield();                                                // provide time fpr system tasks
  }
}

// send response (code = 200 OK)
void SimpleWebServer::respond( int code)
{
  _sendHeader( code);                                       // send header with response code

  _header = true;                                           // true = header was sent
}

// send response (code, content type, content size)
void SimpleWebServer::respond( int code, const char* content_type, size_t size)
{
  _sendHeader( code, content_type, size);                   // send header with response code + content type

  _header = true;                                           // true = header was sent
}

// send response (code, content type, content)
void SimpleWebServer::respond( int code, const char* content_type, const char* content)
{
  if ( content) {                                           // if valid content
    _sendHeader ( code, content_type, strlen( content));    // send header (code, content type, content size)
    _sendContent( content);                                 // send content

    _header  = true;                                        // true = header  was sent
    _content = true;                                        // true = content was sent
    _newline = true;                                        // true = extra CR/NL required
  } else {
    _sendHeader( code, content_type);                       // send header (code, content type)

    _header  = true;                                        // true = header  was sent
  }
}

// send response (code, content label, content value)
void SimpleWebServer::sendContent( const char* content)
{
  if ( content) {
    _sendContent( content);                                 // send content

    _content = true;                                        // true = content was sent
    _newline = true;                                        // true = extra CR/NL required
  }
}

// send response (code, content label, content value)
void SimpleWebServer::sendLine( const char* label, const char* value)
{
  if ( label) _sendContent( label);                         // if valid label send label
  if ( value) _sendContent( value);                         // if valid label send value

  _sendContent( "\r\n");                                    // send end of line message

  _content = true;                                          // true = content was sent
  _newline = false;                                         // true = extra CR/NL required
}

// send response (code, content label, content value)
void SimpleWebServer::sendLine( const __FlashStringHelper* label, const char* value)
{
  if ( label) _sendContent( label);                         // if valid label send label
  if ( value) _sendContent( value);                         // if valid label send value

  _sendContent( "\r\n");                                    // send end of line message

  _content = true;                                          // true = content was sent
  _newline = false;                                         // true = extra CR/NL required
}

// return full HTTP request
char* SimpleWebServer::request()
{
  return _buffer;                                           // return HTTP request
}

// return method of HTTP request
HTTPMethod SimpleWebServer::method()
{
  return _method;                                           // return HTTP method
}

// true = active method equals targeted method
bool SimpleWebServer::method( HTTPMethod method)
{
  return (( method == HTTP_ANY) || ( method == _method));   // verify valid method
}

// return number of (recognized) path items
int SimpleWebServer::pathCount()
{
  return _pathCount;                                        // return number of items in path array
}

// returns value of HTTP path at index i
const char* SimpleWebServer::path( uint8_t i)
{
  return ( i < _pathCount) ? _path[ i] : NULL;
}                                                           // return path item at index i

// checks if pathItem exists at index i
bool SimpleWebServer::path( uint8_t i, const char* pathItem)
{
  return ( i < _pathCount) ? strCmp( pathItem, _path[ i]) : false;
}                                                           // return true if pathItem exists

// return number of (recognized) arguments
int SimpleWebServer::argsCount()
{
  return _argsCount;                                        // return number of items in args array
}

// return value of argument with a specfic label
const char* SimpleWebServer::arg( const char* label)
{
  for ( int i = 0; i < _argsCount; i++) {                   // for all args items
    if ( strCmp( _args[ i].label, label)) {                 // if label exists
      return _args[ i].value ? _args[ i].value : _args[ i].label;
    }                                                      // return value at index i
  }

  return NULL;
}

// return true if value of argument with a specfic label exists
bool SimpleWebServer::arg( const char* label, const char* value)
{
  bool found = false;                                       // true = combination found

  for ( int i = 0; i < _argsCount; i++) {                   // for all args items
    found |= (( strCmp( _args[i].label, label)) &&
              ( strCmp( _args[i].value, value)));           // return true if value exists at index i
  }

  return found;                                             // return result
}

// break down HTTP request (e.g. "GET /path/1?arg1=0&arg2=1 HTTP/1.1")
bool SimpleWebServer::_parseRequest()
{
  char* meth = _buffer;                                     // HTTP method  string
  char* http = _buffer;                                     // HTTP version string
  int   mode = SERVER_METH_INIT;                            // state engine value
  int   leng = strlen( _buffer);                            // length of HTTP request

  if ( leng == 0) return false;                             // do nothing if empty request

  _pathCount = 0;                                           // reset number of path items
  _argsCount = 0;                                           // reset number of argument items

  for ( int i = 0; i < leng; i++) {                         // buffer loop
    switch ( mode) {
    case SERVER_METH_INIT :                                 // HTTP method read init
      mode = SERVER_METH_LOOP;                              // no break = include current char in read loop

    case SERVER_METH_LOOP :                                 // HTTP method read loop
      if ( _buffer[i] == ' ') { _buffer[i] = 0; mode = SERVER_METH_DONE; break; }
      break;                                                // break = next char

    case SERVER_METH_DONE :                                 // HTTP method read done
      if ( _buffer[i] == '/') { _buffer[i] = 0; mode = SERVER_PATH_INIT; break; }
      mode = HTTP_BAD_REQUEST;
      break;                                                // break = next char

    case SERVER_PATH_INIT :                                 // HTTP path item read init
      if ( _pathCount == MAX_PATHCOUNT)       { mode = HTTP_BAD_REQUEST; break; }
      _path[ _pathCount++] = _buffer + i;
      mode = SERVER_PATH_LOOP;                              // no break = include current char in read loop

    case SERVER_PATH_LOOP :                                 // HTTP path item read loop
      if ( _buffer[i] == '/') { _buffer[i] = 0; mode = SERVER_PATH_INIT; break; }
      if ( _buffer[i] == '?') { _buffer[i] = 0; mode = SERVER_ARGS_INIT; break; }
      if ( _buffer[i] == ' ') { _buffer[i] = 0; mode = SERVER_HTTP_INIT; break; }
      break;                                                // break = next char

    case SERVER_PATH_DONE :                                 // HTTP path item read done
    case SERVER_ARGS_INIT :                                 // HTTP args item read init
      if ( _argsCount == MAX_ARGSCOUNT)       { mode = HTTP_BAD_REQUEST; break; }
      if ( _buffer[i] == ' ') { _buffer[i] = 0; mode = HTTP_BAD_REQUEST; break; }
      _args[ _argsCount++  ].label = _buffer + i;           // store label
      mode = SERVER_ARGS_LOOP;                              // no break = include current char in read loop

    case SERVER_ARGS_LOOP :                                 // HTTP args item read loop
      if ( _buffer[i] == '=') { _buffer[i] = 0; mode = SERVER_ARGS_NEXT; break; }
      if ( _buffer[i] == '&') { _buffer[i] = 0; mode = SERVER_ARGS_INIT; break; }
      if ( _buffer[i] == ' ') { _buffer[i] = 0; mode = SERVER_HTTP_INIT; break; }
      break;                                                // break = next char

    case SERVER_ARGS_NEXT :                                 // HTTP args item goto next
      _args[ _argsCount - 1].value = _buffer + i;           // store value
      mode = SERVER_ARGS_LOOP;
      break;                                                // break = next char

    case SERVER_HTTP_INIT :                                 // HTTP version read done
      http = _buffer + i;                                   // store version
      mode = SERVER_HTTP_LOOP;                              // no break = include current char in read loop

    case SERVER_HTTP_LOOP :                                 // HTTP version read loop
      if ( _buffer[i] == '\r') { _buffer[i] = 0;  mode = SERVER_HTTP_DONE; break; }
      break;                                                // break = next char

    case SERVER_HTTP_DONE :                                 // HTTP version read done
      break;

    case HTTP_BAD_REQUEST :                                 // onvalid HTTP request
      break;
    }
  }

  _header  = false;                                         // true = header  was sent
  _content = false;                                         // true = content was sent
  _newline = false;                                         // true = extra CR/NL required

  _method = HTTP_ANY;                                       // determine method index
  if ( strCmp( meth, "GET"    )) { _method = HTTP_GET;     } else
  if ( strCmp( meth, "POST"   )) { _method = HTTP_POST;    } else
  if ( strCmp( meth, "DELETE" )) { _method = HTTP_DELETE;  } else
  if ( strCmp( meth, "OPTIONS")) { _method = HTTP_OPTIONS; } else
  if ( strCmp( meth, "PUT"    )) { _method = HTTP_PUT;     } else
  if ( strCmp( meth, "PATCH"  )) { _method = HTTP_PATCH;   }
  _version = http + 5;                                      // strClr( vers_s + 5);

  #ifdef SIMPLE_WEBSERVER_DEBUG
  VALUE( _method); VALUE( _version) LF;

  VALUE( _pathCount);
  for ( int i = 0; i < _pathCount; i++) {
    VALUE( _path[i]);
  } LF;

  VALUE( _argsCount);
  for ( int i = 0; i < _argsCount; i++) {
    VALUE( _args[i].label); VALUE( _args[i].value);
  } LF;
  #endif

  return ( mode != HTTP_BAD_REQUEST);
}

#ifdef SIMPLE_WEBSERVER_DEBUG
#define CPRINT(S) _client.print(S); PRINT(S);
#else
#define CPRINT(S) _client.print(S);
#endif

// send response header to client (code, content size, content type)
void SimpleWebServer::_sendHeader( int code, const char* content_type, size_t size)
{
  if ( !_client.connected() || _header) return;             // check if header can be send

  _sendHeaderBegin(  code);
  _sendHeaderValue( F( "User-Agent")     , F( "Arduino-ethernet"));
  _sendHeaderValue( F( "Content-Type")   , content_type ? content_type : "text/html");

  if ( size) {
    _sendHeaderValue( F( "Content-Length") , dec( size));
  }

  _sendHeaderValue( F( "User-Connection"), F( "close"));
  _sendHeaderClose();
}

// send heade start line (e.g. HTTP/1.1 200 OK)
void SimpleWebServer::_sendHeaderBegin( int code)
{
  if ( !_client.connected()) return;                        // check if client still active

  CPRINT( F( "HTTP/1.1 ")); CPRINT( code);                  // send HTTP/1.1 line
  CPRINT( " ");  CPRINT( HTTP_CodeMessage( code));          // e.g. HTTP/1.1 200 OK
  CPRINT( F( "\r\n"));                                      // next line
}

// send header key value pair (e.g. label: value)
void SimpleWebServer::_sendHeaderValue( const char* label, const char* value)
{
  if ( !_client.connected()) return;                        // check if client still active

  CPRINT( label); CPRINT( ": "); CPRINT( value);
  CPRINT( F( "\r\n"));                                      // next line
}

// send header key value pair (e.g. label: value)
void SimpleWebServer::_sendHeaderValue( const __FlashStringHelper* label, const char* value)
{
  if ( !_client.connected()) return;                        // check if client still active

  CPRINT( label); CPRINT( ": "); CPRINT( value);
  CPRINT( F( "\r\n"));                                      // next line
}

// send header key value pair (e.g. label: value)
void SimpleWebServer::_sendHeaderValue( const __FlashStringHelper* label, const __FlashStringHelper* value)
{
  if ( !_client.connected()) return;                        // check if client still active

  CPRINT( label); CPRINT( ": "); CPRINT( value);
  CPRINT( F( "\r\n"));                                      // next line
}

// send enf of content message
void SimpleWebServer::_sendHeaderClose()
{
  if ( !_client.connected()) return;                        // check if client still active

  CPRINT( F( "\r\n"));                                      // next line
}

// send content to client (content)
void SimpleWebServer::_sendContent( const char* content)
{
  if ( !_client.connected()) return;                        // check if client still active

  CPRINT( content);                                         // send content
}

// send content to client (FLASH content)
void SimpleWebServer::_sendContent( const __FlashStringHelper* content)
{
  if ( !_client.connected()) return;                        // check if client still active

  CPRINT( content);                                         // send content
}

// close client connection
void SimpleWebServer::_clientStop()
{
  if ( !_client.connected()) return;                        // check if client still active

  if ( _content) { CPRINT( F( "\r\n")); }            // send EOL if content has been sent
  if ( _newline) { CPRINT( F( "\r\n")); }            // send EOL if extra /CR/NL required

  _client.flush(); _client.stop();                          // close client session
}
