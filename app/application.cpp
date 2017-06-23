#include <user_config.h>
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
#include <libraries/Timezone/Timezone.h>
/* // https://github.com/JChristensen/Timezone */

#define TYPEOFBOARD 00

    int8_t Old_Hour;
    int8_t Old_Minute;
    int8_t Old_Second;
    int16_t Old_Milliseconds;
    int8_t Old_Day;
    int8_t Old_DayofWeek; // Sunday is day 0
    int8_t Old_Month; // Jan is month 0
    int16_t Old_Year;  // Full Year numer



#if LCD == 0
// Do not need the board's setup 
#else
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_ST7735_AS tft = Adafruit_ST7735_AS(TFT_CS, TFT_DC, TFT_RST);
#endif


#define MAX_READ 101

#define JSON_HOST "10.42.0.1"
#define JSON_PORT 5008

Timer ButtonTimer;
Timer TimerLed;
Timer CronTimer;
DateTime ShowMyTime;

int inter0;
int verso, data_to_send;
// int32_t read0, last0;
String StrTime;
uint32_t startTime;

LampConfig LampCfg;
LampMessage LampMsg;

uint8_t pins[1] = {PIN_PWM}; // List of pins that you want to connect to pwm
HardwarePWM HW_pwm(pins, 1);
void onNtpReceive(NtpClient& client, time_t timestamp);
void DisplayTimeClock(uint8_t hour, uint8_t minutes, uint8_t seconds);
void DisplayDateClock(uint8_t day, uint8_t month, uint8_t year);

//Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone CE(CEST, CET);
TimeChangeRule *tcr;        //pointer to the time change rule, use to get the TZ abbrev


#if LCD == 3
FTPServer ftp;
#endif

//
// TimedCommand 1 : PowerOff
// TimedCommand 2 : AlarmOn
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

        MsgToSend["ip"]        = WifiStation.getIP().toString();
        
        MsgToSend["pulsante"]  = LampMsg.pulsante;
        MsgToSend["evento"]    = LampMsg.evento;
        MsgToSend["stato"]     = LampMsg.stato;
        MsgToSend["valore"]    = LampMsg.valore;

//        MsgToSend["slenabled"] = LampCfg.sleepenabled;
//        MsgToSend["alenabled"] = LampCfg.alarmenabled;
//        MsgToSend["buenabled"] = LampCfg.buzzerenabled;
//        MsgToSend["alarmtime"] = LampCfg.alarmtime;
//        MsgToSend["sleeptime"] = LampCfg.sleeptime;
//        MsgToSend["powered"]   = LampCfg.powered;
        
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
//  read0 = micros();
//  if (cron.setAlarm==false and digitalRead(PIN_BUTTON)==1 ) {
        if (inter0==true) {   
//            if (last0 < read0-40000) 
//            {
//                last0=read0;
                LampCfg.lamp=LampCfg.lamp+2*verso;
                if (LampCfg.lamp<0) {LampCfg.lamp=0;}
                if (LampCfg.lamp>MAX_READ-1) {LampCfg.lamp=MAX_READ-1; }
                setpwn(LampCfg.lamp);
                Serial.printf("Lamp %d\n",LampCfg.lamp);
//            }
        }
     if (data_to_send==1) {
        LampMsg.evento = LIGHT;
        LampMsg.stato=Lampada_radiocontrollata;
        LampMsg.valore=LampCfg.lamp;
        LampMsg.pulsante=0;
        saveConfig();
        sendData();
        data_to_send=0;    
    }
  
//    }
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
    data_to_send=1;
}        



void setpwn(int led0)
{
    HW_pwm.analogWrite(PIN_PWM, led0*10*LampCfg.powered);
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
    ShowMyTime.convertFromUnixTime(CE.toLocal(SystemClock.now(), &tcr),&Second,&Minute,&Hour,&Day,&DayofWeek,&Month,&Year);
#if LCD == 0
    DisplayTimeClock(Hour, Minute, Second);
    DisplayDateClock(Day, Month, Year);
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
    cron.loop();
    if (cron.setAlarm==true) {
//        Serial.print("Buongiorno \n");
        TimerLed.initializeMs(500, flashaled).start();
        cron.setAlarm=false;
    }
    if (cron.AlarmSeconds>0)
    {
        cron.AlarmSeconds--;
        if (cron.AlarmSeconds==0) {
            TimerLed.initializeMs(500, flashaled).stop(); 
            setpwn(LampCfg.lamp);
        }
    }
    if (cron.setBuzzer==true) {
        TimerLed.initializeMs(500, flashaled).start();
        cron.setBuzzer=false;
    }
    if (cron.setPower==true) {
//        Serial.print("Buonanotte \n");
        verso=1;
        setpwn(0);
        cron.setPower=false;
    }
    draw_clock();
}

void SendPresence()
{
//    publishMessage(WifiStation.getIP().toString().c_str(),9999,-1);
    LampMsg.evento=SEND_PRESENCE;
    LampMsg.stato=Lampada_radiocontrollata;
    LampMsg.valore=LampCfg.lamp;
    LampMsg.pulsante=0;
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
	debugf("connection FAILED");
	WifiAccessPoint.config("LampConfig", "", AUTH_OPEN);
	WifiAccessPoint.enable(true);
	// Stop main screen output
//	procTimer.stop();
//	displayTimer.stop();
//	lcd.clear();

//	lcd.setCursor(0,0);
//	lcd.print("WiFi LampConfig");
//	lcd.setCursor(0,1);
//	lcd.print("  ");
//	lcd.print(WifiAccessPoint.getIP());

	startWebServer();
	WifiStation.waitConnection(connectOk); // Wait connection
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
    data_to_send=0;

#if LCD == 0
    LcdInitialise();
    LcdClear();
#else
    tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab
    tft.fillScreen(ST7735_BLACK);
    draw_clock();
#endif
    // set timezone hourly difference to UTC
    SystemClock.setTimeZone(0);   
    loadConfig();
//    pinMode(1,OUTPUT);
    pinMode(PIN_BUTTON, INPUT);
    attachInterrupt(PIN_BUTTON, interruptHandler00, RISING);
    CronTimer.initializeMs(1000, CronLoop).start();
    ButtonTimer.initializeMs(50, check_button0).start(); 

    WifiStation.enable(true);
    WifiAccessPoint.enable(false);
    WifiStation.config(LampCfg.NetworkSSID, LampCfg.NetworkPassword);

    WifiStation.waitConnection(connectOk, 10, connectFail); // We recommend 20+ seconds at start
    setpwn(LampCfg.lamp);
}
