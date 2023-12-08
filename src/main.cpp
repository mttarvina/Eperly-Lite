// *****************************************************************************
//  Project:            Eperly - Lite
//  Firmware Version:   1.0
//  MCU:                diyMore ESP8266 with 0.99" OLED LCD Module
//  Author:             Mark Angelo Tarvina (Tarvs' Hobbytronics)
//  Email:              mttarvina@gmail.com
//  Last Updated:       01.Dec.2023
// *****************************************************************************


#include <Arduino.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Wire.h>
#include "SSD1306Wire.h"


// Definitions
#define DEBUG                           false
#define SERIAL_TIMEOUT                  8000
#define WIFI_PORT                       80
#define SERVER_TIMEOUT                  2000                                    // (ms)
#define LED_MAX_BRIGHTNESS              255
#define LED_NUM                         8                                       // 8 LEDs in Neopixel ring
#define LED_PIN                         D1                                      // D5
#define LED_ROTATE_TRANSITION_DELAY     120                                     // (ms)
#define LED_HEARTBEAT_TRANSITION_DELAY  25                                      // (ms)
#define EEPROM_SIZE                     259                                     // 3 character indicators + 256 bytes for wifi ssid and password
#define LCD_SDA_PIN                     D5
#define LCD_SCL_PIN                     D6


typedef enum {
    CWHITE                              = 0,           
    CANDLE,          
    TUNGSTEN,        
    IVORY,           
    CLEAR_BLUE_SKY,  
    FLOURESCENT,     
    HALOGEN,         
    OVERCAST_SKY,    
    OFF_WHITE,       
    CREAM,           
    BUTTER,          
    CANARY_YELLOW,   
    BANANA_YELLOW,   
    CHEESE,          
    NEON_YELLOW,     
    MARIGOLD,        
    HONEY,           
    MUSTARD,         
    HARVEST_GOLD,    
    SATIN_GOLD,      
    FLOURESCENT_BLUE,
    PASTEL,          
    AQUA,            
    SKY_BLUE,        
    MALIBU_BLUE,     
    TEAL_BLUE,       
    OCEAN_BLUE,      
    SAPPHIRE_BLUE,   
    NEON_BLUE,       
    INDIGO,          
    AMBER,           
    APRICOT,         
    PEACH,           
    AMARANTH,        
    ROSE_RED,        
    SCARLET,         
    THISTLE,         
    NEON_PURPLE,     
    CLASSIC_PURPLE,  
    GRAPE           
} ColorScheme;



typedef enum {
    STATIC                              = 0,
    HEARTBEAT,
    ROTATE 
} LEDPattern;


// 'favicon', 64x128px
const unsigned char logo [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 
	0x00, 0xe0, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 
	0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0xe0, 0x7f, 0xc0, 0xff, 0xff, 0x03, 0x00, 
	0x00, 0xe0, 0x3f, 0xc0, 0xff, 0xff, 0x03, 0x00, 0x00, 0xe0, 0x1f, 0xe0, 0xff, 0xff, 0x07, 0x00, 
	0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 
	0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x07, 0xf8, 0xff, 0xff, 0x07, 0x00, 
	0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 
	0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x1f, 0xe0, 0x7f, 0xff, 0x07, 0x00, 
	0x00, 0xe0, 0x3f, 0xc0, 0x3f, 0xfe, 0x07, 0x00, 0x00, 0xe0, 0x7f, 0xe0, 0x1f, 0xfe, 0x07, 0x00, 
	0x00, 0xe0, 0xff, 0xff, 0x0f, 0xfe, 0x07, 0x00, 0x00, 0xe0, 0xff, 0xff, 0x07, 0xfe, 0x07, 0x00, 
	0x00, 0xe0, 0xff, 0xff, 0x03, 0xfe, 0x07, 0x00, 0x00, 0xe0, 0xff, 0xff, 0x07, 0xfe, 0x07, 0x00, 
	0x00, 0xe0, 0xff, 0xff, 0x0f, 0xfe, 0x07, 0x00, 0x00, 0xe0, 0x7f, 0xe0, 0x1f, 0xfe, 0x07, 0x00, 
	0x00, 0xe0, 0x3f, 0xc0, 0x3f, 0xfe, 0x07, 0x00, 0x00, 0xe0, 0x1f, 0xe0, 0x7f, 0xfe, 0x07, 0x00, 
	0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 
	0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x07, 0xf8, 0xff, 0xff, 0x07, 0x00, 
	0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 
	0x00, 0xe0, 0x0f, 0xf0, 0xff, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x1f, 0xe0, 0xff, 0xff, 0x07, 0x00, 
	0x00, 0xe0, 0x3f, 0xc0, 0xff, 0xff, 0x03, 0x00, 0x00, 0xc0, 0x7f, 0xc0, 0xff, 0xff, 0x03, 0x00, 
	0x00, 0xc0, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0xfe, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// // Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 1040)
// const int myBitmapallArray_LEN = 1;
// const unsigned char* myBitmapallArray[1] = {
// 	myBitmapfavicon
// };








// Lookup table for color palette
const int           colorTable[40]      = {
    0xFFE08C,
    0xFF9329,
    0xFFD6AA,
    0xFFFFF0,
    0x409CFF,
    0xF4FFFA,
    0xFFF1E0,
    0xC9E2FF,
    0xF2E9EA,
    0xEEE1C6,
    0xF6EB61,
    0xFFEF00,
    0xFEDD00,
    0xFBDB65,
    0xE0E722,
    0xFFAD4A,
    0xEBBC4E,
    0xEAAA00,
    0xDA9100,
    0xCBA135,
    0x11FFEE,
    0x8BD3E6,
    0x05C3DD,
    0x00B5E2,
    0x587EDE,
    0x007C80,
    0x005EB8,
    0x0F52BA,
    0x4D4DFF,
    0x4B0082,
    0xFFC600,
    0xFFB673,
    0xFFCBA4,
    0xF4364C,
    0xFF033E,
    0xBB0000,
    0xD8BFD8,
    0xC724B1,
    0xBB29BB,
    0x8031A7
};


// Variables
const float         infoVersion         = 1.0;
const char          *infoAuthor         = "mtt4rv1n4";
const char          *wifiHostname       = "eperly-lite";
char                wifiSSID[128]       = "";
char                wifiPassword[128]   = "";
bool                wifiInfoPresent     = true;
unsigned long       timeStamp           = 0;
bool                timerEn             = false; 
int                 ledBrightness       = 128;                                  // LED default brightness (50%)
bool                ledState            = false;
uint8_t             redVal              = 0;
uint8_t             greenVal            = 0;
uint8_t             blueVal             = 0;
int                 ledColor            = colorTable[CWHITE];                   // Default color after startup 
LEDPattern          ledPattern          = STATIC;                               // Default pattern after startup
volatile uint8_t    ledIndex            = 0;
int                 ledBrightnessInc    = 0;
bool                heartbeatDir        = false;                                // true = increasing, false = decreasing


CRGB                leds[LED_NUM];
ESP8266WebServer    webServer(WIFI_PORT);
SSD1306Wire         lcd(0x3c, LCD_SDA_PIN, LCD_SCL_PIN);


// Function definitions --> LED Patterns
void led_setColor(void);
void led_setToStatic(void);
void led_setToRotate(void);
void led_setToHeartbeat(void);


// Function definitions --> Web Server
void server_htmlRender(void);
void lamp_on(void);
void lamp_off(void);
void increase_brightness(void);
void decrease_brightness(void);
void increase_redVal(void);
void increase_greenVal(void);
void increase_blueVal(void);
void decrease_redVal(void);
void decrease_greenVal(void);
void decrease_blueVal(void);
void color_white(void);
void color_candle(void);
void color_tungsten(void);
void color_ivory(void);
void color_clearBlueSky(void);
void color_flourescent(void);
void color_halogen(void);
void color_overcastSky(void);
void color_offWhite(void);
void color_cream(void);
void color_butter(void);
void color_canaryYellow(void);
void color_bananaYellow(void);
void color_cheese(void);
void color_neonYellow(void);
void color_marigold(void);
void color_honey(void);
void color_mustard(void);
void color_harvestGold(void);
void color_satinGold(void);
void color_flourescentBlue(void);
void color_pastel(void);
void color_aqua(void);
void color_skyBlue(void);
void color_malibuBlue(void);
void color_tealBlue(void);
void color_oceanBlue(void);
void color_sapphireBlue(void);
void color_neonBlue(void);
void color_indigo(void);
void color_amber(void);
void color_apricot(void);
void color_peach(void);
void color_amaranth(void);
void color_roseRed(void);
void color_scarlet(void);
void color_thistle(void);
void color_neonPurple(void);
void color_classicPurple(void);
void color_grape(void);


// Function definitions --> EEPROM
void eeprom_init(void);
void eeprom_read(void);
void eeprom_write(void);
void eeprom_erase(void);
void serial_getWifiInfo(void);


void setup(){
    bool            flag        = false;
    byte            input;
    unsigned long   prevTime    = 0;
    uint8_t         i2cAddr     = 0;


    Serial.begin(9600);

    lcd.init();
    lcd.setI2cAutoInit(true);
    lcd.flipScreenVertically();
    lcd.setFont(ArialMT_Plain_10);
    lcd.setColor(WHITE);
    lcd.clear();

    lcd.drawFastImage(0, 0, 128, 64, logo);
    lcd.display();
    delay(3000);

    lcd.clear();

    Serial.println("\n\n[INFO] Project: \t\tEperly-Lite");
    lcd.drawString(0, 0, "> Eperly-Lite");
    lcd.display();
    delay(200);


    Serial.printf("[INFO] Firmware Version: \t%.1f\n", infoVersion);
    lcd.drawString(0, 15, "> Firmware Ver.: " + String(infoVersion));
    lcd.display();
    delay(200);

    Serial.println("[INFO] Author: \t\t\tTarvs' Hobbytronics");
    lcd.drawString(0, 25, "> By: Tarvs' Hobbytronics");
    lcd.display();
    delay(200);

    Serial.printf("[INFO] Email: \t\t\tmttarvina@gmail.com\n\n");
    lcd.drawString(0, 35, "> mttarvina@gmail.com");
    lcd.display();
    delay(5000);

    eeprom_init();
    eeprom_read();                                                              // extract wifi info from eeprom

    if (!wifiInfoPresent){
        Serial.println("Wifi info not present in EEPROM.");
        lcd.clear();
        lcd.drawString(0, 0, "> No WiFi info saved.");
        lcd.drawString(0, 15, "> Configure through USB");
        lcd.drawString(0, 25, "> Baud Rate = 9600");
        lcd.display();
        serial_getWifiInfo();
        eeprom_write();                                                         // save wifi info to eeprom
    }

    else {
        Serial.printf("Found WiFi credentials saved on EEPROM for SSID: %s\n", wifiSSID);
        #if DEBUG
            Serial.printf("[DEBUG] Wifi Password: %s\n", wifiPassword);
        #endif

        Serial.println("Would you like to update WiFi credentials? (Y/N):");

        lcd.clear();
        lcd.drawString(0, 0, "> Wifi credentials found.");
        lcd.drawString(0, 15, "> " + String(wifiSSID));
        lcd.drawString(0, 35, "> Update through USB");
        lcd.drawString(0, 45, "> Baud Rate = 9600");
        lcd.display();

        prevTime = millis();
        while (!flag){
            if ((millis() - prevTime) > SERIAL_TIMEOUT){
                flag = true;
            }
            if (Serial.available()){
                input = Serial.read();
                if ((input == 'Y') || (input == 'y')){
                    serial_getWifiInfo();
                    eeprom_write();
                    flag = true;
                }
                else if ((input == 'N') || (input == 'n')){
                    flag = true;
                }
                else if (input == '~'){
                    eeprom_erase();
                    Serial.println("Erased WiFi credentials saved on EEPROM. Reconnect/Reset device to reconfigure.");
                    while (true){
                        delay(10);
                    };
                }
                else if (input == 's'){
                    Serial.println("Scanning I2C devices...");
                    Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
                    for (i2cAddr = 0; i2cAddr < 127; i2cAddr++){
                        Wire.beginTransmission(i2cAddr);
                        if (0 == Wire.endTransmission()){
                            Serial.print("Found device at Addr = ");
                            Serial.println(i2cAddr);
                        }
                    }
                }
            }
        }
    }

    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_NUM);                          // GRB ordering is assumed
    FastLED.setCorrection(TypicalSMD5050);
    FastLED.setDither(BINARY_DITHER);
    FastLED.showColor(CRGB::Black, LED_MAX_BRIGHTNESS);                         // set all LEDs to Black
    FastLED.showColor(CRGB::Black, LED_MAX_BRIGHTNESS);                         // set all LEDs to Black
    
    // Connect to WiFi
    Serial.print("\nConnecting to ");
    Serial.println(wifiSSID);
    lcd.clear();
    lcd.drawString(0, 0, "> Connecting to WiFi");
    lcd.drawString(0, 15, "> " + String(wifiSSID));
    lcd.display();

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID, wifiPassword);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.drawString(0, 0, "> Wifi Connected");
    lcd.drawString(0, 15, "> " + String(wifiSSID));
    lcd.drawString(0, 35, "> " + WiFi.localIP().toString());
    lcd.display();
    
    // Setup routes and start webserver
    webServer.on("/", server_htmlRender);                                       // render the default HTML view
    webServer.on("/on", lamp_on);
    webServer.on("/off", lamp_off);
    webServer.on("/brightness/dec", decrease_brightness);
    webServer.on("/brightness/inc", increase_brightness);
    webServer.on("/color/white", color_white);
    webServer.on("/color/candle", color_candle);
    webServer.on("/color/tungsten", color_tungsten);
    webServer.on("/color/ivory", color_ivory);
    webServer.on("/color/clear_blue_sky", color_clearBlueSky);
    webServer.on("/color/flourescent", color_flourescent);
    webServer.on("/color/halogen", color_halogen);
    webServer.on("/color/overcast_sky", color_overcastSky);
    webServer.on("/color/off_white", color_offWhite);
    webServer.on("/color/cream", color_cream);
    webServer.on("/color/butter", color_butter);
    webServer.on("/color/canary_yellow", color_canaryYellow);
    webServer.on("/color/banana_yellow", color_bananaYellow);
    webServer.on("/color/cheese", color_cheese);
    webServer.on("/color/neon_yellow", color_neonYellow);
    webServer.on("/color/marigold", color_marigold);
    webServer.on("/color/honey", color_honey);
    webServer.on("/color/mustard", color_mustard);
    webServer.on("/color/harvest_gold", color_harvestGold);
    webServer.on("/color/satin_gold", color_satinGold);
    webServer.on("/color/flourescent_blue", color_flourescentBlue);
    webServer.on("/color/pastel", color_pastel);
    webServer.on("/color/aqua", color_aqua);
    webServer.on("/color/sky_blue", color_skyBlue);
    webServer.on("/color/malibu_blue", color_malibuBlue);
    webServer.on("/color/teal_blue", color_tealBlue);
    webServer.on("/color/ocean_blue", color_oceanBlue);
    webServer.on("/color/sapphire_blue", color_sapphireBlue);
    webServer.on("/color/neon_blue", color_neonBlue);
    webServer.on("/color/indigo", color_indigo);
    webServer.on("/color/amber", color_amber);
    webServer.on("/color/apricot", color_apricot);
    webServer.on("/color/peach", color_peach);
    webServer.on("/color/amaranth", color_amaranth);
    webServer.on("/color/rose_red", color_roseRed);
    webServer.on("/color/scarlet", color_scarlet);
    webServer.on("/color/thistle", color_thistle);
    webServer.on("/color/neon_purple", color_neonPurple);
    webServer.on("/color/classic_purple", color_classicPurple);
    webServer.on("/color/grape", color_grape);
    webServer.on("/static", led_setToStatic);
    webServer.on("/rotate", led_setToRotate);
    webServer.on("/heartbeat", led_setToHeartbeat);
    webServer.on("/r/dec", decrease_redVal);
    webServer.on("/g/dec", decrease_greenVal);
    webServer.on("/b/dec", decrease_blueVal);
    webServer.on("/r/inc", increase_redVal);
    webServer.on("/g/inc", increase_greenVal);
    webServer.on("/b/inc", increase_blueVal);
    webServer.begin();
}


void loop(){
    webServer.handleClient();

    if (timerEn && (ledPattern == ROTATE) && ((millis() - timeStamp) > LED_ROTATE_TRANSITION_DELAY)){
        timeStamp = millis();
        ledIndex += 1;
        if (ledIndex == LED_NUM){
            ledIndex = 0;
        }
        if (0 == ledIndex){
            leds[LED_NUM - 1] = CRGB::Black;
            leds[ledIndex] = ledColor;   
        }
        else {
            leds[ledIndex - 1] = CRGB::Black;
            leds[ledIndex] = ledColor;        
        }
        FastLED.show();
        FastLED.show();
    }

    if (timerEn && (ledPattern == HEARTBEAT) && ((millis() - timeStamp) > LED_HEARTBEAT_TRANSITION_DELAY)){
        timeStamp = millis();
        if (ledBrightnessInc == ledBrightness){
            heartbeatDir = false;
        }
        else if (ledBrightnessInc == 0){
            heartbeatDir = true;
        }
        
        if (heartbeatDir){
            ledBrightnessInc += 1;
            FastLED.showColor(ledColor, ledBrightnessInc);
        }
        else {
            ledBrightnessInc -= 1;
            FastLED.showColor(ledColor, ledBrightnessInc);
        }
    }
}


void server_htmlRender(void){
    String strHtmlContent = "";
    String strMainSwitch = "";                                                  // for Lamp On/Off button
    String strPattern = "";                                                     // for LED pattern
    String strBrightnessVal = "";                                               // Display brightness level
    String strRVal = "";                                                        // Red value
    String strGVal = "";                                                        // Red value
    String strBVal = "";                                                        // Red value
    String strHtmlHead = R"""(
<!doctype html><html lang='en'>
<head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>
    <style>
        body {
            font-family: 'Rubik', sans-serif;
            color: #253349;
        }
        .centertext {
            text-align: center;
        }
        .text-justify {
            text-align: justify;
        }
        .button-disabled {
            border: none;
            background-color: grey;
            padding: 5px 5px;
            text-align: center;
        }
        .button-enabled1 {
            border: none;
            background-color: green;
            padding: 5px 5px;
            text-align: center;
        }
        .button-enabled2 {
            border: none;
            background-color: yellow;
            padding: 5px 5px;
            text-align: center;
        }
        .button-color {
            border-width: 2px 2px;
            padding: 10px 10px;
        }
    </style>
    <title>Eperly - Home</title>
</head>
    )""";

    String strH1 = R"""(
<body>
    <div class='centertext'><h2>Eperly - Lite</h2><p><small>Version: 1.0</small></p></div><hr>
    )""";

    if (ledState){
        strMainSwitch = R"""(
    <div class='centertext'><a href='/off'><button class='button-enabled1' style='font-size: x-large;'>Lamp OFF</button></a></div><hr>
        )""";
    }
    else {
        strMainSwitch = R"""(
    <div class='centertext'><a href='/on'><button class='button-disabled' style='font-size: x-large;'>Lamp ON</button></a></div><hr>
        )""";
    }
    

    switch (ledPattern){
        case STATIC:
            strPattern = R"""(
    <div class='centertext'>
        <a href='/static'><button class='button-enabled2' style='font-size: large;'>Static</button></a>
        <a href='/heartbeat'><button class='button-disabled' style='font-size: large;'>Heartbeat</button></a>
        <a href='/rotate'><button class='button-disabled' style='font-size: large;'>Rotate</button></a>        
    </div><hr>
    <div class='centertext'>
        <a href='/brightness/dec'><button class='button-disabled' style='font-size: large;'>  -  </button></a>
            )""";
            break;
        case HEARTBEAT:
            strPattern = R"""(
    <div class='centertext'>
        <a href='/static'><button class='button-disabled' style='font-size: large;'>Static</button></a>
        <a href='/heartbeat'><button class='button-enabled2' style='font-size: large;'>Heartbeat</button></a>
        <a href='/rotate'><button class='button-disabled' style='font-size: large;'>Rotate</button></a>        
    </div><hr>
    <div class='centertext'>
        <a href='/brightness/dec'><button class='button-disabled' style='font-size: large;'>  -  </button></a>
            )""";
            break;
        case ROTATE:
            strPattern = R"""(
            <div class='centertext'>
                <a href='/static'><button class='button-disabled' style='font-size: large;'>Static</button></a>
                <a href='/heartbeat'><button class='button-disabled' style='font-size: large;'>Heartbeat</button></a>
                <a href='/rotate'><button class='button-enabled2' style='font-size: large;'>Rotate</button></a>        
            </div><hr>
            <div class='centertext'>
                <a href='/brightness/dec'><button class='button-disabled' style='font-size: large;'>  -  </button></a>
            )""";
    }

    strBrightnessVal = "<button class='button-disabled' style='font-size: large;'> Brightness = " + String(static_cast<int>(100*ledBrightness/LED_MAX_BRIGHTNESS)) + "% </button>";

    String strColorPalette = R"""(
        <a href='/brightness/inc'><button class='button-disabled' style='font-size: large;'>  +  </button></a>
    </div><hr>
    <div class='centertext'>
        <a href='/color/white'><button class='button-color' style='background-color: #FFE08C;'></button></a>
        <a href='/color/candle'><button class='button-color' style='background-color: #FF9329;'></button></a>
        <a href='/color/tungsten'><button class='button-color' style='background-color: #FFD6AA;'></button></a>
        <a href='/color/ivory'><button class='button-color' style='background-color: #FFFFF0;'></button></a>
        <a href='/color/clear_blue_sky'><button class='button-color' style='background-color: #409CFF;'></button></a>
        <a href='/color/flourescent'><button class='button-color' style='background-color: #F4FFFA;'></button></a>
        <a href='/color/halogen'><button class='button-color' style='background-color: #FFF1E0;'></button></a>
        <a href='/color/overcast_sky'><button class='button-color' style='background-color: #C9E2FF;'></button></a>
        <a href='/color/off_white'><button class='button-color' style='background-color: #F2E9EA;'></button></a>
        <a href='/color/cream'><button class='button-color' style='background-color: #EEE1C6;'></button></a>
    </div><br>
    <div class='centertext'>
        <a href='/color/butter'><button class='button-color' style='background-color: #F6EB61;'></button></a>
        <a href='/color/canary_yellow'><button class='button-color' style='background-color: #FFEF00;'></button></a>
        <a href='/color/banana_yellow'><button class='button-color' style='background-color: #FEDD00;'></button></a>
        <a href='/color/cheese'><button class='button-color' style='background-color: #FBDB65;'></button></a>
        <a href='/color/neon_yellow'><button class='button-color' style='background-color: #E0E722;'></button></a>
        <a href='/color/marigold'><button class='button-color' style='background-color: #FFAD4A;'></button></a>
        <a href='/color/honey'><button class='button-color' style='background-color: #EBBC4E;'></button></a>
        <a href='/color/mustard'><button class='button-color' style='background-color: #EAAA00;'></button></a>
        <a href='/color/harvest_gold'><button class='button-color' style='background-color: #DA9100;'></button></a>
        <a href='/color/satin_gold'><button class='button-color' style='background-color: #CBA135;'></button></a>
    </div><br>
    <div class='centertext'>
        <a href='/color/flourescent_blue'><button class='button-color' style='background-color: #11FFEE;'></button></a>
        <a href='/color/pastel'><button class='button-color' style='background-color: #8BD3E6;'></button></a>
        <a href='/color/aqua'><button class='button-color' style='background-color: #05C3DD;'></button></a>
        <a href='/color/sky_blue'><button class='button-color' style='background-color: #00B5E2;'></button></a>
        <a href='/color/malibu_blue'><button class='button-color' style='background-color: #587EDE;'></button></a>
        <a href='/color/teal_blue'><button class='button-color' style='background-color: #007C80;'></button></a>
        <a href='/color/ocean_blue'><button class='button-color' style='background-color: #005EB8;'></button></a>
        <a href='/color/sapphire_blue'><button class='button-color' style='background-color: #0F52BA;'></button></a>
        <a href='/color/neon_blue'><button class='button-color' style='background-color: #4D4DFF;'></button></a>
        <a href='/color/indigo'><button class='button-color' style='background-color: #4B0082;'></button></a>
    </div><br>
    <div class='centertext'>
        <a href='/color/amber'><button class='button-color' style='background-color: #FFC600;'></button></a>
        <a href='/color/apricot'><button class='button-color' style='background-color: #FFB673;'></button></a>
        <a href='/color/peach'><button class='button-color' style='background-color: #FFCBA4;'></button></a>
        <a href='/color/amaranth'><button class='button-color' style='background-color: #F4364C;'></button></a>
        <a href='/color/rose_red'><button class='button-color' style='background-color: #FF033E;'></button></a>
        <a href='/color/scarlet'><button class='button-color' style='background-color: #BB0000;'></button></a>
        <a href='/color/thistle'><button class='button-color' style='background-color: #D8BFD8;'></button></a>
        <a href='/color/neon_purple'><button class='button-color' style='background-color: #C724B1;'></button></a>
        <a href='/color/classic_purple'><button class='button-color' style='background-color: #BB29BB;'></button></a>
        <a href='/color/grape'><button class='button-color' style='background-color: #8031A7;'></button></a>
    </div>
    <hr>
    <div class='centertext'>
        <a href='/r/dec'><button class='button-disabled' style='font-size: large;'>  -  </button></a>
    )""";
    
    strRVal = "<button class='button-disabled' style='font-size: large;'> R = " + String(redVal) + " </button>";

    String strBuf1 = R"""(
        <a href='/r/inc'><button class='button-disabled' style='font-size: large;'>  +  </button></a>
    </div><hr>
    <div class='centertext'>
        <a href='/g/dec'><button class='button-disabled' style='font-size: large;'>  -  </button></a>
    )""";

    strGVal = "<button class='button-disabled' style='font-size: large;'> G = " + String(greenVal) + " </button>";

    String strBuf2 = R"""(
        <a href='/g/inc'><button class='button-disabled' style='font-size: large;'>  +  </button></a>
    </div><hr>
    <div class='centertext'>
        <a href='/b/dec'><button class='button-disabled' style='font-size: large;'>  -  </button></a>
    )""";

    strBVal = "<button class='button-disabled' style='font-size: large;'> B = " + String(blueVal) + " </button>";

    String strBuf3 = R"""(
        <a href='/b/inc'><button class='button-disabled' style='font-size: large;'>  +  </button></a>
    </div><hr>
    <footer class='centertext'><div><small>Powered by: Tarvs' Hobbytronics</small></div></footer>        
</html>
    )""";

    strHtmlContent = strHtmlHead + strH1 + strMainSwitch + strPattern + strBrightnessVal + strColorPalette + strRVal + strBuf1 + strGVal + strBuf2 + strBVal + strBuf3;
    webServer.send(200, "text/html", strHtmlContent);
}


void lamp_on(void){
    ledState = true;
    if (ledPattern == STATIC){
        led_setToStatic();
    }
    else if (ledPattern == ROTATE){
        led_setToRotate();
    }
    else if (ledPattern == HEARTBEAT){
        led_setToHeartbeat();
    }
}


void lamp_off(void){
    if ((ledPattern == ROTATE) || (ledPattern == HEARTBEAT)){
        timerEn = false;
    }
    FastLED.setBrightness(ledBrightness);
    for (uint8_t i = 0; i < LED_NUM; i++){
        leds[i] = CRGB::Black;
    }
    FastLED.show();
    FastLED.show();
    redVal = 0x00;
    greenVal = 0x00;
    blueVal = 0x00; 
    ledState = false;
    server_htmlRender();
}


void increase_brightness(void){
    if (ledState){
        ledBrightness += 5;
        if (ledBrightness > LED_MAX_BRIGHTNESS){
            ledBrightness = LED_MAX_BRIGHTNESS;
        }
        if (ledPattern == STATIC){
            FastLED.showColor(ledColor, ledBrightness);
        }
        else if (ledPattern == ROTATE){
            FastLED.setBrightness(ledBrightness);
            FastLED.show();
            FastLED.show();
        }
    }
    server_htmlRender();
}


void decrease_brightness(void){
    if (ledState){
        ledBrightness -= 5;
        if (ledBrightness < 1){
            ledBrightness = 1;
        }
        if (ledPattern == STATIC){
            FastLED.showColor(ledColor, ledBrightness);
        }
        else if (ledPattern == ROTATE){
            FastLED.setBrightness(ledBrightness);
            FastLED.show();
            FastLED.show();
        }
    }
    server_htmlRender();
}


void led_setColor(void){
    if (ledState){
        if (ledPattern == STATIC){
            FastLED.setBrightness(ledBrightness);
            for (uint8_t i = 0; i < LED_NUM; i++){
                leds[i] = ledColor;
            }
            FastLED.show();
            FastLED.show();
        }
        else if (ledPattern == HEARTBEAT){
            FastLED.showColor(ledColor, ledBrightnessInc);
        }
        redVal = (ledColor & 0xFF0000) >> 16;
        greenVal = (ledColor & 0x00FF00) >> 8;
        blueVal = ledColor & 0x0000FF;
    }
    server_htmlRender();
}


void color_white(void){
    ledColor = colorTable[CWHITE];
    led_setColor();        
}


void color_candle(void){
    ledColor = colorTable[CANDLE];
    led_setColor();
}


void color_tungsten(void){
    ledColor = colorTable[TUNGSTEN];
    led_setColor();
}


void color_ivory(void){
    ledColor = colorTable[IVORY];
    led_setColor();
}


void color_clearBlueSky(void){
    ledColor = colorTable[CLEAR_BLUE_SKY];
    led_setColor();
}


void color_flourescent(void){
    ledColor = colorTable[FLOURESCENT];
    led_setColor();
}


void color_halogen(void){
    ledColor = colorTable[HALOGEN];
    led_setColor();
}


void color_overcastSky(void){
    ledColor = colorTable[OVERCAST_SKY];
    led_setColor();
}


void color_offWhite(void){
    ledColor = colorTable[OFF_WHITE];
    led_setColor();
}


void color_cream(void){
    ledColor = colorTable[CREAM];
    led_setColor();
}


void color_butter(void){
    ledColor = colorTable[BUTTER];
    led_setColor();
}


void color_canaryYellow(void){
    ledColor = colorTable[CANARY_YELLOW];
    led_setColor();
}


void color_bananaYellow(void){
    ledColor = colorTable[BANANA_YELLOW];
    led_setColor();
}


void color_cheese(void){
    ledColor = colorTable[CHEESE];
    led_setColor();
}


void color_neonYellow(void){
    ledColor = colorTable[NEON_YELLOW];
    led_setColor();
}


void color_marigold(void){
    ledColor = colorTable[MARIGOLD];
    led_setColor();
}


void color_honey(void){
    ledColor = colorTable[HONEY];
    led_setColor();
}


void color_mustard(void){
    ledColor = colorTable[MUSTARD];
    led_setColor();
}


void color_harvestGold(void){
    ledColor = colorTable[HARVEST_GOLD];
    led_setColor();
}


void color_satinGold(void){
    ledColor = colorTable[SATIN_GOLD];
    led_setColor();
}


void color_flourescentBlue(void){
    ledColor = colorTable[FLOURESCENT_BLUE];
    led_setColor();
}


void color_pastel(void){
    ledColor = colorTable[PASTEL];
    led_setColor();
}


void color_aqua(void){
    ledColor = colorTable[AQUA];
    led_setColor();
}


void color_skyBlue(void){
    ledColor = colorTable[SKY_BLUE];
    led_setColor();
}


void color_malibuBlue(void){
    ledColor = colorTable[MALIBU_BLUE];
    led_setColor();
}


void color_tealBlue(void){
    ledColor = colorTable[TEAL_BLUE];
    led_setColor();
}


void color_oceanBlue(void){
    ledColor = colorTable[OCEAN_BLUE];
    led_setColor();
}


void color_sapphireBlue(void){
    ledColor = colorTable[SAPPHIRE_BLUE];
    led_setColor();
}


void color_neonBlue(void){
    ledColor = colorTable[NEON_BLUE];
    led_setColor();
}


void color_indigo(void){
    ledColor = colorTable[INDIGO];
    led_setColor();
}


void color_amber(void){
    ledColor = colorTable[AMBER];
    led_setColor();
}


void color_apricot(void){
    ledColor = colorTable[APRICOT];
    led_setColor();
}


void color_peach(void){
    ledColor = colorTable[PEACH];
    led_setColor();
}


void color_amaranth(void){
    ledColor = colorTable[AMARANTH];
    led_setColor();
}


void color_roseRed(void){
    ledColor = colorTable[ROSE_RED];
    led_setColor();
}


void color_scarlet(void){
    ledColor = colorTable[SCARLET];
    led_setColor();
}


void color_thistle(void){
    ledColor = colorTable[THISTLE];
    led_setColor();
}


void color_neonPurple(void){
    ledColor = colorTable[NEON_PURPLE];
    led_setColor();
}


void color_classicPurple(void){
    ledColor = colorTable[CLASSIC_PURPLE];
    led_setColor();
}


void color_grape(void){
    ledColor = colorTable[GRAPE];
    led_setColor();
}


void led_setToStatic(void){
    if (ledState){
        if ((ledPattern == ROTATE) || (ledPattern == HEARTBEAT)){
            timerEn = false;
        }
        ledPattern = STATIC;
        led_setColor();
    }
    else {
        server_htmlRender();
    }
    
}


void led_setToRotate(void){
    if (ledState){
        ledPattern = ROTATE;
        ledIndex = 0;
        FastLED.showColor(CRGB::Black, LED_MAX_BRIGHTNESS);
        FastLED.showColor(CRGB::Black, LED_MAX_BRIGHTNESS);
        leds[LED_NUM - 1] = CRGB::Black;
        leds[ledIndex] = ledColor;
        FastLED.show();
        FastLED.show();

        timerEn = true;
        timeStamp = millis();
    }
    server_htmlRender();
}


void led_setToHeartbeat(void){
    if (ledState){
        ledPattern = HEARTBEAT;
        ledBrightnessInc = 0;
        FastLED.showColor(ledColor, ledBrightnessInc);
        timerEn = true;
        timeStamp = millis();
    }
    server_htmlRender();
}


void increase_redVal(void){
    redVal += 1;
    if (redVal > 255){
        redVal = 255;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void increase_greenVal(void){
    greenVal += 1;
    if (greenVal > 255){
        greenVal = 255;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void increase_blueVal(void){
    blueVal += 1;
    if (blueVal > 255){
        blueVal = 255;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void decrease_redVal(void){
    redVal -= 1;
    if (redVal < 0){
        redVal = 0;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void decrease_greenVal(void){
    greenVal -= 1;
    if (greenVal < 0){
        greenVal = 0;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void decrease_blueVal(void){
    blueVal -= 1;
    if (blueVal < 0){
        blueVal = 0;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void eeprom_init(void){
    Serial.println("Initializing EEPROM");
    EEPROM.begin(EEPROM_SIZE);
}


void eeprom_read(void){
    int     addr            = 0;
    byte    readValue;
    uint8_t ssidIndex       = 0;
    uint8_t passwordIndex   = 0;

    readValue = EEPROM.read(addr);
    if ('\n' == char(readValue)){
        wifiInfoPresent = true;
        addr += 1;
    }
    else {
        wifiInfoPresent = false;
        return;
    }
    
    while(true){
        readValue = EEPROM.read(addr);
        if ('\n' == char(readValue)){
            break;
        }
        else {
            wifiSSID[ssidIndex] = char(readValue);
            ssidIndex += 1;
            addr += 1;
        }
    }

    addr += 1;
    while(true){
        readValue = EEPROM.read(addr);
        if ('\n' == char(readValue)){
            break;
        }
        else {
            wifiPassword[passwordIndex] = char(readValue);
            passwordIndex += 1;
            addr += 1;
        }
    }
}


void eeprom_erase(void){
    for (int i = 0; i < EEPROM_SIZE; i++){
        EEPROM.write(i, 0);                             
    }
    EEPROM.commit();
}


void eeprom_write(void){
    int addr        = 0;
    int i           = 0;

    Serial.println("Writing WiFi info to EEPROM...");
    
    // overwrite all EEPROM data
    eeprom_erase();

    EEPROM.write(addr, '\n');                                                   // write the first character indicator
    addr += 1;

    for (i = 0; i < (sizeof(wifiSSID)/sizeof(char)); i++){                      // write the SSID
        if ('\0' == wifiSSID[i]){
            break;
        }
        else{
            EEPROM.write(addr, wifiSSID[i]);
            addr += 1;
        }
    }

    EEPROM.write(addr, '\n');                                                   // write the 2nd character indicator
    addr += 1;

    for (i = 0; i < (sizeof(wifiPassword)/sizeof(char)); i++){                  // write the Password
        if ('\0' == wifiPassword[i]){
            break;
        }
        else{
            EEPROM.write(addr, wifiPassword[i]);
            addr += 1;
        }
    }

    EEPROM.write(addr, '\n');                                                   // write the 3rd character indicator
    EEPROM.commit();
    Serial.println("Finished writing WiFi credentials to EEPROM.");
}


void serial_getWifiInfo(void){
    int     i               = 0;
    bool    flag            = false;
    bool    correctInput    = false;
    char    input;
    int     ssidIndex       = 0;
    int     passwordIndex   = 0;
    int     validChars      = 0;

    while (!correctInput){
        Serial.println("Please enter WiFi SSID:");
        flag = false;
        validChars = 0;
        while (!flag){
            if (Serial.available()){
                input = Serial.read();
                if (('\n' != input) && ('\r' != input)){
                    wifiSSID[ssidIndex] = input;
                    ssidIndex += 1;
                    validChars += 1;
                }
                else {                   
                    if (validChars > 0){
                        wifiSSID[ssidIndex] = '\0';
                        flag = true;
                        Serial.printf("SSID: %s\n", wifiSSID);
                    }
                }
            }
        }
        Serial.println("SSID Correct? (Y/N):");
        flag = false;
        while (!flag){
            if (Serial.available()){
                input = Serial.read();
                if ((input == 'Y') || (input == 'y')){
                    flag = true;
                    correctInput = true;
                }
                else if ((input == 'N') || (input == 'n')){
                    for (i = 0; i < sizeof(wifiSSID)/sizeof(char); i++){
                        wifiSSID[i] = '\0';
                    }
                    ssidIndex = 0;
                    flag = true;
                    correctInput = false;
                }
                else {
                    flag = false;
                }
            }
        }
    }

    flag = false;
    correctInput = false;
    while (!correctInput){
        Serial.println("Please enter WiFi Password:");
        flag = false;
        validChars = 0;
        while (!flag){
            if (Serial.available()){
                input = Serial.read();
                if (('\n' != input) && ('\r' != input)){
                    wifiPassword[passwordIndex] = input;
                    passwordIndex += 1;
                    validChars += 1;
                }
                else {
                    if (validChars > 0){
                        wifiPassword[passwordIndex] = '\0';
                        flag = true;
                        Serial.printf("Password: %s\n", wifiPassword);
                    }
                }
            }
        }
        Serial.println("Password Correct? (Y/N):");
        flag = false;
        while (!flag){
            if (Serial.available()){
                input = Serial.read();
                if ((input == 'Y') || (input == 'y')){
                    flag = true;
                    correctInput = true;
                }
                else if ((input == 'N') || (input == 'n')){
                    for (i = 0; i < sizeof(wifiPassword)/sizeof(char); i++){
                        wifiPassword[i] = '\0';
                    }
                    passwordIndex = 0;
                    flag = true;
                    correctInput = false;
                }
                else {
                    flag = false;
                }
            }
        }
    }
}