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
        LampCfg.sleepenabled =  (int)root["sleepenabled"];
        LampCfg.alarmenabled =  (int)root["alarmenabled"];
        LampCfg.buzzerenabled = (int)root["buzzerenabled"];
        LampCfg.powered = (int)root["powered"];
        LampCfg.alarmtime =     String((const char*)root["alarmtime"]);
        LampCfg.sleeptime =     String((const char*)root["sleeptime"]);
        LampCfg.NetworkSSID =     String((const char*)root["ssid"]);
        LampCfg.NetworkPassword =     String((const char*)root["password"]);
        Serial.print(LampCfg.lamp);
        Serial.print(" 1 \n");
        Serial.print(LampCfg.sleepenabled);
        Serial.print(" 1 \n");
        Serial.print(LampCfg.alarmenabled);
        Serial.print(" 1 \n");
        Serial.print(LampCfg.buzzerenabled);
        Serial.print(" 1 \n");
        Serial.print(LampCfg.alarmtime);
        Serial.print(" 1 \n");
        Serial.print(LampCfg.sleeptime);
        Serial.print(" 1 \n");
        Serial.print(LampCfg.NetworkSSID);
        Serial.print(" 1 \n");
        Serial.print(LampCfg.NetworkPassword);
        Serial.print(" 1 \n");
        delete[] jsonString;
    }
    else
    {
        debugf("Il file di configurazione non esiste");
        LampCfg.NetworkSSID = WIFI_SSID;
        LampCfg.NetworkPassword = WIFI_PWD;
    }
}

void saveConfig()
{
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["lamp"] = LampCfg.lamp;

    root["sleepenabled"]  = LampCfg.sleepenabled;
    root["alarmenabled"]  = LampCfg.alarmenabled;
    root["buzzerenabled"] = LampCfg.buzzerenabled;
    root["powered"] = LampCfg.powered;

    root["alarmtime"] = LampCfg.alarmtime.c_str();
    root["sleeptime"] = LampCfg.sleeptime.c_str();

    root["ssid"] = LampCfg.NetworkSSID.c_str();
    root["password"] = LampCfg.NetworkPassword.c_str();


    char buf[3096];
    root.prettyPrintTo(buf, sizeof(buf));
    fileSetContent(LAMP_CONFIG_FILE, buf);
//    root.printTo(Serial);
}


