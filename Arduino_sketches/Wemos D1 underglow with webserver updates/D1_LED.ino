//This is an early WIP and som things are not finished or polished.
//BLYNK declarations and WIFI
//INSERT YOUR OWN VALUES
#define BLYNK_TEMPLATE_ID "INSERT_BLYNK_TEMPLATE_ID_HERE"
#define BLYNK_DEVICE_NAME "INSERT_BLYNK_DEVICE_NAME_HERE"
#define BLYNK_FIRMWARE_VERSION        "INSERT_SKETCH_VERSION_HERE"
const char* host = "INSERT_HOSTNAME_HERE";
char auth[] = "INSERT_BLYNK_AUTH_TOKEN_HERE";
char ssid[] = "INSERT_WIFI_SSID_HERE";
char pass[] = "INSERT_WIFI_PASSWORD_HERE";
const char* update_path = "/firmware";
const char* update_username = "admin";//or whatever username you want to use for updates
const char* update_password = "INSERT_AUTHORIZATION_PASSWORD_HERE";

//Libriaries
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <BlynkSimpleEsp8266.h>
#include <FastLED.h>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//ATTENTION!
//Adjust the following section to your configuration!
//If this section is not configured correctly, things will not work as expected!

//Hardware
#define LED_DATA_PIN  2
#define NUM_LEDS 109
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100
#define NUM_OF_BLENDS 2
#define NUM_OF_PALETTES 8
#define NUM_OF_COLORS 1
#define BRIGHTNESS 255
#define BUILTIN_LED 14

//Blynk
#define SWITCH_PALETTE V0
#define SWITCH_BLEND V1
#define PALETTE V2
#define BLEND V3
#define ONOFF V4
#define BRIGHTNESS_OUTPUT V5
#define BRIGHTNESS_INPUT V6
#define COLOR_SWITCH_INPUT V7
#define COLOR_OUTPUT V8
#define RGB_INPUT V9


//global variables
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;
int paletteId = 0;
int blendId = 0;
String paletteName = "Rainbow";
String blendName = "Linear blend";
uint8_t inputBrightness = 255;
uint8_t brightness = 255;
bool onoff = false;
bool incBright = true;
int currentColorIndex = 0;
String currentColorText = "Blue";
CRGB currentColor = CRGB::Blue;

void setup() {
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, pass);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
    }
    
    MDNS.begin(host);
    httpUpdater.setup(&httpServer, update_path, update_username, update_password);
    httpServer.begin();
    MDNS.addService("http", "tcp", 80);
    
    delay( 3000 );
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    Blynk.begin(auth, ssid, pass);
}

void loop()
{
    httpServer.handleClient();
    MDNS.update();
    Blynk.run();
    Blynk.logEvent("color_switch", currentColor);
    static uint8_t startIndex = 0;
    if (paletteName == "Glowing") {
      if (brightness <= inputBrightness && incBright) {
        brightness++;
        if (brightness >= inputBrightness) {
          incBright = false;
          brightness = (uint8_t) inputBrightness - 1;
        }
        if (onoff) {
          FastLED.show();
          FastLED.delay(1000 / UPDATES_PER_SECOND);
        }
      } else if (brightness >= 1 && !incBright) {
        brightness--;
        if (brightness <= 1) {
          incBright = true;
          brightness = (uint8_t) 1;
        }
        if (onoff) {
          FastLED.show();
          FastLED.delay(1000 / UPDATES_PER_SECOND);
        }
      }
    } else {
       startIndex = startIndex + 1;
       if (onoff) {
        FastLED.show();
        FastLED.delay(1000 / UPDATES_PER_SECOND);
      }
    }
    FillLEDsFromPaletteColors(startIndex);
    
}

BLYNK_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_WRITE(SWITCH_PALETTE) {
  if(param.asInt()) 
   {
      paletteId++;
      if (paletteId >=  NUM_OF_PALETTES) {
        paletteId = 0;
      }
      ChangePalette(paletteId);
   }
}

BLYNK_WRITE(SWITCH_BLEND) {
  if(param.asInt()) 
   {
      blendId++;
      if (blendId >=  NUM_OF_BLENDS) {
        blendId = 0;
      }
      ChangeBlend(blendId);
   }
}

BLYNK_WRITE(ONOFF) {
  if(param.asInt()) {
    onoff = true;
  } else {
    onoff = false;
    FastLED.clear();
    FastLED.show();
  }
}

BLYNK_WRITE(BRIGHTNESS_INPUT) {
  inputBrightness = (uint8_t) param.asInt();
  brightness = inputBrightness - 2;
  Blynk.virtualWrite(BRIGHTNESS_OUTPUT, inputBrightness);
}

BLYNK_WRITE(COLOR_SWITCH_INPUT) {
  if(param.asInt()) {
    if (currentColorIndex >=  NUM_OF_COLORS) {
      currentColorIndex = 0;
    }
    currentColorIndex++;
    Blynk.virtualWrite(COLOR_OUTPUT, currentColorText);
  }
}

BLYNK_WRITE(RGB_INPUT) {
  uint8_t red = param[0].asInt();
  uint8_t green = param[1].asInt();
  uint8_t blue = param[2].asInt();
  currentColor = CRGB(red, green, blue);
  Blynk.logEvent("color_switch", currentColor);
  if (paletteId == 7) {
    SetupGlowingPalette();
  }
}

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void ChangePalette(int paletteId)
{
    switch (paletteId) {
        case(0):  
          currentPalette = RainbowColors_p;
          paletteName = "Rainbow";
          break;
        case(1):
          currentPalette = RainbowStripeColors_p;
          paletteName = "Rainbow stripe";
          break;
        case(2):
          SetupPurpleAndGreenPalette();
          paletteName = "Purple and green";
          break;
        case(3):
          SetupBlackAndWhiteStripedPalette();
          paletteName = "Black and white";
          break;
        case(4):
          currentPalette = CloudColors_p;
          paletteName = "Cloud";
          break;
        case(5):
          currentPalette = PartyColors_p;
          paletteName = "Party";
          break;
        case(6):
          currentPalette = myRedWhiteBluePalette_p;
          paletteName = "Red, white and blue";
          break;
        case(7):
          SetupGlowingPalette();
          paletteName = "Glowing";
          break;
    }
    Blynk.logEvent("current_palette", "current palette: "+ paletteName);
    Blynk.virtualWrite(PALETTE, paletteName);
}

void ChangeBlend(int blendId) {
  switch (blendId) {
    case (0) :
      currentBlending = LINEARBLEND;
      blendName = "Linear blend";
      break;
    case (1) :
      currentBlending = NOBLEND;
      blendName = "No blend";
      break;
  }
  Blynk.logEvent("current_blend", "current blend: "+ blendName);
  Blynk.virtualWrite(BLEND, blendName);
}

void SetupBlackAndWhiteStripedPalette()
{
    fill_solid(currentPalette, 16, CRGB::Black);
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}

const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

void SetupGlowingPalette()
{
  fill_solid(currentPalette, 16, currentColor);
}
