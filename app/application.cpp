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
#include <libraries/Adafruit_ST7735_AS/Adafruit_ST7735_AS.h>
//#include "../include/FreeSansBold9pt7b.h"
//#include "../include/DSEG7ClassicRegular18pt7b.h"
#include <sming/core/SystemClock.h>

#define TYPEOFBOARD 00

    int8_t Old_Hour;
    int8_t Old_Minute;
    int8_t Old_Second;
    int16_t Old_Milliseconds;
    int8_t Old_Day;
    int8_t Old_DayofWeek; // Sunday is day 0
    int8_t Old_Month; // Jan is month 0
    int16_t Old_Year;  // Full Year numer



/*
 * Hardware SPI mode:
 * GND      (GND)         GND
 * VCC      (VCC)         3.3v
 * D0       (CLK)         GPIO14   D5
 * D1       (MOSI)        GPIO13   D7
 * RES      (RESET)       GPIO16   D0
 * DC       (DC)          GPIO0    D3
 * CS       (CS)          GPIO2    D4
 */
    
#define TFT_SCLK 	14 /* D5 */
#define TFT_MOSI 	13 /* D7 */
#define TFT_RST  	16 /* D0 */
#define	TFT_DC   	15 /* D8 */ // 15 8  
#define TFT_CS   	4  /* D2 */ //  4 2

#define PIN_BUTTON      5  /* D1 */ //  5 1
#define PIN_PWM         2  /* D4 */ //  2 4
#define PIN_BUZZER      0  /* D3 */ //  0 3

#if LCD == 0
// Do not need the board's setup 
#else
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_ST7735_AS tft = Adafruit_ST7735_AS(TFT_CS, TFT_DC, TFT_RST);
#endif


#define MAX_READ 101

#define JSON_HOST "192.168.1.120"
#define JSON_PORT 5008

Timer ButtonTimer;
Timer TimerLed;
Timer CronTimer;
DateTime ShowMyTime;

int inter0;
int verso;
int32_t read0, last0;
String StrTime;
uint32_t startTime;

LampConfig LampCfg;
LampMessage LampMsg;

uint8_t pins[1] = {PIN_PWM}; // List of pins that you want to connect to pwm
HardwarePWM HW_pwm(pins, 1);
void onNtpReceive(NtpClient& client, time_t timestamp);

#if LCD == 3
FTPServer ftp;
#endif

//
// TimedCommand 1 : PowerOff
// TimedCommand 2 : Alarm
// TimedCommand 3 : Buzzer
//


TimedCommand command1("0","00.*.*.*.*.*","POWER_OFF","PIN_PWM");
TimedCommand command2("0","00.*.*.*.*.*","ALARM_ON","0");
TimedCommand command3("0","00.*.*.*.*.*","ON","5");


// create an array of timed commands
TimedCommand *tCommands[] = {
    &command1,
    &command2,
    &command3
};
        
// create a cron object and pass it the array of timed commands
// and the count of timed commands
Cron cron(tCommands,3);

// Forward declarations
void onMessageReceived(String topic, String message);
//void publishMessage(int evento, int linea, int tempo);
void sendData();
void setpwn(int led0);

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


        client.sendString("POST /json HTTP/1.1\r\n");
        client.sendString("Accept: */*\r\n");
        client.sendString("Content-Type: application/json;charset=utf-8\r\n");
        client.sendString("Content-Length: "+String(MsgToSend.measureLength()+1)+"\r\n");
        client.sendString("\r\n");
        char buf[MsgToSend.measureLength()];
        MsgToSend.printTo(buf, sizeof(buf)+1);
        client.sendString(buf);
        MsgToSend.printTo(Serial);
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


void check_button0() {
  read0 = micros();
  if (cron.setAlarm==false and digitalRead(PIN_BUTTON)==1 ) {
        if (inter0==true) {   
            if (last0 < read0-40000) 
            {
                last0=read0;
                LampCfg.lamp=LampCfg.lamp+2*verso;
                if (LampCfg.lamp<0) {LampCfg.lamp=0;}
                if (LampCfg.lamp>MAX_READ-1) {LampCfg.lamp=MAX_READ-1; }
                setpwn(LampCfg.lamp);
                Serial.printf("Lamp %d\n",LampCfg.lamp);
            }
        }
   
    }
}

void IRAM_ATTR interruptHandler01();

void IRAM_ATTR interruptHandler00()
{
    inter0=true;
    Serial.print("Press button\n");
    detachInterrupt(PIN_BUTTON);
    attachInterrupt(PIN_BUTTON, interruptHandler01, FALLING);
}

void IRAM_ATTR interruptHandler01()
{
    inter0=false;
    cron.setAlarm=false;
    cron.setPower=false;
    cron.setBuzzer=false;
    cron.AlarmSeconds=1;
    verso=verso*-1;
    Serial.print("Release button\n");
    detachInterrupt(PIN_BUTTON);
    attachInterrupt(PIN_BUTTON, interruptHandler00, RISING);
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
    HW_pwm.analogWrite(PIN_PWM, led0*10);
}

#if LCD==3
void startFTP()
{
        if (!fileExist("index.html"))
                fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

        // Start FTP server
        ftp.listen(21);
        ftp.addUser("me", "123"); // FTP account
}
#endif

void draw_clock(void) {
    int8_t Hour;
    int8_t Minute;
    int8_t Second;
    int16_t Milliseconds;
    int8_t Day;
    int8_t DayofWeek; // Sunday is day 0
    int8_t Month; // Jan is month 0
    int16_t Year;  // Full Year numer


    ShowMyTime.convertFromUnixTime(SystemClock.now(),&Second,&Minute,&Hour,&Day,&DayofWeek,&Month,&Year);
#if LCD == 0
    DisplayTime(Hour, Minute, Second);
#else    
//    tft.fillScreen(ST7735_BLACK);
//    tft.setFont(&DSEG7ClassicRegular18pt7b);
    tft.setTextSize(4);
    tft.setTextColor(ST7735_BLACK);
    tft.setCursor(0, 30);
    tft.print(Old_Hour);
    tft.print(":");
    tft.print(Old_Minute);
    
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(0, 30);
    tft.print(Hour);
    tft.print(":");
    tft.print(Minute);
    tft.setTextSize(1);
//    tft.print(":");
//    tft.print(Second);
//    tft.setFont(&FreeSansBold9pt7b);
    tft.setCursor(0,50);
    tft.setTextSize(2);
    tft.setCursor(0,70);
    tft.setTextColor(ST7735_BLACK);
    tft.print("Lunedi");
    tft.print(Old_DayofWeek);
    tft.setCursor(0,70);
    tft.setTextColor(ST7735_RED);
    tft.print("Lunedi");
    tft.print(DayofWeek);

    tft.setCursor(0,90);
    tft.setTextColor(ST7735_BLACK);
    tft.print(Old_Day);
    tft.print(Old_Month);
    tft.print(Old_Year);
    tft.setCursor(0,90);
    tft.setTextColor(ST7735_GREEN);
    tft.print(Day);
    tft.print(Month);
    tft.print(Year);
#endif    
    Old_Hour=Hour;
    Old_Minute=Minute;
    Old_DayofWeek=DayofWeek;
    Old_Day=Day;
    Old_Month=Month;
    Old_Year=Year;
}

void CronLoop()
{
//    debugf("CronLoop");
    cron.loop();
    if (cron.setAlarm==true) {
        Serial.print("Buongiorno \n");
        TimerLed.initializeMs(500, flashaled).start();
        cron.setAlarm=false;
    }
//    else {
//        TimerLed.initializeMs(500, flashaled).stop();
//        setpwn(LampCfg.lamp);
//    }
    if (cron.setBuzzer==true) {
        TimerLed.initializeMs(500, flashaled).start();
    }
    if (cron.setPower==true) {
        Serial.print("Buonanotte \n");
//        LampCfg.lamp=0;
        verso=1;
//        setpwn(LampCfg.lamp);
        setpwn(0);
        cron.setPower=false;
    }
    if (cron.AlarmSeconds>0)
    {
        cron.AlarmSeconds--;
        if (cron.AlarmSeconds=0) {TimerLed.initializeMs(500, flashaled).stop();}
    }
//    if (cron.AlarmSeconds=0) {TimerLed.initializeMs(500, flashaled).stop();}
    draw_clock();
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
    Serial.print("Local Time : ");
    Serial.println(SystemClock.getSystemTimeString());
    Serial.print("UTC Time   : ");
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
 //   Timer0.initializeMs(50, check_button0).start();

    debugf("connected");

    // At first run we will download web server content
    if (!fileExist("index.html") || !fileExist("config.html") || !fileExist("bootstrap.css.gz") || !fileExist("jquery.js.gz"))
        downloadContentFiles();
    else
        startWebServer();
#if LCD == 3
    startFTP();
#endif
    //flashled.initializeMs(500, flashaled).stop();
    //CronTimer.initializeMs(1000, CronLoop).start();
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
    spiffs_mount(); // Mount file system, in order to work with files
    Serial.begin(SERIAL_BAUD_RATE); // 115200 or 9600 by default
    delay(3000);
    inter0=false;
    cron.setAlarm=false;
    cron.setPower=false;
    cron.setBuzzer=false;
    cron.AlarmSeconds=0;
    cron.PrintJobs();
    verso=1;

#if LCD == 0
    LcdInitialise();
    LcdClear();
#else
    tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab
    tft.fillScreen(ST7735_BLACK);
    draw_clock();
#endif
    // set timezone hourly difference to UTC
    SystemClock.setTimeZone(1);   
    loadConfig();
//    pinMode(1,OUTPUT);
    pinMode(PIN_BUTTON, INPUT);
    attachInterrupt(PIN_BUTTON, interruptHandler00, RISING);
    CronTimer.initializeMs(1000, CronLoop).start();
    ButtonTimer.initializeMs(50, check_button0).start(); 
        
    WifiStation.enable(true);
    WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here
    WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
    setpwn(LampCfg.lamp);
}
