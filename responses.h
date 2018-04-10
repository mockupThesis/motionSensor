#ifndef _RESPONSES_H
#define _RESPONSES_H

#include <ESPAsyncWebServer.h>
#include "Console.h"

void sendOkResponse(AsyncWebServerRequest* request)
{
    consolePrint("Server", "200 %s %s" ,request->methodToString(), request->url().c_str());
    request->send(200, "application/json", F("{\"status\":\"Ok\"}"));
}

void sendBadResponse(AsyncWebServerRequest* request)
{
    consolePrint("Server", "400 %s %s" ,request->methodToString(), request->url().c_str());
    request->send(400, "application/json", F("{\"status\":\"Bad Request\"}"));
}

void sendNotFoundResponse(AsyncWebServerRequest* request)
{
    consolePrint("Server", "404 %s %s" ,request->methodToString(), request->url().c_str());
    request->send(404, "application/json", F("{\"status\":\"Not found\"}"));
}

void sendFailResponse(AsyncWebServerRequest* request)
{
    consolePrint("Server", "500 %s %s" ,request->methodToString(), request->url().c_str());
    request->send(500, "application/json", F("{\"status\":\"Fail\"}"));
}

#endif