#ifndef _OTAHANDLER_H
#define _OTAHANDLER_H

#include <ArduinoOTA.h>

class OTAHandler {

  static const char* TAG;
  String hostname;

  static void onUpdateStart();
  static void onUpdateProgress(unsigned int progress, unsigned int total);
  static void onUpdateEnd();
  static void onUpdateError(ota_error_t error);

public:

  explicit OTAHandler(String hostname);

  void begin();
  void handle();

};

#endif // _OTAHANDLER_H
