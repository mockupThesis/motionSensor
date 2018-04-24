#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <ArduinoJson.h>
#include <FS.h>

#define DEBUG                   true
#define JSON_BUFFER_SIZE        200

class Configuration
{
public:
    Configuration(const char* configFileName, const char* defaultSsid, const char* defaultPass,
                  const char* defaultMqttHost, int mqttPort, int defaultAp);
    bool begin();
    bool load();
    bool save();

    const char* ssid() const;
    void setSsid(const char* ssid);
    const char* password() const;
    void setPassword(const char* password);
    int apMode() const;
    void setApMode(int apMode);
    const char* mqttHost() const;
    int mqttPort() const;


private:
    static const char* TAG;
    const char* mFileName;
    char mSsid[32];
    char mPass[63];
    char mMqttHost[32];
    int mApMode;
    int mMqttPort;

    void createConfigFile();
};

#endif
