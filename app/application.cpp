// #include <user_config.h>
#include <SmingCore.h>

///////////////////////////////////////////////////////////////////
// Set your SSID & Pass for initial configuration
#include "../include/configuration.h" // application configuration
///////////////////////////////////////////////////////////////////
// wget "http://192.168.1.220/api/output?status=1&led=2" -o pippo
///////////////////////////////////////////////////////////////////
#include "special_chars.h"
#include "webserver.h"
#include <libraries/CronLibrary/TimedCommand.h>
#include <libraries/CronLibrary/Cron.h>
#include "NtpClientDelegateDemo.h"
#include "../include/configuration.h"

#define TYPEOFBOARD 00

uint8_t pins[1] = {15}; // List of pins that you want to connect to pwm
HardwarePWM HW_pwm(pins, 1);
void onNtpReceive(NtpClient& client, time_t timestamp);

#define MAX_READ 101

#define JSON_HOST "192.168.1.120"
#define JSON_PORT 5008

Timer Timer0;
Timer flashled;
Timer CronTimer;

int inter0;
int verso;
int32_t read0, last0;
String StrTime;

LampConfig LampCfg;
LampMessage LampMsg;


FTPServer ftp;
     
//
// TimedCommand 1 : PowerOff
// TimedCommand 2 : Alarm
// TimedCommand 3 : Buzzer
//


TimedCommand command1("0","00.*.*.*.*.*","POWER_OFF","0");
TimedCommand command2("0","00.*.*.*.*.*","ON","12");
TimedCommand command3("0","00.*.*.*.*.*","ON","13");

/*
TimedCommand command4("0","15.*.*.*.*.*","OFF","12");

TimedCommand command5("0","20.*.*.*.*.*","ON","13");
TimedCommand command6("0","25.*.*.*.*.*","OFF","13");

TimedCommand command7("0","30.*.*.*.*.*","ON","12");
TimedCommand command8("0","35.*.*.*.*.*","OFF","12");

TimedCommand command9("0","40.*.*.*.*.*","ON","13");
TimedCommand command10("0","45.*.*.*.*.*","OFF","13");

TimedCommand command11("0","50.*.*.*.*.*","ON","12");
TimedCommand command12("0","55.*.*.*.*.*","OFF","12");

TimedCommand command13("0","0.*.*.*.*.*","ALARM_ON","15");
TimedCommand command14("0","30.*.*.*.*.*","ALARM_ON","15");
*/



// create an array of timed commands
TimedCommand *tCommands[] = {
    &command1,
    &command2,
    &command3
/*
    &command4,

    &command5,
    &command6,

    &command7,
    &command8,

    &command9,
    &command10,

    &command11,
    &command12,

    &command13,
    &command14
*/
};
        
// create a cron object and pass it the array of timed commands
// and the count of timed commands
Cron cron(tCommands,3);

// Forward declarations
void onMessageReceived(String topic, String message);
//void publishMessage(int evento, int linea, int tempo);
void sendData();
void setpwn(int led0);

// Publish our message
//void publishMessage(String string0, int led, int numero)
//void publishMessage(int evento, int linea, int tempo)
//void publishMessage()
//{
//    String string0 = WifiStation.getIP().toString().c_str();
//    String String0 = LampMsg.evento;
//    String String1 = LampMsg.stato;

//    String2="{ \"sensorid\":"+String1+"";
//    String2=String2+" , \"basetta\":\""+string0+"\"";

//    if (LampMsg.evento !=-1)
//    {
//        String2=String2+" , \"count\":"+String0;
//    }
//    String2=String2+" }";

//    Serial.println(String2);    
//    sendData();
//}

void JsonOnComplete(TcpClient& client, bool successful)
{
    // debug msg
    debugf("JsonOnComplete");
    debugf("successful: %d", successful);
    client.close();
}

void JsonOnReadyToSend(TcpClient& client, TcpConnectionEvent sourceEvent)
{
    // debug msg
    debugf("JsonOnReadyToSend");
    debugf("sourceEvent: %d", sourceEvent);

    // The connection is made at the time of shipment
    if(sourceEvent == eTCE_Connected)
    {
        DynamicJsonBuffer jsonBuffer;
        JsonObject& MsgToSend = jsonBuffer.createObject();

        MsgToSend["basetta"] = WifiStation.getIP().toString();
        MsgToSend["elemento"] = LampMsg.elemento;
        MsgToSend["evento"]  = LampMsg.evento;
        MsgToSend["stato"]   = LampMsg.stato;
        MsgToSend["valore"]  = LampMsg.valore;
//        MsgToSend["tempo"]  = LampMsg.tempo;


        client.sendString("POST /json HTTP/1.1\r\n");
        client.sendString("Accept: */*\r\n");
        client.sendString("Content-Type: application/json;charset=utf-8\r\n");
        client.sendString("Content-Length: "+String(MsgToSend.measureLength()+1)+"\r\n");
        client.sendString("\r\n");
        //MsgToSend.prettyPrintTo(client,MsgToSend.measureLength());
        //json.printTo(client); // very slow ??client.sendString("\r\n");
//        client.sendString(MsgToSend);
        char buf[MsgToSend.measureLength()];
        MsgToSend.printTo(buf, sizeof(buf)+1);
        client.sendString(buf);
        MsgToSend.printTo(Serial);
//        client.println(out);
        client.sendString("\r\n");
    }
}

bool JsonOnReceive(TcpClient& client, char *buf, int size)
{
    // debug msg
    debugf("JsonOnReceive");
    debugf("%s", buf);
    client.close();
}

// Create an object of class JsonMon TcpClient
TcpClient JsonMon(JsonOnComplete, JsonOnReadyToSend, JsonOnReceive);



// This function will be called by timer
void sendData()
{
    // We read the sensor data
    // connect to the server narodmon
    JsonMon.connect(JSON_HOST, JSON_PORT);
}


// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
    Serial.print(topic);
    Serial.print(":\r\n\t"); // Pretify alignment for printing
    Serial.println(message);
}


void check_button0() {
  read0 = micros();
  if (cron.setAlarm==false and digitalRead(INT_PIN0)==0 ) {
        if (inter0==true) {   
 //           debugf("Pressed : direction %d, Led %d, timestamp %d \n",verso,cfg.lamp);
            if (last0 < read0-40000) 
            {

                last0=read0;
                LampCfg.lamp=LampCfg.lamp+2*verso;
                if (LampCfg.lamp<0) {LampCfg.lamp=0;}
                if (LampCfg.lamp>=MAX_READ-1) {LampCfg.lamp=MAX_READ-1; }
//                if (volte0>=MAX_READ-1) {volte0=MAX_READ-1;}
//                debugf("Pressed : direction %d, Led %d\n",verso,cfg.lamp);
                setpwn(LampCfg.lamp);
            }
        }
   
    }
}

void IRAM_ATTR interruptHandler01();

void IRAM_ATTR interruptHandler00()
{
    inter0=true;
//    debugf("Key pressed Value %d",cfg.lamp);
//    debugf("Pressed : interrupt1 %d interrupt2 %d delta %d\n",interrupt1,interrupt2, interrupt1-interrupt2);
//    interrupt2=interrupt1;
//    interrupt1=micros();
    detachInterrupt(INT_PIN0);
    attachInterrupt(INT_PIN0, interruptHandler01, RISING);
}

void IRAM_ATTR interruptHandler01()
{
    inter0=false;
    cron.setAlarm=false;
    cron.setPower=false;
    cron.setBuzzer=false;
//    debugf("Key released Value %d",cfg.lamp);
    verso=verso*-1;
//    debugf("Key released : direction %d, Led %d\n",verso,cfg.lamp);
    detachInterrupt(INT_PIN0);
    attachInterrupt(INT_PIN0, interruptHandler00, FALLING);
}        


////// WEB Clock //////
Timer clockRefresher;
HttpClient clockWebClient;
uint32_t lastClockUpdate = 0;
DateTime clockValue;
const int clockUpdateIntervalMs = 10 * 60 * 1000; // Update web clock every 10 minutes

void onClockUpdating(HttpClient& client, bool successful)
{
    if (!successful)
    {
            debugf("CLOCK UPDATE FAILED %d (code: %d)", successful, client.getResponseCode());
            lastClockUpdate = 0;
            return;
    }

    // Extract date header from response
    clockValue = client.getServerDate();
    if (clockValue.isNull()) clockValue = client.getLastModifiedDate();
}

void refreshClockTime()
{
    uint32_t nowClock = millis();
    if (nowClock < lastClockUpdate) lastClockUpdate = 0; // Prevent overflow, restart
    if ((lastClockUpdate == 0 || nowClock - lastClockUpdate > clockUpdateIntervalMs) && !clockWebClient.isProcessing())
    {
            clockWebClient.downloadString("google.com", onClockUpdating);
            lastClockUpdate = nowClock;
    }
    else if (!clockValue.isNull())
            clockValue.addMilliseconds(clockRefresher.getIntervalMs());

    if (!clockValue.isNull())
    {
            StrTime = clockValue.toShortDateString() + " " + clockValue.toShortTimeString(false);

            if ((millis() % 2000) > 1000)
                    StrTime.setCharAt(13, ' ');
            else
                    StrTime.setCharAt(13, ':');
    }
}

void startWebClock()
{
    lastClockUpdate = 0;
    clockRefresher.stop();
    clockRefresher.initializeMs(500, refreshClockTime).start();
}


void setpwn(int led0)
{
    HW_pwm.analogWrite(INT_PIN15, led0*10);
}


void startFTP()
{
        if (!fileExist("index.html"))
                fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

        // Start FTP server
        ftp.listen(21);
        ftp.addUser("me", "123"); // FTP account
}


void flashaled()
{
        setpwn(100);
        delay(50);
        setpwn(0);
        delay(50);
        setpwn(100);
        delay(50);    
        setpwn(0);
        delay(50);
        setpwn(100);
        delay(50);    
        setpwn(0);
        delay(50);
}

void CronLoop()
{
//    debugf("CronLoop");
    cron.loop();
    if (cron.setAlarm==true) {
        flashled.initializeMs(500, flashaled).start();
    }
    else {
        flashled.initializeMs(500, flashaled).stop();
        setpwn(LampCfg.lamp);
    }
    if (cron.setBuzzer==true) {
        flashled.initializeMs(500, flashaled).start();
    }
    if (cron.setPower==true) {
        LampCfg.lamp=0;
        verso=1;
        setpwn(LampCfg.lamp);
        cron.setPower=false;
    }
}

void SendPresence()
{
//    publishMessage(WifiStation.getIP().toString().c_str(),9999,-1);
    LampMsg.evento=SEND_PRESENCE;
    LampMsg.elemento=Lampada_radiocontrollata;
    sendData();
}
// Callback example using defined class ntpClientDemo
ntpClientDemo *demo;

// CallBack example 1 
// ntpClientDemo dm1 = ntpClientDemo();
// or use 
// ntpClientDemo dm1;

void onPrintSystemTime() {
    Serial.print("Local Time    : ");
    Serial.println(SystemClock.getSystemTimeString());
    Serial.print("UTC Time: ");
    Serial.println(SystemClock.getSystemTimeString(eTZ_UTC));
}


// Called when time has been received by NtpClient (option 1 or 2)
// Either after manual requestTime() or when
// and automatic request has been made.
void onNtpReceive(NtpClient& client, time_t timestamp) {
    SystemClock.setTime(timestamp);

    Serial.print("Time synchronized: ");
    Serial.println(SystemClock.getSystemTimeString());
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
    debugf("I'm CONNECTED");
    Serial.println(WifiStation.getIP().toString());
    Timer0.initializeMs(50, check_button0).start();

    debugf("connected");

    // At first run we will download web server content
    if (!fileExist("index.html") || !fileExist("config.html") || !fileExist("bootstrap.css.gz") || !fileExist("jquery.js.gz"))
        downloadContentFiles();
    else
        startWebServer();
    startFTP();
    flashled.initializeMs(500, flashaled).stop();
    CronTimer.initializeMs(1000, CronLoop).start();
    SendPresence();
    demo = new ntpClientDemo();
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
    debugf("I'm NOT CONNECTED!");
    WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

void init()
{
//    HW_pwm.restart();

    flashled.initializeMs(500, flashaled).start();
    spiffs_mount(); // Mount file system, in order to work with files
    Serial.begin(SERIAL_BAUD_RATE); // 115200 or 9600 by default
    delay(3000);
    inter0=false;
    cron.setAlarm=false;
    cron.setPower=false;
    cron.setBuzzer=false;
    cron.PrintJobs();
    verso=1;
    loadConfig();
    // set timezone hourly difference to UTC
    SystemClock.setTimeZone(1);   
//    Serial.println(cron(tCommands[1]));
    // Station - WiFi client
    attachInterrupt(INT_PIN0, interruptHandler00, FALLING);
    WifiStation.enable(true);
    WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here
    WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
    setpwn(LampCfg.lamp);
}
