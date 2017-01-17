// #include <user_config.h>
#include <SmingCore.h>

#include "../include/configuration.h"
#include <libraries/CronLibrary/Cron.h>

HttpServer server;

bool serverStarted = false;
extern String alarmtime, powertime;
extern int powerenabled,alarmenabled, buzzerenabled;
extern void setpwn(int led0);
extern LampConfig LampCfg;
extern LampMessage LampMsg;
extern void sendData();
extern Cron cron;

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
    vars["powertime"] = LampCfg.powertime;
    vars["powerenabled"] = LampCfg.powerenabled;
    vars["alarmenabled"] = LampCfg.alarmenabled;
    vars["buzzerenabled"] = LampCfg.buzzerenabled;
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
    vars["SSID"] = LampCfg.NetworkSSID;
    vars["lamp"] = LampCfg.lamp;
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
    sensors["poweroff enabled"] = LampCfg.powerenabled;
    sensors["poweroff time"] = LampCfg.powertime;
    sensors["alarm enabled"] = LampCfg.alarmenabled;
    sensors["alarm time"] = LampCfg.alarmtime;
    sensors["buzzer enabled"] = LampCfg.buzzerenabled;
    response.sendJsonObject(stream);
}


void onApiPower(HttpRequest &request, HttpResponse &response)
{
    LampCfg.powertime = (String)request.getPostParameter("time");
    LampCfg.powerenabled = request.getPostParameter("state","-1").toInt();
    LampMsg.evento = POWER;
    LampMsg.elemento = Lampada_radiocontrollata;
    LampMsg.stato = LampCfg.powerenabled;
    LampMsg.valore = 60*LampCfg.powertime.substring(0,2).toInt()+LampCfg.powertime.substring(3,5).toInt();
    cron.UpdateCommand(0,LampCfg.powerenabled,"00."+LampCfg.powertime.substring(3,5)+"."+LampCfg.powertime.substring(0,2)+".*.*.*");
    saveConfig();
    sendData();
//    debugf("Update onApiPower");
}

void onApiAlarm(HttpRequest &request, HttpResponse &response)
{
    LampCfg.alarmtime = (String)request.getPostParameter("time");
    LampCfg.alarmenabled = request.getPostParameter("state","-1").toInt();
    LampMsg.evento = ALARM;
    LampMsg.elemento = Lampada_radiocontrollata;
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
    LampMsg.elemento = Lampada_radiocontrollata;
    LampMsg.stato = LampCfg.buzzerenabled;
    LampMsg.valore = 60*LampCfg.alarmtime.substring(0,2).toInt()+LampCfg.alarmtime.substring(3,5).toInt();   
    cron.UpdateCommand(2,LampCfg.buzzerenabled,"00."+LampCfg.alarmtime.substring(3,5)+"."+LampCfg.alarmtime.substring(0,2)+".*.*.*");
    saveConfig();
    sendData();
//    debugf("Update onApiBuzzer");
}


void onApiOutput(HttpRequest &request, HttpResponse &response)
{
        int val = request.getPostParameter("status", "-1").toInt();
        int led = request.getPostParameter("led", "-1").toInt();
        if (val == 0 || val == 1)
        {    
            if (led == 0)
                LampCfg.lamp=val;
        }
        else
                val = -1;
        JsonObjectStream* stream = new JsonObjectStream();
        JsonObject& json = stream->getRoot();
        json["status"] = val != -1;
        if (val == -1) json["error"] = "Wrong control parameter value, please use: ?control=0|1";
        response.sendJsonObject(stream);
}        

void onReboot(HttpRequest &request, HttpResponse &response)
{
    System.restart();
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
        LampMsg.elemento = Lampada_radiocontrollata;
        LampMsg.stato = stato;
        LampMsg.valore = LampCfg.lamp;
        saveConfig();
        sendData();
        debugf("Update config onSet");
        
    }
}

void startWebServer()
{
    if (serverStarted) return;

    server.listen(80);
    server.addPath("/", onIndex);
    server.addPath("/api", onApiDoc);
    server.addPath("/api/status", onApiStatus);
    server.addPath("/api/set", onApiSet);
    server.addPath("/api/off", onApiPower);
    server.addPath("/api/alarm", onApiAlarm);
    server.addPath("/api/buzzer", onApiBuzzer);
    server.addPath("/api/output", onApiOutput);    
//    server.addPath("/config", onConfiguration);
    server.addPath("/reboot", onReboot);
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
            downloadClient.downloadFile("http://server/templates/LightControl/LightControl.html", "index.html");
    else if (dowfid == 1)
            downloadClient.downloadFile("http://server/templates/LightControl/LightConfig.html", "config.html");
    else
    {
            // Content download was completed
            downloadTimer.stop();
            startWebServer();

    }
}
