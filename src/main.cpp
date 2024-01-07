// *****************************************************************************
//  Project:            Eperly - Lite
//  Firmware Version:   1.1
//  MCU:                diyMore ESP8266 with 0.99" OLED LCD Module
//  Author:             Mark Angelo Tarvina (Tarvs' Hobbytronics)
//  Email:              mttarvina@gmail.com
//  Last Updated:       08.Dec.2023
// *****************************************************************************


// *****************************************************************************
// Release Notes:
// v1.0
//      + Initial release
//
// v1.1
//      + Updated server webpage, added CSS styling by @dajcoding
//      + Increased wait time for logo display from 3secs to 5secs
//      + Increased LED brightness adjustment increment from 5 to 25
//      + Increase LED heartbeat transition delay from 25ms to 50ms
//      + Increase LED heartbeat brightness increment from 1 to 2
//      + Increase LED color adjustment (r,g,b values) increment from 1 to 5
//      + Updated color palette
//      + Modified the approach when changing LED color
//          - Now checks the uri link and determines what color index is present
//          - in the hyperlink
//          - Removed typedef enum ColorSheme
// *****************************************************************************


#include <Arduino.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Wire.h>
#include "SSD1306Wire.h"


// Definitions
#define DEBUG                           true
#define SERIAL_TIMEOUT                  8000
#define WIFI_PORT                       80
#define SERVER_TIMEOUT                  5000                                    // (ms)
#define LED_MAX_BRIGHTNESS              255
#define LED_MIN_BRIGHTNESS              15
#define LED_NUM                         8                                       // 8 LEDs in Neopixel ring
#define LED_PIN                         D1                                      // D5
#define LED_ROT_TRANS_DELAY             120                                     // (ms)
#define LED_HRTBT_TRANS_DELAY           50                                      // (ms)
#define LED_HRTBT_BRIGHTNESS_INC        2                                       
#define LED_BRIGHTNESS_INC              25
#define LED_COLOR_TUNE_INC              5                                      
#define EEPROM_SIZE                     259                                     // 3 character indicators + 256 bytes for wifi ssid and password
#define LCD_SDA_PIN                     D5
#define LCD_SCL_PIN                     D6


typedef enum {
    STATIC                              = 0,
    HEARTBEAT,
    ROTATE 
} LEDPattern;


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


const int           colorTable[40]      = {
    // Common room lighting colors according to color temperature
    0xFF9329,                                                                   // Candle
    0xFFC58F,                                                                   // tungsten 40w
    0xFFD6AA,                                                                   // tungsten 100w
    0xFFF1E0,                                                                   // halogen
    0xFFFAF4,                                                                   // carbon arc
    0xFFFFF0,                                                                   // high noon
    0xFFFFFB,                                                                   // ivory
    0xFFFFFF,                                                                   // direct sunlight
    0xC9E2FF,                                                                   // overcast sky
    0x409CFF,                                                                   // clear blue sky

    // red - pink - orange palette
    0xF2D68B,                                                                   // color[10]
    0xEF8A1B,
    0xED583B,
    0xF4B8CE,
    0xEFA8A8,
    0xE85B94,
    0xC9245F,
    0xEF353F,
    0xBF1D29,
    0x89030D,

    // green palette
    0xD4EEEB,
    0xC6E4D9,
    0x85D0C6,
    0x73CDD1,
    0x00BDAE,
    0x1194A7,
    0x10686B,
    0x597C2B,
    0x0A5C36,
    0x14452F,

    // blue - purple palette
    0xC1E9FC,
    0x6ACDE6,
    0x0087BF,
    0x29338E,
    0xD69AC8,
    0xC28DE0,
    0x9990BA,
    0x7F4599,
    0x691D69,
    0x411E5C,
};


// Variables
const float         infoVersion         = 1.1;
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
int                 ledColor            = colorTable[7];                        // Default color after startup 
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
void render_inactive(void);
void render_active(void);
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
void color_set(void);


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
    String          buf         = "";

    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_NUM);                          // GRB ordering is assumed
    FastLED.setCorrection(TypicalSMD5050);
    FastLED.setDither(BINARY_DITHER);
    FastLED.showColor(CRGB::Black, LED_MAX_BRIGHTNESS);                         // set all LEDs to Black
    FastLED.showColor(CRGB::Black, LED_MAX_BRIGHTNESS);                         // set all LEDs to Black

    Serial.begin(9600);

    lcd.init();
    lcd.setI2cAutoInit(true);
    lcd.flipScreenVertically();
    lcd.setFont(ArialMT_Plain_10);
    lcd.setColor(WHITE);
    lcd.clear();

    lcd.drawFastImage(0, 0, 128, 64, logo);
    lcd.display();
    delay(5000);

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
        delay(200);
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
    webServer.on("/static", led_setToStatic);
    webServer.on("/rotate", led_setToRotate);
    webServer.on("/heartbeat", led_setToHeartbeat);
    webServer.on("/r/dec", decrease_redVal);
    webServer.on("/g/dec", decrease_greenVal);
    webServer.on("/b/dec", decrease_blueVal);
    webServer.on("/r/inc", increase_redVal);
    webServer.on("/g/inc", increase_greenVal);
    webServer.on("/b/inc", increase_blueVal);
    webServer.on("/color/0", color_set);
    webServer.on("/color/1", color_set);
    webServer.on("/color/2", color_set);
    webServer.on("/color/3", color_set);
    webServer.on("/color/4", color_set);
    webServer.on("/color/5", color_set);
    webServer.on("/color/6", color_set);
    webServer.on("/color/7", color_set);
    webServer.on("/color/8", color_set);
    webServer.on("/color/9", color_set);
    webServer.on("/color/10", color_set);
    webServer.on("/color/11", color_set);
    webServer.on("/color/12", color_set);
    webServer.on("/color/13", color_set);
    webServer.on("/color/14", color_set);
    webServer.on("/color/15", color_set);
    webServer.on("/color/16", color_set);
    webServer.on("/color/17", color_set);
    webServer.on("/color/18", color_set);
    webServer.on("/color/19", color_set);
    webServer.on("/color/20", color_set);
    webServer.on("/color/21", color_set);
    webServer.on("/color/22", color_set);
    webServer.on("/color/23", color_set);
    webServer.on("/color/24", color_set);
    webServer.on("/color/25", color_set);
    webServer.on("/color/26", color_set);
    webServer.on("/color/27", color_set);
    webServer.on("/color/28", color_set);
    webServer.on("/color/29", color_set);
    webServer.on("/color/30", color_set);
    webServer.on("/color/31", color_set);
    webServer.on("/color/32", color_set);
    webServer.on("/color/33", color_set);
    webServer.on("/color/34", color_set);
    webServer.on("/color/35", color_set);
    webServer.on("/color/36", color_set);
    webServer.on("/color/37", color_set);
    webServer.on("/color/38", color_set);
    webServer.on("/color/39", color_set);
    webServer.begin();
}


void loop(){
    webServer.handleClient();

    if (timerEn && (ledPattern == ROTATE) && ((millis() - timeStamp) > LED_ROT_TRANS_DELAY)){
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

    if (timerEn && (ledPattern == HEARTBEAT) && ((millis() - timeStamp) > LED_HRTBT_TRANS_DELAY)){
        timeStamp = millis();
        if (heartbeatDir){
            ledBrightnessInc += LED_HRTBT_BRIGHTNESS_INC;
        }
        else {
            ledBrightnessInc -= LED_HRTBT_BRIGHTNESS_INC;
        }
        if (ledBrightnessInc >= ledBrightness){
            ledBrightnessInc = ledBrightness;
            heartbeatDir = false;
        }
        else if (ledBrightnessInc <= 0){
            ledBrightnessInc = 0;
            heartbeatDir = true;
        }
        FastLED.showColor(ledColor, ledBrightnessInc);
    }
}


void server_htmlRender(void){
    if (ledState){
        render_active();
    }
    else {
        render_inactive();
    }
}

void render_inactive(void){
    String strHtmlContent = R"""(
<!DOCTYPE html>
<html lang='en'>
<head>
<meta charset='UTF-8'/>
<meta name='viewport' content='width=device-width, initial-scale=0.75'/>
<style>
:root {
--bg-1: rgb(0, 0, 0);
--bg-2: rgb(10, 10, 10);
--bg-3: rgb(40, 40, 40);

--body-dark-bg1: rgb(0, 0, 0);
--body-light-bg1: rgb(230, 172, 118);

--dark-building0: rgb(43, 3, 43);
--dark-building1: rgb(74, 4, 74);
--dark-building2: rgb(109, 5, 109);

--light-building0: rgb(90, 56, 35);
--light-building1: rgb(130, 78, 46);
--light-building2: rgb(191, 121, 78);
}
body {
padding: 0;
background: var(--body-dark-bg1);
overflow-y: scroll;
overflow-x: hidden;
display: flex;
align-items: center;
flex-direction: column;
font-family: fantasy, cursive;
}
#title {
color: white;
margin-top: 10vh;
display: flex;
justify-content: center;
}
#building-container0 {
width: 100vw;
display: flex;
flex-direction: row;
justify-content: center;
position: fixed;
}
#building-container1 {
width: 100vw;
display: flex;
flex-direction: row;
justify-content: center;
position: fixed;
}
#building-container2 {
width: 100vw;
display: flex;
flex-direction: row;
justify-content: center;
position: fixed;
}
#building-filter {
width: 100vw;
height: 100vh;
background: linear-gradient(rgb(0, 0, 0, 0) 40%, var(--body-dark-bg1) 90%);
position: fixed;
}
#page-container {
display: flex;
align-items: center;
flex-direction: row;
flex-wrap: wrap;
max-width: 100vh;
position: absolute;
margin-top: 10vh;
max-height: 100vh;
}
.lightBulb-container {
max-width: 30vh;
height: 30vh;
margin-top: 5vh;
display: flex;
align-items: center;
justify-content: center;
}
.quantizable {
display: flex;
justify-content: space-between;
margin-top: 20px;
margin-bottom: 30px;
max-width: 100vh;
}
button {
width: 100px;
opacity: 0%;
margin-left: 30px;
margin-right: 30px;
border-radius: 10px;
box-shadow: 5px 5px 10px 2px rgba(0,0,0,.8);
font-size: 200%;
}
.lightBulb {
position: relative;
min-width: 25vh;
min-height: 25vh;
background: black;
border: 1vh solid white;
border-radius: 50%;
display: flex;
align-items: center;
justify-content: center;
font-size: 300%;
color: white;
box-shadow: 5px 5px 10px 2px rgba(0,0,0,.8);
}
a {
text-decoration: none;
}
#optionsMenu {
max-width: 30vh;
height: 30vh;
margin-top: 5vh;
display: flex;
flex-direction: column;
align-items: center;
justify-content: center;
gap: 2vh;
visibility: hidden;
}
#building01 {
width: 30vh;
height: 75vh;
background: var(--dark-building0);
margin-left: 400px;
margin-top: 25vh;
position: absolute;
}
#building02 {
width: 15vh;
height: 60vh;
background: var(--dark-building0);
margin-left: 100px;
margin-top: 40vh;
position: absolute;
}
#building03 {
width: 35vh;
height: 70vh;
background: var(--dark-building0);
margin-left: -350px;
margin-top: 30vh;
position: absolute;
}
#building11 {
width: 15vh;
height: 55vh;
background: var(--dark-building1);
margin-left: -350px;
margin-top: 45vh;
position: absolute;
}
#building12 {
width: 15vh;
height: 50vh;
background: var(--dark-building1);
margin-left: -150px;
margin-top: 50vh;
position: absolute;
}
#building13 {
width: 15vh;
height: 55vh;
background: var(--dark-building1);
margin-left: 400px;
margin-top: 45vh;
position: absolute;
}
#building21 {
width: 45vh;
height: 52vh;
background: var(--dark-building2);
margin-left: 220px;
margin-top: 52vh;
position: absolute;
}
#building22 {
width: 15vh;
height: 50vh;
background: var(--dark-building2);
margin-left: -420px;
margin-top: 50vh;
position: absolute;
}
</style>
<title>Eperly-Lite v1.1</title>
</head>
<body id='body'>
<div id='building-container0'>
<div id='building01' class='building'></div>
<div id='building02' class='building'></div>
<div id='building03' class='building'></div>
</div>
<div id='building-container1'>
<div id='building11' class='building'></div>
<div id='building12' class='building'></div>
<div id='building13' class='building'></div>
</div>
<div id='building-container2'>
<div id='building21' class='building'></div>
<div id='building22' class='building'></div>
<div id='building23' class='building'></div>
</div>
<div id='building-filter'></div>
<div id='page-container'>
<div class='lightBulb-container'>
<a href='/on'><div class='lightBulb'>OFF</div></a>
</div>
<div id='optionsMenu'>
<button type='button' class='optionMenu'>Static</button>
<button type='button' class='optionMenu'>Heartbeat</button>
<button type='button' class='optionMenu'>Rotate</button>
</div>
</div>
</body>
</html>
    )""";
    webServer.send(200, "text/html", strHtmlContent);
}


void render_active(void){
    String strHtmlContent = R"""(
<!DOCTYPE html>
<html lang='en'>
<head>
<meta charset='UTF-8'/>
<meta name='viewport' content='width=device-width, initial-scale=0.75'/>
<style>
:root {
--bg-1: rgb(0, 0, 0);
--bg-2: rgb( 10, 10, 10);
--bg-3: rgb(40, 40, 40);

--body-dark-bg1: rgb(0, 0, 0);
--body-light-bg1: rgb(230, 172, 118);

--dark-building0: rgb(43, 3, 43);
--dark-building1: rgb(74, 4, 74);
--dark-building2: rgb(109, 5, 109);

--light-building0: rgb(90, 56, 35);
--light-building1: rgb(130, 78, 46);
--light-building2: rgb(191, 121, 78);
}
body {
padding: 0;
background: var(--body-light-bg1);
overflow-y: scroll;
overflow-x: hidden;
display: flex;
align-items: center;
flex-direction: column;
font-family: fantasy, cursive;
}
#title {
color: black;
margin-top: 10vh;
display: flex;
justify-content: center;
}
#building-container0 {
width: 100vw;
display: flex;
flex-direction: row;
justify-content: center;
position: fixed;
}
#building-container1 {
width: 100vw;
display: flex;
flex-direction: row;
justify-content: center;
position: fixed;
}
#building-container2 {
width: 100vw;
display: flex;
flex-direction: row;
justify-content: center;
position: fixed;
}
#building-filter {
width: 100vw;
height: 100vh;
background: linear-gradient(rgb(0, 0, 0, 0) 40%, var(--body-light-bg1) 90%);
position: fixed;
}
#page-container {
display: flex;
align-items: center;
flex-direction: row;
flex-wrap: wrap;
max-width: 100vh;
position: absolute;
margin-top: 10vh;
max-height: 100vh;
}
.slider-container {
max-width: 100vh;
height: 50vh;
margin-top: 40vh;
overflow: visible;
z-index: 2;
color: black;
}
.paletteContainer {
max-width: 100vh;
display: flex;
flex-wrap: wrap;
gap: 10px;
margin: 50px;
}
.lightBulb-container {
max-width: 30vh;
height: 30vh;
margin-top: 5vh;
display: flex;
align-items: center;
justify-content: center;
}
.quantizable {
display: flex;
justify-content: space-between;
margin-top: 20px;
margin-bottom: 30px;
max-width: 100vh;
}
button {
width: 100px;
opacity: 100%;
margin-left: 30px;
margin-right: 30px;
border-radius: 10px;
box-shadow: 5px 5px 10px 2px rgba(0,0,0,.8);
font-size: 200%;
}
button:hover {
scale: 120%;
}
.paletteContainer {
margin-left: 0px;
margin-top: 0px;
display: grid;
grid-template-columns: 35px 35px 35px 35px 35px 35px 35px 35px 35px 35px;
z-index: 1;
}
.colorBtn {
height: 35px;
width: 35px;
}
.adjustBtn {
width: 100px;
opacity: 100%;
margin-left: 30px;
margin-right: 30px;
border-radius: 10px;
box-shadow: 5px 5px 10px 2px rgba(0,0,0,.8);
font-size: 200%;
}
.lightBulb {
position: relative;
min-width: 25vh;
min-height: 25vh;
background: white;
border: 1vh solid black;
border-radius: 50%;
display: flex;
align-items: center;
justify-content: center;
font-size: 300%;
color: black;
box-shadow: 5px 5px 10px 2px rgba(0,0,0,.8);
}
a {
text-decoration: none;
}
#optionsMenu {
max-width: 30vh;
height: 30vh;
margin-top: 5vh;
display: flex;
flex-direction: column;
align-items: center;
justify-content: center;
gap: 2vh;
visibility: visible;
}
#fineTuneBtn {
display: flex;
opacity: 100%;
justify-content: space-between;
margin-top: 20px;
margin-bottom: 30px;
max-width: 100vh;
}
.optionMenu {
width: 100px;
height: 30px;
font-family: fantasy, cursive;
font-size: 100%;
box-shadow: 5px 5px 10px 2px rgba(0,0,0,.8);
}
#building01 {
width: 30vh;
height: 75vh;
background: var(--light-building0);
margin-left: 400px;
margin-top: 25vh;
position: absolute;
}
#building02 {
width: 15vh;
height: 60vh;
background: var(--light-building0);
margin-left: 100px;
margin-top: 40vh;
position: absolute;
}
#building03 {
width: 35vh;
height: 70vh;
background: var(--light-building0);
margin-left: -350px;
margin-top: 30vh;
position: absolute;
}
#building11 {
width: 15vh;
height: 55vh;
background: var(--light-building1);
margin-left: -350px;
margin-top: 45vh;
position: absolute;
}
#building12 {
width: 15vh;
height: 50vh;
background: var(--light-building1);
margin-left: -150px;
margin-top: 50vh;
position: absolute;
}
#building13 {
width: 15vh;
height: 55vh;
background: var(--light-building1);
margin-left: 400px;
margin-top: 45vh;
position: absolute;
}
#building21 {
width: 45vh;
height: 52vh;
background: var(--light-building2);
margin-left: 220px;
margin-top: 52vh;
position: absolute;
}
#building22 {
width: 15vh;
height: 50vh;
background: var(--light-building2);
margin-left: -420px;
margin-top: 50vh;
position: absolute;
}
</style>
<title>Eperly-Lite v1.1</title>
</head>
<body id='body'>
<div id='title'>
Eperly-Lite v1.1
</div>

<div id='building-container0'>
<div id='building01' class='building'></div>
<div id='building02' class='building'></div>
<div id='building03' class='building'></div>
</div>
<div id='building-container1'>
<div id='building11' class='building'></div>
<div id='building12' class='building'></div>
<div id='building13' class='building'></div>
</div>
<div id='building-container2'>
<div id='building21' class='building'></div>
<div id='building22' class='building'></div>
<div id='building23' class='building'></div>
</div>
<div id='building-filter'></div>
<div id='page-container'>
<div class='lightBulb-container'>
<a href='/off'><div id='lightBulb' class='lightBulb'>ON</div></a>
</div>
<div id='optionsMenu'>
<a href='/static'><button type='button' class='optionMenu'>Static</button></a>
<a href='/heartbeat'><button type='button' class='optionMenu'>Heartbeat</button></a>
<a href='/rotate'><button type='button' class='optionMenu'>Rotate</button></a>
</div>
</div>
<div class='slider-container'>
<div id='fineTuneBtn' class='quantizable'>
<a href='/brightness/dec'><button class='adjustBtn'>-</button></a>
<p>Brightness</p>
<a href='/brightness/inc'><button class='adjustBtn'>+</button></a>
</div>
<div id='fineTuneBtn' class='quantizable'>
<a href='/r/dec'><button class='adjustBtn'>-</button></a>
<p>Red</p>
<a href='/r/inc'><button class='adjustBtn'>+</button></a>
</div>
<div id='fineTuneBtn' class='quantizable'>
<a href='/g/dec'><button class='adjustBtn'>-</button></a>
<p>Green</p>
<a href='/g/inc'><button class='adjustBtn'>+</button></a>
</div>
<div id='fineTuneBtn' class='quantizable'>
<a href='/b/dec'><button class='adjustBtn'>-</button></a>
<p>Blue</p>
<a href='/b/inc'><button class='adjustBtn'>+</button></a>
</div>
</div>
<div class='paletteContainer'>
<a href='/color/0'><button class='colorBtn' style='background-color: #FF9329;'></button></a>
<a href='/color/1'><button class='colorBtn' style='background-color: #FFC58F;'></button></a>
<a href='/color/2'><button class='colorBtn' style='background-color: #FFD6AA;'></button></a>
<a href='/color/3'><button class='colorBtn' style='background-color: #FFF1E0;'></button></a>
<a href='/color/4'><button class='colorBtn' style='background-color: #FFFAF4;'></button></a>
<a href='/color/5'><button class='colorBtn' style='background-color: #FFFFF0;'></button></a>
<a href='/color/6'><button class='colorBtn' style='background-color: #FFFFFB;'></button></a>
<a href='/color/7'><button class='colorBtn' style='background-color: #FFFFFF;'></button></a>
<a href='/color/8'><button class='colorBtn' style='background-color: #C9E2FF;'></button></a>
<a href='/color/9'><button class='colorBtn' style='background-color: #409CFF;'></button></a>
<a href='/color/10'><button class='colorBtn' style='background-color: #F2D68B;'></button></a>
<a href='/color/11'><button class='colorBtn' style='background-color: #EF8A1B;'></button></a>
<a href='/color/12'><button class='colorBtn' style='background-color: #ED583B;'></button></a>
<a href='/color/13'><button class='colorBtn' style='background-color: #F4B8CE;'></button></a>
<a href='/color/14'><button class='colorBtn' style='background-color: #EFA8A8;'></button></a>
<a href='/color/15'><button class='colorBtn' style='background-color: #E85B94;'></button></a>
<a href='/color/16'><button class='colorBtn' style='background-color: #C9245F;'></button></a>
<a href='/color/17'><button class='colorBtn' style='background-color: #EF353F;'></button></a>
<a href='/color/18'><button class='colorBtn' style='background-color: #BF1D29;'></button></a>
<a href='/color/19'><button class='colorBtn' style='background-color: #89030D;'></button></a>
<a href='/color/20'><button class='colorBtn' style='background-color: #D4EEEB;'></button></a>
<a href='/color/21'><button class='colorBtn' style='background-color: #C6E4D9;'></button></a>
<a href='/color/22'><button class='colorBtn' style='background-color: #85D0C6;'></button></a>
<a href='/color/23'><button class='colorBtn' style='background-color: #73CDD1;'></button></a>
<a href='/color/24'><button class='colorBtn' style='background-color: #00BDAE;'></button></a>
<a href='/color/25'><button class='colorBtn' style='background-color: #1194A7;'></button></a>
<a href='/color/26'><button class='colorBtn' style='background-color: #10686B;'></button></a>
<a href='/color/27'><button class='colorBtn' style='background-color: #597C2B;'></button></a>
<a href='/color/28'><button class='colorBtn' style='background-color: #0A5C36;'></button></a>
<a href='/color/29'><button class='colorBtn' style='background-color: #14452F;'></button></a>
<a href='/color/30'><button class='colorBtn' style='background-color: #C1E9FC;'></button></a>
<a href='/color/31'><button class='colorBtn' style='background-color: #6ACDE6;'></button></a>
<a href='/color/32'><button class='colorBtn' style='background-color: #0087BF;'></button></a>
<a href='/color/33'><button class='colorBtn' style='background-color: #29338E;'></button></a>
<a href='/color/34'><button class='colorBtn' style='background-color: #D69AC8;'></button></a>
<a href='/color/35'><button class='colorBtn' style='background-color: #C28DE0;'></button></a>
<a href='/color/36'><button class='colorBtn' style='background-color: #9990BA;'></button></a>
<a href='/color/37'><button class='colorBtn' style='background-color: #7F4599;'></button></a>
<a href='/color/38'><button class='colorBtn' style='background-color: #691D69;'></button></a>
<a href='/color/39'><button class='colorBtn' style='background-color: #411E5C;'></button></a>
</div>
</body>
</html>
    )""";
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
        ledBrightness += LED_BRIGHTNESS_INC;
        if (ledBrightness > LED_MAX_BRIGHTNESS){
            ledBrightness = LED_MAX_BRIGHTNESS;
        }
        if ((ledPattern == STATIC) || (ledPattern == ROTATE)){
            FastLED.setBrightness(ledBrightness);
            FastLED.show();
            FastLED.show();
        }
    }
    server_htmlRender();
}


void decrease_brightness(void){
    if (ledState){
        ledBrightness -= LED_BRIGHTNESS_INC;
        if (ledBrightness < LED_MIN_BRIGHTNESS){
            ledBrightness = LED_MIN_BRIGHTNESS;
        }
        if ((ledPattern == STATIC) || (ledPattern == ROTATE)){
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


void color_set(void){
    uint8_t     index;
    String      hyperlink = webServer.uri();

    index = 39;
    while (index >= 0){
        if (hyperlink.indexOf(String(index)) >= 0){
            break;
        }
        index -= 1;
    }
    ledColor = colorTable[index];
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
        ledBrightnessInc = ledBrightness;
        heartbeatDir = false;
        FastLED.showColor(ledColor, ledBrightnessInc);
        timerEn = true;
        timeStamp = millis();
    }
    server_htmlRender();
}


void increase_redVal(void){
    redVal += LED_COLOR_TUNE_INC;
    if (redVal > 255){
        redVal = 255;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void increase_greenVal(void){
    greenVal += LED_COLOR_TUNE_INC;
    if (greenVal > 255){
        greenVal = 255;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void increase_blueVal(void){
    blueVal += LED_COLOR_TUNE_INC;
    if (blueVal > 255){
        blueVal = 255;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void decrease_redVal(void){
    redVal -= LED_COLOR_TUNE_INC;
    if (redVal < 0){
        redVal = 0;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void decrease_greenVal(void){
    greenVal -= LED_COLOR_TUNE_INC;
    if (greenVal < 0){
        greenVal = 0;
    }
    ledColor = (redVal << 16) | (greenVal << 8) | blueVal;
    led_setColor();
}


void decrease_blueVal(void){
    blueVal -= LED_COLOR_TUNE_INC;
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