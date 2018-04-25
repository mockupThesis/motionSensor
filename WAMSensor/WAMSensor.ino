#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
extern "C"
{
  #include "user_interface.h"
}
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#include "Console.h"
#include "Configuration.h"
#include "OTAHandler.h"
#include "Multiplexer.h"
#include "Sensors.h"
#include "responses.h"

#define VERSION                 1063
#define CONFIG_PATH             "/config.json"
#define DEBUG                   true
#define DEVICENAME              "WAMSensor"
#define DEFAULT_SSID            "WAMSensor"
#define DEFAULT_PASS            "Dildozer4"
#define DEFAULT_AP              1
#define MQTT_HOST               "mqtt.bayi.hu"
#define MQTT_PORT               1883
#define DEVICE_FIX_IP           IPAddress(192, 168, 1, 219) 
#define GATEWAY                 IPAddress(192, 168, 1, 254)  
#define SUBNET                  IPAddress(255, 255, 255, 0)

typedef struct {
  bool shouldReboot;
  bool shouldReconnect;
  bool shouldSave;
  bool hasSensor;
  bool isConnecting;
  bool hasClient;
  uint8_t connectTries;
  bool isAp;
  bool isRunning;
  IPAddress ip;
} State;

State systemState;
const char* TAG = "Main";
OTAHandler otaHandler(DEVICENAME);
Configuration   conf(CONFIG_PATH, DEFAULT_SSID, DEFAULT_PASS, MQTT_HOST, MQTT_PORT, DEFAULT_AP);
AsyncWebServer  cmdServer(80);
AsyncWebSocket  ws("/ws");
sensors_event_t event;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

char buff[10];
imu::Quaternion quat;

/**
  * @TODO:
  * - wifi setup az html-be, meg connect gomb meg cím beírása stb.
  */

void initSerial()
{
  Serial.begin(115200);
  while(!Serial) {}
  Serial.println(); Serial.println();
  Serial.setDebugOutput(true);
  consolePrint(TAG, "WAMSensor v%u booting ...", VERSION);
}

void initMDNS()
{
  consolePrint(TAG, "Starting mDNS Service");
  MDNS.begin(DEVICENAME);
  MDNS.addService("http", "tcp", 80);
}

void initMQTT()
{
    consolePrint("MQTT", "Connecting to MQTT Broker: %s:%d", conf.mqttHost(), conf.mqttPort());
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.setServer(conf.mqttHost(), conf.mqttPort());
    mqttClient.setCleanSession(true);
    mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
    consolePrint("MQTT", "Connected to MQTT Broker.");
    mqttClient.publish("sensor/status", 0, true, "Sensor connected");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  String text; 
  switch( reason) {
  case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
     text = "TCP_DISCONNECTED"; 
     break; 
  case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
     text = "MQTT_UNACCEPTABLE_PROTOCOL_VERSION"; 
     break; 
  case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
     text = "MQTT_IDENTIFIER_REJECTED";  
     break;
  case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE: 
     text = "MQTT_SERVER_UNAVAILABLE"; 
     break;
  case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
     text = "MQTT_MALFORMED_CREDENTIALS"; 
     break;
  case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
     text = "MQTT_NOT_AUTHORIZED"; 
     break;
  
  }
  Serial.printf(" [%8u] Disconnected from the broker reason = %s\n", millis(), text.c_str() );
  Serial.printf(" [%8u] Reconnecting to MQTT..\n", millis());
  mqttClient.connect();

  /*if (WiFi.status() != WL_CONNECTED) {
    mqttClient.connect();
  }*/
}

void initWifi()
{
  consolePrint("Wifi", "Init Wifi");
  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  wifi_station_set_hostname((char*) DEVICENAME);
  systemState.connectTries = 0;
}

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void*arg, uint8_t* data, size_t len)
{
    // Jelenleg tök mindegy mit küld, ezt kapja ...
    uint8_t sensorNum = 0;
    sensorsGetEvent(sensorNum, &event);

    // MQTT adatküldés
    dtostrf(event.orientation.x, 4, 2, buff);
    mqttClient.publish("sensor/yaw", 0, true, buff);
    dtostrf(event.orientation.y, 4, 2, buff);
    mqttClient.publish("sensor/pitch", 0, true, buff);
    dtostrf(event.orientation.z, 4, 2, buff);
    mqttClient.publish("sensor/roll", 0, true, buff);
    
    if (type == WS_EVT_DATA)
    {
        // @TODO: Send sensornum
        ws.printfAll("%.2f\t%.2f\t%.2f", event.orientation.x, event.orientation.y, event.orientation.z);
        //ws.printfAll("%.2f\t%.2f\t%.2f\t%.2f", quat.w(), quat.x(), quat.y(), quat.z());
    }
}

void initServer()
{
    consolePrint("Server", "Initializing Server");
    ws.onEvent(onEvent);

    cmdServer.addHandler(&ws);
    cmdServer.onNotFound(sendNotFoundResponse);
    cmdServer.serveStatic("/", SPIFFS, "/www/").setLastModified("Mon, 20 Jun 2016 14:00:00 GMT");
    cmdServer.on("/api/status", HTTP_GET, cmdStatus);
    cmdServer.on("/api/reboot", HTTP_POST, cmdReboot);
    cmdServer.on("/api/wifi", HTTP_POST, cmdWifi);
    cmdServer.on("/api/save", HTTP_POST, cmdSave);
    cmdServer.on("/api/reconnect", HTTP_POST, cmdReconnect);
    cmdServer.on("/api/mqtt", HTTP_POST, cmdMqtt);

    cmdServer.begin();

    if (!ws.enabled())
        ws.enable(true);
  
}

void defaultAp()
{
    systemState.isConnecting = true;
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
    consolePrint("Wifi", "Creating AP: %s with password: %s", DEFAULT_SSID, DEFAULT_PASS );
    WiFi.softAP(DEFAULT_SSID, DEFAULT_PASS);
    delay(500);
    systemState.ip = WiFi.softAPIP();
    systemState.isAp = true;
    consolePrint("Wifi", "Connected. IP Address: %s", systemState.ip.toString().c_str());
    otaHandler.begin();
    initMDNS();
    systemState.isConnecting = false;
    digitalWrite(LED_BUILTIN, LOW);
}

bool connectWifi()
{
    digitalWrite(LED_BUILTIN, LOW);
    systemState.connectTries++;
    if (systemState.connectTries > 3)
    {
        defaultAp();
        return true;
    }
    systemState.isConnecting = true;
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);

    if (conf.apMode() == 1)
    {
        consolePrint("Wifi", "Creating AP: %s with password: %s", conf.ssid(), conf.password() );
        WiFi.softAP(conf.ssid(), conf.password());
        delay(500);
        systemState.ip = WiFi.softAPIP();
        systemState.isAp = true;
    } else {
        consolePrint("Wifi", "Trying to connect to: %s" , conf.ssid());
        //WiFi.config(DEVICE_FIX_IP, GATEWAY, SUBNET);
        WiFi.mode(WIFI_STA);
        WiFi.begin(conf.ssid(), conf.password());
        int timeout = 0;
        bool ledState = false;
        Serial.print("[Wifi] Connecting ");
        while (WiFi.status() != WL_CONNECTED && timeout < 50)
        {
            delay(250);
            ++timeout;
            ledState = !ledState;
            if (ledState)
                digitalWrite(LED_BUILTIN, LOW);
            if (ledState)
                digitalWrite(LED_BUILTIN, HIGH);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() != WL_CONNECTED)
        {
            consolePrint("Wifi", "Connection Failed.");
            systemState.isConnecting = false;
            return false;
        } else {
            systemState.ip = WiFi.localIP();
            systemState.isAp = false;
            digitalWrite(LED_BUILTIN, LOW);
        }
    }
    consolePrint("Wifi", "Connected. IP Address: %s", systemState.ip.toString().c_str());
    initMQTT();
    otaHandler.begin();
    initMDNS();
    systemState.connectTries = 0;
    systemState.isConnecting = false;
    digitalWrite(LED_BUILTIN, HIGH);
    return true;
}

void cmdStatus(AsyncWebServerRequest* request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->setCode(200);

    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["version"] = VERSION;
    json["ssid"] = conf.ssid();
    json["ap"] = conf.apMode() == 1;
    json["ip"] = systemState.ip.toString();
    json["heap"] = ESP.getFreeHeap();
    json["connected"] = WiFi.status() == WL_CONNECTED;
    json["mqtt_host"] = conf.mqttHost();
    json["mqtt_port"] = conf.mqttPort();
    json.printTo(*response);
    request->send(response);

    consolePrint("Server", "200 %s %s" ,request->methodToString(), request->url().c_str());
}

void cmdWifi(AsyncWebServerRequest* request)
{
    consolePrint("cmdWifi", "POST REQUEST");
    int params = request->params();
    consolePrint("cmdWifi", "PARAMS COUNT %f", params);
    for (int i = 0; i < params; ++i)
    {
        AsyncWebParameter* p = request->getParam(i);
        consolePrint("cmdWifi", "PARAM %s", p->name().c_str());
        if (p->name().startsWith("body"))
        {
            DynamicJsonBuffer jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(p->value().c_str());
            if (!root.success())
            {
                sendFailResponse(request);
                return;
            }
            conf.setSsid((const char*) root["ssid"]);
            conf.setPassword((const char*) root["pass"]);
            conf.setApMode(root["ap"]);
            sendOkResponse(request);
            return;
        }
    }
    sendBadResponse(request);
}

void cmdSave(AsyncWebServerRequest* request)
{
    systemState.shouldSave = true;
    sendOkResponse(request);
}

void cmdReboot(AsyncWebServerRequest* request)
{
    systemState.shouldReboot = true;
    sendOkResponse(request);
}

void cmdReconnect(AsyncWebServerRequest* request)
{
    systemState.shouldReconnect = true;
    sendOkResponse(request);
}

void cmdMqtt(AsyncWebServerRequest* request)
{
    consolePrint("cmdMqtt", "POST REQUEST");
    int params = request->params();
    consolePrint("cmdMqtt", "PARAMS COUNT %f", params);
    for (int i = 0; i < params; ++i)
    {
        AsyncWebParameter* p = request->getParam(i);
        consolePrint("cmdMqtt", "PARAM %s", p->name().c_str());
        if (p->name().startsWith("body"))
        {
            DynamicJsonBuffer jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(p->value().c_str());
            if (!root.success())
            {
                sendFailResponse(request);
                return;
            }
            conf.setMqttHost((const char*) root["mqtt_host"]);
            conf.setMqttPort(root["mqtt_port"]);
            systemState.shouldSave = true;
            sendOkResponse(request);
            return;
        }
    }
    sendBadResponse(request);
}

void setup()
{
    system_update_cpu_freq(160);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    initSerial();

    conf.begin();

    // @TODO: Itt van valami gebasz, pár újraindítás után az I2C busz "beragad" valami fenn tartja az SDA-t és nem engedi el többé ...
    Wire.begin();

    sensorsBegin();
    systemState.hasSensor = sensorsScan();

    initServer();
    initWifi();

    consolePrint(TAG, "Free memory: %u", ESP.getFreeHeap());
}

void loop()
{
    // State handler

    if (systemState.hasSensor)
    {
        sensorsHandle();
        quat = getSensor().ctrl->getQuat();

        dtostrf(quat.w(), 4, 2, buff);
        mqttClient.publish("sensor/quatW", 0, true, buff);
        dtostrf(quat.y(), 4, 2, buff);
        mqttClient.publish("sensor/quatY", 0, true, buff);
        dtostrf(quat.x(), 4, 2, buff);
        mqttClient.publish("sensor/quatX", 0, true, buff);
        dtostrf(quat.z(), 4, 2, buff);
        mqttClient.publish("sensor/quatZ", 0, true, buff);
  
    }

    if (systemState.shouldReboot)
    {
        delay(500);
        systemState.shouldReboot = false;
        ESP.restart();
        delay(500);
    }
    
    if (systemState.shouldSave)
    {
        delay(500);
        systemState.shouldSave = false;
        conf.save();
    }

    if (systemState.shouldReconnect)
    {
        systemState.shouldReconnect = false;
        systemState.connectTries = 0;
        delay(500);
        connectWifi();
    }

    if (!systemState.isConnecting && WiFi.status() != WL_CONNECTED && !systemState.isAp)
    {
        connectWifi();
    }

    delay(50);

    otaHandler.handle();

}
