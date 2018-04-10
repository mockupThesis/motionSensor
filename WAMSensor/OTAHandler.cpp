#include "OTAHandler.h"
#include "Console.h"

const char* OTAHandler::TAG = "OTA";

void OTAHandler::onUpdateStart()
{
    consolePrint(TAG, "Update Start ...");
}

void OTAHandler::onUpdateProgress(unsigned int progress, unsigned int total)
{
    consolePrint(TAG, "Progress: %u%%", (progress / (total / 100)));
}

void OTAHandler::onUpdateEnd()
{
    consolePrint(TAG, "Update Success.");
}

void OTAHandler::onUpdateError(ota_error_t error)
{
    error = error;
    ESP.restart();
}

OTAHandler::OTAHandler(String hostname) : 
    hostname(hostname)
{}

void OTAHandler::begin()
{
    consolePrint(TAG, "Starting service.");
    ArduinoOTA.setHostname(hostname.c_str());
    ArduinoOTA.onStart(OTAHandler::onUpdateStart);
    ArduinoOTA.onProgress(OTAHandler::onUpdateProgress);
    ArduinoOTA.onEnd(OTAHandler::onUpdateEnd);
    ArduinoOTA.onError(OTAHandler::onUpdateError);
    ArduinoOTA.begin();
}

void OTAHandler::handle()
{
  ArduinoOTA.handle();
}
