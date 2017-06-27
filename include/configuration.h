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
#define SWITCH_CONFIG_FILE ".switch.conf" // leading point for security reasons :)

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
#define SLEEP 5
#define TEMPERATURE 6
#define SWITCH 7
#define PIR 8
#define AMPERE 9

#define SEND_PRESENCE 9999
#define CONFIGURATION 9998

#define Lampada_radiocontrollata 1
#define Pulsantiera_4_posti 2
#define Cronotermostato 3
#define Rivelatore_temperatura 4
#define Rivelatore_movimento 5
#define Rivelatore_luce 6
#define Rivelatore_gas 7
#define Rivelatore_pressione 8
#define Rivelatore_acqua 9
#define Termometro 10
#define Amperometro 10

#define LCD 0
// 0 BN
// 1 COLOR

#define LCD_C     LOW
#define LCD_D     HIGH

struct LampConfig
{
	LampConfig()
	{
            lamp=50;
            alarmtime="09:30";
            sleeptime="23:30";
            sleepenabled=0;
            alarmenabled=0;
            buzzerenabled=0;
            powered=1;
            NetworkSSID="";
            NetworkPassword="";
	}

	String NetworkSSID;
	String NetworkPassword;
 
        String alarmtime, sleeptime;
        int lamp, sleepenabled,alarmenabled, buzzerenabled, powered;
        
};


struct SwitchConfig
{
	SwitchConfig()
	{
            Led0=0;
            Led2=0;
            Led4=0;
            Led5=0;
            powered=0;
            NetworkSSID="";
            NetworkPassword="";
	}

	String NetworkSSID;
	String NetworkPassword;
 
        int Led0, Led2, Led4, Led5, powered;
        
};


struct LampMessage
{
    LampMessage()
    {
        pulsante=0;
        evento=0;
        stato=0;
        valore=0;
    }
    int evento, stato, valore, pulsante;
};

extern void loadConfig();
extern void saveConfig();
extern void startWebClock();
extern void flashleds();
extern LampConfig ActiveConfig;
extern void publishMessage(int evento, int linea, int tempo);
extern void DisplayTime(uint8_t hour, uint8_t minutes, uint8_t seconds);
extern void LcdInitialise(void);
extern void LcdClear();
extern void LcdWrite(uint8_t dc, uint8_t data);

#endif /* INCLUDE_CONFIGURATION_H_ */
