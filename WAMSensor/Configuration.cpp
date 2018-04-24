#include "Configuration.h"
#include "Console.h"

const char* Configuration::TAG = "Config";

Configuration::Configuration(const char* configFilename, const char* defaultSsid, const char* defaultPass,
                             const char* defaultMqttHost, int mqttPort, int defaultAp) :
    mFileName(configFilename),
    mApMode(defaultAp),
    mMqttPort(mqttPort)
{
    strlcpy(this->mSsid, defaultSsid, sizeof(this->mSsid));
    strlcpy(this->mPass, defaultPass, sizeof(this->mPass));
    strlcpy(this->mMqttHost, defaultMqttHost, sizeof(this->mMqttHost));
}

bool Configuration::begin()
{
    consolePrint(TAG, "Initializing Filesystem");
    SPIFFS.begin();

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    if (fs_info.totalBytes == 0)
    {
        consolePrint(TAG, "No filesystem found!");
        if (SPIFFS.format())
        {
            consolePrint(TAG, "Formatted fileystem.");
            SPIFFS.info(fs_info);
        } else {
            consolePrint(TAG, "Format error");
            return false;
        }
    }
    consolePrint(TAG, "Filesystem Total bytes: %d b, Used bytes: %d b.", fs_info.totalBytes, fs_info.usedBytes);
    File file = SPIFFS.open(this->mFileName, "r");
    if (!file)
    {
        consolePrint(TAG, "No configuration file found. Creating file ...");
        this->save();
        file = SPIFFS.open(this->mFileName, "r");
        if (!file)
        {
            consolePrint(TAG, "Config file can't be created");
            return false;
        }
    }
    file.close();

    this->load();

    return true;

}

bool Configuration::load()
{
    consolePrint(TAG, "Loading config file: %s", this->mFileName);
    File file = SPIFFS.open(this->mFileName, "r");
    StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(file);
    file.close();
    bool success = root.success();
    if (root.containsKey("ssid"))
        strlcpy(this->mSsid, root["ssid"], sizeof(this->mSsid));

    if (root.containsKey("pass"))
        strlcpy(this->mPass, root["pass"], sizeof(this->mPass));
    
    if (root.containsKey("ap"))
        this->mApMode = root["ap"];

    if (root.containsKey("mqtt_host"))
        strlcpy(this->mMqttHost, root["mqtt_host"], sizeof(this->mMqttHost));

    if (root.containsKey("mqtt_port"))
        this->mMqttPort = root["mqtt_port"];

    consolePrint(TAG, "Loaded: ");
    root.prettyPrintTo(Serial);
    Serial.println();

    if (!success)
    {
        consolePrint(TAG, "Invalid JOSN format. Recreating file ...");
        this->save();
    }
    return true;
}

bool Configuration::save()
{
    consolePrint(TAG, "Saving config");
    SPIFFS.remove(this->mFileName);

    StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["ssid"] = this->mSsid;
    root["pass"] = this->mPass;
    root["ap"] = this->mApMode;
    root["mqtt_host"] = this->mMqttHost;
    root["mqtt_port"] = this->mMqttPort;
    File file = SPIFFS.open(this->mFileName, "w");
    if (root.printTo(file) == 0) {
        consolePrint(TAG, "Failed saving config");
        return false;
    }
    file.close();
    return true;
}

const char* Configuration::ssid() const
{
    return this->mSsid;
}

void Configuration::setSsid(const char* ssid)
{
    strlcpy(this->mSsid, ssid, sizeof(this->mSsid));
}

const char* Configuration::password() const
{
    return this->mPass;
}

void Configuration::setPassword(const char* password)
{
    strlcpy(this->mPass, password, sizeof(this->mPass));
}

int Configuration::apMode() const
{
    return this->mApMode;
}

void Configuration::setApMode(int apMode)
{
    this->mApMode = apMode;
}

const char* Configuration::mqttHost() const
{
    return this->mMqttHost;
}

int Configuration::mqttPort() const
{
    return this->mMqttPort;
}

