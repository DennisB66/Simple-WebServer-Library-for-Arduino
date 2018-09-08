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

int errorCode = 400;

SimpleWebServerTask::SimpleWebServerTask( TaskFunc func, const char* device, HTTPMethod method)
: SimpleTask( func)
, _device( NULL)
, _method( method)
{
  if ( device) {                                            // create device string
    _device = (char*)  malloc( sizeof( char) * ( strlen( device) + 1));
    strcpy( _device, device);
  }
}

SimpleWebServerTask::~SimpleWebServerTask()
{
  if ( _device) free( _device);                             // destroy device string
}

// return targeted device
const char* SimpleWebServerTask::device()
{
  return _device;
}

// return targeted method
HTTPMethod SimpleWebServerTask::method()
{
  return _method;
}

// create server instance (port = 80)
SimpleWebServer::SimpleWebServer( int port)
: SimpleTaskList()
, _port  ( port)
, _server( port)
, _pathCount( 0)
, _argsCount( 0)
, _pathList ( 0)
, _argsList ( 0)
{
}

// start webserver
void SimpleWebServer::begin()
{
  _server.begin();                                          // ethernet server begin
}

// return server port number
int SimpleWebServer::getPort()
{
  return _port;                                            // return server port number
}

// check on incoming client HTTP request
bool SimpleWebServer::available()
{
#if   defined(__AVR__)
  EthernetClient client = _server.available();              // check on incoming client request
#elif defined(ESP8266)
  WiFiClient     client = _server.available();              // check on incoming client request
#endif

  if ( client) {                                            // incoming client request
    memset( _buffer, 0, HTTP_BUFFER_SIZE);                  // clear buffer

    int i = 0;                                              // read counter
    while ( client.connected()) {                           // check on active client session
      if ( client.available()) {                            // check on available client data
        char c = client.read();                             // read client data (char by char)

        if (( c == '\r') || ( c == '\n')) {                 // check on end of HTTP request
          break;                                            // HTTP request complete
        }

        if ( i < HTTP_BUFFER_SIZE - 1) {                    // check on buffer overrun (safeguard terminator)
          _buffer[i++] = c;                                 // store incoming data into buffer
        }
      }
      _client = client;                                     // retain client session
    }

    #ifdef DEBUG_SIMPLE_WEBSERVER                           // print results if debugging
    PRINT( F( "> ")); PRINT( _buffer) LF;                   // print full HTTP request including \n"
    #endif

    _parseRequest();                                        // extract http path + args
    _parsePath();                                           // http path items into path array
    _parseArgs();                                           // http args items into args array

    if ( strlen( _pathList[0]) == 0) {                      // if path = empty
      response( HTTP_OK);                                   // respond on HTTP Identify request
      return false;                                         // failure: no further processing required
    } else {
      return true;                                          // success: ready for further processing
    }
  }

  return false;                                             // failure: no HTTP request available
}

// send response (code = 200 OK)
void SimpleWebServer::response( int code)
{
  _sendHeader( code);                                       // send header with response code
  _client.flush();
  _client.stop();                                           // end of client session
}

// send response (code, content type)
void SimpleWebServer::response( int code, const char* content_type)
{
  _sendHeader( code, 0, content_type);                      // send header with response code + content type
  _client.flush();
  _client.stop();                                           // end of client session
}

// send response (code, content type, content)
void SimpleWebServer::response( int code, const char* content_type, char* content)
{
  _sendHeader( code, strlen( content) + 1, content_type);   // send header with response code + content type
  _sendContent( content);                                   // send content (e.g. JSON)
  _client.flush();
  _client.stop();                                           // end of client session
}
// send response (code, content type, FLASH content)
void SimpleWebServer::response( int code, const char* content_type, __FlashStringHelper* content)
{
  _sendHeader( code, 0, content_type);                      // send header with response code + content type
  _sendContent( content);                                   // send content (e.g. JSON)
  _client.flush();
  _client.stop();                                           // terminate client session
}

// attach callback function (callback, device, method)
void  SimpleWebServer::handleOn( TaskFunc func, const char* name, HTTPMethod method)
{
  SimpleWebServerTask* task = new SimpleWebServerTask( func, name, method);
                                                            // create new webserver task
  _attach( task);                                           // attach task to list
}

// route incoming requests to the proper callback
void SimpleWebServer::handle()
{
  SimpleWebServerTask* task = (SimpleWebServerTask*) _rootTask;
                                                            // start with first task
  TaskFunc             func;                                // last callback function

  errorCode = 400;                                          // default return code = error

  if ( available()) {                                       // request available from client
    while ( task != NULL) {                                 // if list contains tasks
      func = task->func();                                  // get callback function
      if ( method( task->method()) && path( 0, task->device())) {
                                                            // check targeted method & targeted device
        ( *func)();                                         // execute callback function
      }

      task = (SimpleWebServerTask*) task->next();           // goto next task
    }

    response( errorCode);                                   // respond to client
  }
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
char* SimpleWebServer::path( int i)
{
  return (( i >= 0) && ( i < _pathCount)) ? _pathList[ i] : NULL;
}                                                           // return path item at index i

// checks if pathItem exists at index i
bool SimpleWebServer::path( int i, const char* pathItem)
{
  return (( i >= 0) && ( i < _pathCount)) ? strCmp( pathItem, _pathList[ i]) : false;
}                                                           // return true if pathItem exists

// return number of (recognized) arguments
int SimpleWebServer::argsCount()
{
  return _argsCount;                                        // return number of items in args array
}

// return label of n-th (recognized) argument
char* SimpleWebServer::argLabel( int i)
{
  return (( i >= 0) && ( i < _argsCount)) ? _argsList[ i].label : NULL;
}                                                           // return label of argument at index i

// return value of n-th (recognized) argument
char* SimpleWebServer::arg( int i)
{
  return (( i >= 0) && ( i < _argsCount)) ? _argsList[i].value : NULL;
}                                                           // return value of argument at index i

// checks if argmument value exists at index i
bool SimpleWebServer::arg( int i, const char* value)
{
  return (( i >= 0) && ( i < _argsCount)) ? strCmp( value, _argsList[i].value) : false;
}                                                           // return true if value exists at index i

// return value of argument with a specfic label
char* SimpleWebServer::arg( const char* name)
{
  for ( int i = 0; i < _argsCount; i++) {                   // for all args items
    if ( strCmp( _argsList[ i].label, name)) {              // if name exists
      return _argsList[ i].value ?  _argsList[ i].value : _argsList[ i].label;
                                                            // return value at index i
    }
  }

  return NULL;
}

// return true if value of argument with a specfic label exists
bool SimpleWebServer::arg( const char* name, const char* value)
{
  for ( int i = 0; i < _argsCount; i++) {                   // for all args items
    if ( strCmp( _argsList[ i].label, name)) {              // if name exists
      return strCmp( _argsList[ i].value, value);           // return true if value exists at index i
    }
  }

  return false;
}

// stop  client session
void SimpleWebServer::_clientStop()
{
  if ( _client.connected()) _client.stop();                 // terminate client session if still open
}

// parse HTTP request into path & args
void SimpleWebServer::_parseRequest()
{                                                           // "GET /path?arg=0 HTTP/1.1"
  char* meth_e = strchr( _buffer, ' ');                     //  ^ end of method
  char* path_s = strchr( _buffer, '/');                     //      ^ start of path
  char* path_e = strchr( path_s + 1 , ' ');                 //                 ^ end of path
  char* args_s = strchr( path_s + 1 , '?');                 //           ^ start of args
  char* vers_s = strstr( path_e + 1 , "HTTP/");             //                  ^ start of version

  if (( path_s == 0) || ( path_e == 0) || ( vers_s == 0)) {
    return;                                                 // failure: no valid HTTP request
  }

  if ( args_s) {
    _path = path_s; _args = args_s; strClr( path_e);        // set path + set args
  } else {
    _path = path_s; _args = path_e; strClr( path_e);        // set path + set args to empty string
  }

  _version = vers_s + 5; //strClr( vers_s + 8);
  _method  = HTTP_ANY;   strClr( meth_e);                   // determine method index
  if ( strCmp( _buffer, "GET"    )) { _method = HTTP_GET;     } else
  if ( strCmp( _buffer, "POST"   )) { _method = HTTP_POST;    } else
  if ( strCmp( _buffer, "DELETE" )) { _method = HTTP_DELETE;  } else
  if ( strCmp( _buffer, "OPTIONS")) { _method = HTTP_OPTIONS; } else
  if ( strCmp( _buffer, "PUT"    )) { _method = HTTP_PUT;     } else
  if ( strCmp( _buffer, "PATCH"  )) { _method = HTTP_PATCH;   }

  #ifdef DEBUG_SIMPLE_WEBSERVER
  VALUE( F( "> method "), _buffer ) LF;
  VALUE( F( "> version"), _version) LF;
  VALUE( F( "> path"   ), _path ? _path : "NULL\r\n") LF;
  VALUE( F( "> args"   ), _args ? _args : "NULL\r\n") LF;
  #endif
}

// parse HTTP request path
void SimpleWebServer::_parsePath()
{
  if ( _pathList) delete[] _pathList;                       // destroy argument array
  _pathSize  = strlen( _path);                              // determine path length (incl. args)
  _pathList  = 0;                                           // reset path item array
  _pathCount = 0;                                           // reset path item counter

  for ( int i = 0; i < _pathSize; i++) {                    // for complete path string
    _pathCount += ( _path[i] == '/');                       // count path items starting with '/')
  }

  #ifdef DEBUG_SIMPLE_WEBSERVER
  LABEL( F( "> pathCount = "), _pathCount) LF;
  #endif

  if ( _pathCount == 0) return;                             // no path items found

  _pathList = new pathItem[ _pathCount];                    // create new path item array
                                                            // /path1/path2
  char*  ptr_S = _path;                                     // ^ ptr_S = start of path1
  char*  ptr_E = strchr( ptr_S + 1, '/');                   //       ^ ptr_E = end of path1
  char*  ptr_L = _path + _pathSize;                         //             ^ ptr_L =end of path
  int    count = 0;                                         // path item count

  while ( count < _pathCount) {                             // for all path items
    ptr_E = ptr_E ? ptr_E : ptr_L;                          // check if ptr_E is still valid

    _pathList[ count] = ptr_S + 1; *ptr_S = 0;              // path item = 1st char after '/'

    ptr_S = ptr_E;                                          // new path item start = old path item end
    ptr_E = strchr( ptr_S + 1, '/');                        // ptr_E = next '/' after ptr_S
    count++;                                                // prepare for next argument
  }

  #ifdef DEBUG_SIMPLE_WEBSERVER
  for ( int i = 0; i < _pathCount; i++) {
    LABEL( F( "> path "),  i);
    LABEL( F( ": "     ), _pathList[ i]) LF;
  }
  #endif
}

// parse HTTP request args
void SimpleWebServer::_parseArgs()
{
  if ( _argsList) delete[] _argsList;                       // destroy argument array
  _argsSize  = strlen( _args);                              // determine args length
  _argsList  = 0;                                           // reset args item array
  _argsCount = ( _args[0] == '?');                          // reset args item counter

  for ( int i = 0; i < _argsSize; i++) {
    _argsCount += ( _args[i] == '&');                       // count arguments ending on '&')
  }

  #ifdef DEBUG_SIMPLE_WEBSERVER
  LABEL( F("> argsCount = "), _argsCount) LF;
  #endif

  if ( _argsCount == 0) return;

  _argsList = new argument[ _argsCount];                    // create args item array
                                                            // ?par1=val1&par2=val2
  char* arg_S = _args;                                      // ^ arg_s = start of par1
  char* val_S = strchr( arg_S + 1, '=');                    //      ^ val_s = end of par1
  char* arg_E = strchr( arg_S + 1, '&');                    //           ^ arg_e = end of val1
  char* ptr_L = _args + _argsSize;                          //                     ^ ptr_L = end of args
  int   count = 0;                                          // args item counter

  while ( count < _argsCount) {
    val_S = val_S ? val_S : ptr_L;                          // check if val_S is still valid
    arg_E = arg_E ? arg_E : ptr_L;                          // check if arg_E is still valid

    if ( val_S < arg_E) {                                   // check if proper '=' has been found
      _argsList[ count].label = arg_S + 1; *arg_S = 0;      // label item = 1st char after '?' or '&'
      _argsList[ count].value = val_S + 1; *val_S = 0;      // value item = 1st char after '='
    } else {
      _argsList[ count].label = arg_S + 1; *arg_S = 0;      // label item = 1st char after '?' or '&'
      _argsList[ count].value = NULL;                       // value item = non existing

      #ifdef DEBUG_SIMPLE_WEBSERVER
      VALUE( F( "> error in argument list for argument "), count);
      #endif
    }

    arg_S = arg_E;                                          // label item = next parem item
    val_S = strchr( arg_S + 1, '=');                        // set end of val (next '=')
    arg_E = strchr( arg_S + 1, '&');                        // set end of arg (next'&'
    count++;                                                // prepare for next argument
  }

  strClr( _args);                                           // terminate _path correctly

  #ifdef DEBUG_SIMPLE_WEBSERVER
  for ( int i = 0; i < _argsCount; i++) {
    LABEL( F( "> args "  ),  i); PRINT( F(  ": "));
    VALUE( _argsList[ i].label, _argsList[ i].value) LF;
  }
  #endif
}

#if defined(SIMPLEWEBSERVER_DEBUG)
#define CPRINT(S) _client.print(S); PRINT(S);
#else
#define CPRINT(S) _client.print(S);
#endif

// send response to client (code, content type)
void SimpleWebServer::_sendHeader( int code, size_t size, const char* content_type)
{
  if ( _client.connected()) {                               // if client session still active
    CPRINT( F( "HTTP/1.1 "));                               // send return code
    CPRINT( code);
    CPRINT( " " );
    CPRINT( HTTP_CodeMessage( code));
    CPRINT( "\r\n");

  //CPRINT( "Host"); CPRINT( Ethernet.getHostName();

    CPRINT( F( "User-Agent: "));                            // send additional headers
    CPRINT( F( "Arduino-ethernet"));
    CPRINT( "\r\n");

    CPRINT( F( "Content-Length: "));
    CPRINT( size);
    CPRINT( "\r\n");

    CPRINT( F( "Content-Type: "));
    CPRINT( content_type ? content_type : "text/html");
    CPRINT( "\r\n");

    CPRINT( F( "Connection: "));
    CPRINT( F( "close"));
    CPRINT( "\r\n");
    CPRINT( "\r\n");
  }
}

// send content to client (content)
void SimpleWebServer::_sendContent( const char* content)
{
  if ( _client.connected()) {                               // if client session still active
    CPRINT( content);                                       // send content
    CPRINT( "\r\n");
  }
}

// send content to client (FLASH content)
void SimpleWebServer::_sendContent( const __FlashStringHelper* content)
{
  if ( _client.connected()) {                               // if client session still active
    CPRINT( content);                                       // send content
    CPRINT( "\r\n");
  }
}
