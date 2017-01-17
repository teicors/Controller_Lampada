#include "../include/configuration.h"

#include <SmingCore.h>

LampConfig ActiveConfig;

#define SENSORDATA_JSON_SIZE (JSON_OBJECT_SIZE(8))

extern	String NetworkSSID;
extern	String NetworkPassword;
extern  LampConfig LampCfg;


void loadConfig()
{
    DynamicJsonBuffer jsonBuffer;
    LampConfig cfg;
    if (fileExist(LAMP_CONFIG_FILE))
    {
        int size = fileGetSize(LAMP_CONFIG_FILE);
        char* jsonString = new char[size + 1];
        fileGetContent(LAMP_CONFIG_FILE, jsonString, size + 1);
        JsonObject& root = jsonBuffer.parseObject(jsonString);
        LampCfg.lamp =          (int)root["lamp"];
        LampCfg.powerenabled =  (int)root["powerenabled"];
        LampCfg.alarmenabled =  (int)root["alarmenabled"];
        LampCfg.buzzerenabled = (int)root["buzzerenabled"];
        LampCfg.alarmtime =     String((const char*)root["alarmtime"]);
        LampCfg.powertime =     String((const char*)root["powertime"]);
        delete[] jsonString;
    }
    else
    {
        debugf("Il file non esiste");
        LampCfg.NetworkSSID = WIFI_SSID;
        LampCfg.NetworkPassword = WIFI_PWD;
    }
}

void saveConfig()
{
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["lamp"] = LampCfg.lamp;

    root["powerenabled"]  = LampCfg.powerenabled;
    root["alarmenabled"]  = LampCfg.alarmenabled;
    root["buzzerenabled"] = LampCfg.buzzerenabled;

    root["alarmtime"] = LampCfg.alarmtime.c_str();
    root["powertime"] = LampCfg.powertime.c_str();


    char buf[3096];
    root.prettyPrintTo(buf, sizeof(buf));
    fileSetContent(LAMP_CONFIG_FILE, buf);
    root.printTo(Serial);
}


