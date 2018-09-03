#ifndef _SENSORS_H
#define _SENSORS_H

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include "Multiplexer.h"
#include "Console.h"
#include "HumanBodyBones.h"

#define SENSOR_COUNT 2

// @TODO: Ebből lehetne valami classt csinálni ...

const char* SENSORSTAG = "Sensors";

typedef struct {
    int32_t id;
    uint8_t port;
    uint8_t address;
    Adafruit_BNO055* ctrl;
    bool inited = false;
    char* bone;
    sensors_event_t event;
} SensorType;

// @TODO: Ebből kellene majd egy tömb
SensorType sensor;

SensorType sensors[SENSOR_COUNT];
int actSensor = 0;

void sensorsBegin()
{
    consolePrint(SENSORSTAG, "Initializing Sensors");
    multiplexerSetChannel(1);
}

bool sensorsScan()
{
    consolePrint(SENSORSTAG, "Scanning I2C Bus ...");
    bool foundSensor = false;
    uint8_t sensorId = 55;
    for (uint8_t port = 0; port < 8; port++)
    {
        multiplexerSetChannel(port);
        consolePrint(SENSORSTAG, "I2C Port#%u", port);
        
        for (uint8_t addr = 0; addr < 127; addr++)
        {
            Wire.beginTransmission(addr);
            byte status = Wire.endTransmission();
            if (status == 0)
            {
                if (addr >= 0x70 && addr <= 0x77)
                {
                    consolePrint(SENSORSTAG, "I2C Multiplexer Device Found @ 0x%X", addr);
                } else {
                    if (addr == 0x28 || addr == 0x29)
                    {
                        consolePrint(SENSORSTAG, "I2C Sensor Found @ 0x%X", addr);
                        sensors[actSensor].address = addr;
                        sensors[actSensor].port = port;
                        sensors[actSensor].ctrl = new Adafruit_BNO055(sensorId, addr);
                        if (sensors[actSensor].ctrl->begin())
                        {
                            consolePrint(SENSORSTAG, "Sensor inited successfully");
                            delay(1000); // Ez rohadtul kell ide
                            sensors[actSensor].ctrl->setExtCrystalUse(true);
                        } else {
                            consolePrint(SENSORSTAG, "Sensor init failed");
                        }
                        sensors[actSensor].inited = true;
                        
                        consolePrint(SENSORSTAG, "%s assigned to the sensor", getSensorTopicNameByID(sensorId));
                        sensors[actSensor].bone = getSensorTopicNameByID(sensorId);
                        
                        sensors[actSensor].id = sensorId++;                  
                        foundSensor = true;
                        actSensor++;
                    } else {
                        consolePrint(SENSORSTAG, "I2C Device Found @ 0x%X", addr);
                    }
                }
            }
        }


    }
    return foundSensor;
}

void sensorsHandle()
{
    if (sensor.inited)
    {
        multiplexerSetChannel(sensor.port);
        sensor.ctrl->getEvent(&sensor.event);
    }
}

void sensorsGetEvent(uint8_t sensorNum, sensors_event_t* event)
{
    // @TODO: Ne szarja már le ennyire látványosan a sensorNum -ot ... ja, kellene hozzá a tömb
    // @TODO: Ezt inkább quaternionba kellene, na meg inkább másolni, vagy mégse, mert a calib datara is szükség lesz, szal kellene saját struct
    for(int i = 0; i < SENSOR_COUNT; i++) {
      if (sensors[i].inited)
        {
          // @TODO: Jobb lenne az adatokat másolni ...
          multiplexerSetChannel(sensors[i].port);
          sensors[i].ctrl->getEvent(event);

          /* Ez se jó ötlet ..
          event->orientation.x = sensor.event.orientation.x;
          event->orientation.y = sensor.event.orientation.y;
          event->orientation.z = sensor.event.orientation.z;
          */
      }
    }
}

imu::Quaternion getQuaternion(int sensorId) {
    if (sensors[sensorId].inited)
    {
        multiplexerSetChannel(sensors[sensorId].port);
        return sensors[sensorId].ctrl->getQuat();
    }
}

imu::Vector<3> getRotation(int sensorId) {
    if (sensors[sensorId].inited)
    {
        multiplexerSetChannel(sensors[sensorId].port);
        return sensors[sensorId].ctrl->getVector(Adafruit_BNO055::VECTOR_EULER);
    }
}

String getSensorBone(int sensorId) {
    return sensors[sensorId].bone;
}

int getSensorCount() {
  return SENSOR_COUNT;
}

#endif
