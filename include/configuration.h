#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

// #include <user_config.h>
#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
        #define WIFI_SSID "myhotspot" // Put you SSID and Password here
        #define WIFI_PWD "asdfgh1234"
#endif

#define LAMP_CONFIG_FILE ".lamp.conf" // leading point for security reasons :)

#define INT_PIN0 0   // GPIO0 d3
#define INT_PIN2 2   // GPIO2 d4
#define INT_PIN4 4   // GPIO4 d2
#define INT_PIN5 5   // GPIO5 d1
#define INT_PIN14 14 // GPIO14 d5
#define INT_PIN12 12 // GPIO12 d6
#define INT_PIN13 13 // GPIO13 d7
#define INT_PIN15 15 // GPIO15 d8

#define say(a) ( Serial.print(a) )

#define LIGHT 1
#define ALARM 2
#define POWER 3
#define BUZZER 4
#define TEMPERATURE 5
#define SWITCH 6
#define PIR 7
#define AMPERE 8
#define xxx 9
#define yyy 10

#define Lampada_radiocontrollata 1
#define Pulsantiera_4_posti 2
#define Rivelatore_temperatura 3
#define Cronotermostato 4

#define SEND_PRESENCE 9999
#define CONFIGURATION 9998

struct LampConfig
{
	LampConfig()
	{
            lamp=50;
            alarmtime="09:30";
            powertime="23:30";
            powerenabled=0;
            alarmenabled=0;
            buzzerenabled=0;
	}

	String NetworkSSID;
	String NetworkPassword;
 
        String alarmtime, powertime;
        int lamp, powerenabled,alarmenabled, buzzerenabled;
        
};

struct LampMessage
{
    LampMessage()
    {
        elemento=0;
        evento=0;
        stato=0;
        valore=0;
    }
    int evento, stato, valore, elemento;
};

void loadConfig();
void saveConfig();
extern void startWebClock();
extern void flashleds();
extern LampConfig ActiveConfig;
extern void publishMessage(int evento, int linea, int tempo);

#endif /* INCLUDE_CONFIGURATION_H_ */
