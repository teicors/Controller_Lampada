#include <user_config.h>
#include <SmingCore.h>

#include "../include/configuration.h"
#include <libraries/CronLibrary/Cron.h>

HttpServer server;

bool serverStarted = false;
extern String alarmtime, sleeptime;
extern int sleepenabled,alarmenabled, buzzerenabled, powered;
extern void setpwn(int led0);
extern LampConfig LampCfg;
extern LampMessage LampMsg;
extern void sendData();
extern Cron cron;
void downloadContentFiles();


// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif


void onIndex(HttpRequest &request, HttpResponse &response)
{
    TemplateFileStream *tmpl = new TemplateFileStream("index.html");
    auto &vars = tmpl->variables();
    vars["lamp"] = LampCfg.lamp;
    vars["alarmtime"] = LampCfg.alarmtime;
    vars["sleeptime"] = LampCfg.sleeptime;
    vars["sleepenabled"] = LampCfg.sleepenabled;
    vars["alarmenabled"] = LampCfg.alarmenabled;
    vars["buzzerenabled"] = LampCfg.buzzerenabled;
    vars["powered"]= LampCfg.powered;
    response.setAllowCrossDomainOrigin("*");
    response.sendTemplate(tmpl);
}

void onConfiguration(HttpRequest &request, HttpResponse &response)
{
//    LampConfig cfg = loadConfig();
    loadConfig();
    if (request.getRequestMethod() == RequestMethod::POST)
    {
            debugf("Update config");
            // Update config
            if (request.getPostParameter("SSID").length() > 0) // Network
            {
                    LampCfg.NetworkSSID = request.getPostParameter("SSID");
                    LampCfg.NetworkPassword = request.getPostParameter("Password");
            }
            saveConfig();
//            startWebClock(); // Apply time zone settings
            response.redirect();
    }

    debugf("Send template");
    TemplateFileStream *tmpl = new TemplateFileStream("config.html");
    auto &vars = tmpl->variables();
    vars["alarmtime"] = 60*LampCfg.alarmtime.substring(0,2).toInt()+LampCfg.alarmtime.substring(3,5).toInt();
    vars["sleeptime"] = 60*LampCfg.sleeptime.substring(0,2).toInt()+LampCfg.sleeptime.substring(3,5).toInt();
    vars["SSID"] = LampCfg.NetworkSSID;
    vars["lamp"] = LampCfg.lamp;
    response.setAllowCrossDomainOrigin("*");
    response.sendTemplate(tmpl);
}

void onSetup(HttpRequest &request, HttpResponse &response)
{
//    LampConfig cfg = loadConfig();
    loadConfig();
    if (request.getRequestMethod() == RequestMethod::POST)
    {
            debugf("Update config");
            // Update config
            if (request.getPostParameter("SSID").length() > 0) // Network
            {
                    LampCfg.NetworkSSID = request.getPostParameter("SSID");
                    LampCfg.NetworkPassword = request.getPostParameter("Password");
            }
            saveConfig();
//            startWebClock(); // Apply time zone settings
            response.redirect();
    }

    debugf("Send template");
    TemplateFileStream *tmpl = new TemplateFileStream("setup.html");
    auto &vars = tmpl->variables();
    vars["SSID"] = LampCfg.NetworkSSID;
    vars["lamp"] = LampCfg.lamp;
    response.setAllowCrossDomainOrigin("*");
    response.sendTemplate(tmpl);
}

void onFile(HttpRequest &request, HttpResponse &response)
{
    String file = request.getPath();
    if (file[0] == '/')
            file = file.substring(1);

    if (file[0] == '.')
            response.forbidden();
    else
    {
            response.setCache(86400, true); // It's important to use cache for better performance.
            response.sendFile(file);
    }
}

/// API ///

void onApiDoc(HttpRequest &request, HttpResponse &response)
{
        TemplateFileStream *tmpl = new TemplateFileStream("api.html");
        auto &vars = tmpl->variables();
        vars["IP"] = (WifiStation.isConnected() ? WifiStation.getIP() : WifiAccessPoint.getIP()).toString();
        response.sendTemplate(tmpl);
}

void onApiStatus(HttpRequest &request, HttpResponse &response)
{
    JsonObjectStream* stream = new JsonObjectStream();
    JsonObject& json = stream->getRoot();
    json["status"] = (bool)true;
    JsonObject& sensors = json.createNestedObject("sensors");
    sensors["lamp"] = LampCfg.lamp;
    sensors["poweroff enabled"] = LampCfg.sleepenabled;
    sensors["poweroff time"] = LampCfg.sleeptime;
    sensors["alarm enabled"] = LampCfg.alarmenabled;
    sensors["alarm time"] = LampCfg.alarmtime;
    sensors["buzzer enabled"] = LampCfg.buzzerenabled;
    sensors["power enabled"] = LampCfg.powered;
    response.sendJsonObject(stream);
}

void OnApiShowInfo(HttpRequest &request, HttpResponse &response) {
    Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
    Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    Serial.printf("System Chip ID: %x\r\n", system_get_chip_id());
    Serial.printf("SPI Flash ID: %x\r\n", spi_flash_get_id());
    //Serial.printf("SPI Flash Size: %d\r\n", (1 << ((spi_flash_get_id() >> 16) & 0xff)));
}


void onApiSleep(HttpRequest &request, HttpResponse &response)
{
    LampCfg.sleeptime = (String)request.getPostParameter("time");
    LampCfg.sleepenabled = request.getPostParameter("state","-1").toInt();
    LampMsg.evento = SLEEP;
    LampMsg.pulsante = 0;
    LampMsg.stato = LampCfg.sleepenabled;
    LampMsg.valore = 60*LampCfg.sleeptime.substring(0,2).toInt()+LampCfg.sleeptime.substring(3,5).toInt();
    cron.UpdateCommand(0,LampCfg.sleepenabled,"00."+LampCfg.sleeptime.substring(3,5)+"."+LampCfg.sleeptime.substring(0,2)+".*.*.*");
    saveConfig();
    sendData();
//    debugf("Update onApiSleep");
}

void onApiAlarm(HttpRequest &request, HttpResponse &response)
{
    LampCfg.alarmtime = (String)request.getPostParameter("time");
    LampCfg.alarmenabled = request.getPostParameter("state","-1").toInt();
    LampMsg.evento = ALARM;
    LampMsg.pulsante = 0;
    LampMsg.stato = LampCfg.alarmenabled;
    LampMsg.valore = 60*LampCfg.alarmtime.substring(0,2).toInt()+LampCfg.alarmtime.substring(3,5).toInt();
    cron.UpdateCommand(1,LampCfg.alarmenabled,"00."+LampCfg.alarmtime.substring(3,5)+"."+LampCfg.alarmtime.substring(0,2)+".*.*.*");
    saveConfig();
    sendData();
//    debugf("Update onApiAlarm");
}

void onApiBuzzer(HttpRequest &request, HttpResponse &response)
{
    LampCfg.buzzerenabled = request.getPostParameter("state","-1").toInt();
    LampMsg.evento = BUZZER;
    LampMsg.pulsante = 0;
    LampMsg.stato = LampCfg.buzzerenabled;
    LampMsg.valore = 60*LampCfg.alarmtime.substring(0,2).toInt()+LampCfg.alarmtime.substring(3,5).toInt();   
    cron.UpdateCommand(2,LampCfg.buzzerenabled,"00."+LampCfg.alarmtime.substring(3,5)+"."+LampCfg.alarmtime.substring(0,2)+".*.*.*");
    saveConfig();
    sendData();
//    debugf("Update onApiBuzzer");
}

void onApiPower(HttpRequest &request, HttpResponse &response)
{
    LampCfg.powered = request.getPostParameter("state","-1").toInt();
    LampMsg.evento = POWER;
    LampMsg.pulsante = 0;
    LampMsg.stato = LampCfg.powered;
    setpwn(LampCfg.lamp);
    saveConfig();
    sendData();
//    debugf("Update onSWPower");
}

void onSWPower(HttpRequest &request, HttpResponse &response)
{
    LampCfg.powered = request.getPostParameter("state","-1").toInt();
    LampMsg.evento = POWER;
    LampMsg.pulsante = 0;
    LampMsg.stato = LampCfg.powered;
    setpwn(LampCfg.lamp);
    saveConfig();
    sendData();
//    debugf("Update onSWPower");
}

void onSWAlarm(HttpRequest &request, HttpResponse &response)
{
    LampCfg.alarmenabled = request.getPostParameter("state","-1").toInt();
    LampMsg.evento = ALARM;
    LampMsg.pulsante = 0;
    LampMsg.stato = LampCfg.alarmenabled;
    saveConfig();
    sendData();
//    debugf("Update onSWPower");
}

void onSWBuzzer(HttpRequest &request, HttpResponse &response)
{
    LampCfg.buzzerenabled = request.getPostParameter("state","-1").toInt();
    LampMsg.evento = BUZZER;
    LampMsg.pulsante = 0;
    LampMsg.stato = LampCfg.buzzerenabled;
    saveConfig();
    sendData();
//    debugf("Update onSWPower");
}

void onSWSleep(HttpRequest &request, HttpResponse &response)
{
    LampCfg.sleepenabled = request.getPostParameter("state","-1").toInt();
    LampMsg.evento = SLEEP;
    LampMsg.pulsante = 0;
    LampMsg.stato = LampCfg.sleepenabled;
    saveConfig();
    sendData();
//    debugf("Update onSWPower");
}

void onApiOutput(HttpRequest &request, HttpResponse &response)
{
        int valore = request.getPostParameter("valore", "-1").toInt();
        int pulsante = request.getPostParameter("pulsante", "-1").toInt();
        if (valore == 0 || valore == 1)
        {    
            if (pulsante == 0)
                LampCfg.lamp=valore;
        }
        else
                valore = -1;
        JsonObjectStream* stream = new JsonObjectStream();
        JsonObject& json = stream->getRoot();
        json["status"] = valore != -1;
        if (valore == -1) json["error"] = "Wrong control parameter value, please use: ?control=0|1";
        response.sendJsonObject(stream);
}        

void onReboot(HttpRequest &request, HttpResponse &response)
{
//	rboot_set_current_rom(rboot_get_current_rom());
//	Serial.println("Restarting...\r\n");
        System.restart();
}

void onUpdate(HttpRequest &request, HttpResponse &response)
{
    downloadContentFiles();
}

void onApiSet(HttpRequest &request, HttpResponse &response)
{
    LampCfg.lamp = request.getPostParameter("value","-1").toInt();
    int stato = request.getPostParameter("state","-1").toInt();
    debugf("valore lamp %d, evento %d",LampCfg.lamp, stato);

    setpwn(LampCfg.lamp);

    if (stato ==1)
    {
        LampMsg.evento = LIGHT;
        LampMsg.pulsante = 0;
        LampMsg.stato = stato;
        LampMsg.valore = LampCfg.lamp;
        saveConfig();
        sendData();
        debugf("Update config onSet");
    }
}

void onSetTime(HttpRequest &request, HttpResponse &response)
{
    uint32_t time = request.getPostParameter("time","-1").toInt();
    debugf("valore time %d ", time);

    if (time > 0)
    {
        SystemClock.setTime(time, eTZ_UTC);
        debugf("Update config onSet");     
        Serial.print("ntpClientDemo Callback Time_t = ");
        Serial.print(time);
        Serial.print(" Time = ");
        Serial.println(SystemClock.getSystemTimeString());   
    }
}

void startWebServer()
{
    if (serverStarted) return;

    server.listen(80);
    server.addPath("/", onIndex);
    server.addPath("/update", onUpdate);
    server.addPath("/setup", onSetup);
    server.addPath("/api", onApiDoc);
    server.addPath("/api/status", onApiStatus);
    server.addPath("/api/set", onApiSet);

    server.addPath("/api/sleep", onApiSleep);
    server.addPath("/api/alarm", onApiAlarm);
    server.addPath("/api/buzzer", onApiBuzzer);

    server.addPath("/api/output", onApiOutput);    
    server.addPath("/config", onConfiguration);
    server.addPath("/reboot", onReboot);
    server.addPath("/api/SetTime", onSetTime);
    server.addPath("/api/ShowInfo", OnApiShowInfo);
    
    server.addPath("/api/swsleep",onSWSleep);
    server.addPath("/api/swalarm",onSWAlarm);
    server.addPath("/api/swpower",onSWPower);
    server.addPath("/api/swbuzzer",onSWBuzzer);
    
    server.setDefaultHandler(onFile);
    serverStarted = true;

    if (WifiStation.isEnabled())
            debugf("STA: %s", WifiStation.getIP().toString().c_str());
    if (WifiAccessPoint.isEnabled())
        debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}

/// FileSystem Initialization ///

Timer downloadTimer;
HttpClient downloadClient;
int dowfid = 2;
void downloadContentFiles()
{
    if (!downloadTimer.isStarted())
    {
            downloadTimer.initializeMs(3000, downloadContentFiles).start();
    }

    if (downloadClient.isProcessing()) return; // Please, wait.
    debugf("DownloadContentFiles");

    if (downloadClient.isSuccessful())
            dowfid++; // Success. Go to next file!
    downloadClient.reset(); // Reset current download status

    if (dowfid == 0)
            downloadClient.downloadFile("http://fileserver/templates/LightControl/LightControl.html", "index.html");
    else if (dowfid == 1)
            downloadClient.downloadFile("http://fileserver/templates/LightControl/LightConfig.html", "config.html");
    else
    {
            // Content download was completed
            downloadTimer.stop();
            startWebServer();

    }
}
