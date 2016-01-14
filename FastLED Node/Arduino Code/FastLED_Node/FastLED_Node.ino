#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_NODE_ID 9
#include <SPI.h>
#include <MySensor.h>

#include "FastLED.h"
FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    3
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    38
CRGB leds[NUM_LEDS];

// globals for cool color functions
uint8_t gHue = 0;
uint8_t colour ;
int step = -1;
int center = 0; 


unsigned int currentSpeed = 0 ;
int brightness = 96;
unsigned int requestedMode = 0 ;
int messageType = 0 ;
int previousMessageType = -1;

String hexColor = "000000" ;

unsigned long previousTime = 0 ;

void setup() {
  delay(1000);
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  FastLED.show();
}

void presentation() {
  sendSketchInfo("FastLED Node", "1.0");
  present(0, S_RGB_LIGHT, "Makes strip said color", false);
}

void loop() {
  switch (messageType) {

    //************** CASE 1 **************
    case (1):
      Serial.print("Hex color override: "); Serial.println(hexColor);
      colorWipe();
      messageType = 0 ;
      break;

    //************** CASE 2 **************
    case (2):
      if ( (previousTime + (long) currentSpeed ) < millis() ) {
        if (requestedMode == 2) {rainbow(); FastLED.show(); }
        if (requestedMode == 3) {rainbowWithGlitter();FastLED.show(); }
        if (requestedMode == 4) {ripple();FastLED.show(); }
        if (requestedMode == 5) {confetti();FastLED.show(); }
        if (requestedMode == 6) {sinelon();FastLED.show(); }
        if (requestedMode == 7) {bpm();FastLED.show(); }
        if (requestedMode == 8) {juggle();FastLED.show(); }
        previousTime = millis();
      }
      break;

    //************** CASE 3 **************
    case (3):   // Adjust timing of case 2 using non-blocking code (no DELAYs)
      Serial.print("Case 3 received. Speed set to: "); Serial.print(currentSpeed); Serial.println(" ms.");
      messageType = 2; 
      break;

    //************** CASE 4 **************
    case (4):   // Adjust brightness of whole strip of case 2 using non-blocking code (no DELAYs)
      Serial.print("Case 4 received. Brightness set to: "); Serial.println(brightness);
      FastLED.setBrightness(brightness); FastLED.show();
      messageType = previousMessageType ; // We get off type 4 ASAP
      break;



  }
}

void receive(const MyMessage &message) {
  Serial.println("Message received: ");

  if (message.type == V_RGB) { 
    messageType = 1 ;
    hexColor = message.getString();
    Serial.print("RGB color: "); Serial.println(hexColor);
    }
    
  if (message.type == V_VAR1) { 
    requestedMode = message.getInt();
    Serial.println(requestedMode);
    messageType = 2 ;
    Serial.print("Neo mode: "); Serial.println(requestedMode);
  }

  if (message.type == V_VAR2) { // This line is for the speed of said mode
    currentSpeed = message.getInt() ;
    Serial.println(currentSpeed);
    messageType = 3 ;
    Serial.print("Neo speed: "); Serial.println(currentSpeed);
  }

  if (message.type == V_VAR3) { // This line is for the brightness of said mode
    brightness = message.getInt() ;
    //if(brightness > 255) {brightness = 255;}
    //if(brightness < 0) {brightness = 0;}
    Serial.println(brightness);
    previousMessageType = messageType;
    messageType = 4 ;
    Serial.print("Neo brightness: "); Serial.println(brightness);
  }
}


//********************** FastLED FUNCTIONS ***********************


void colorWipe() {
  for (int i = 0; i < NUM_LEDS ; i++) {
    leds[i] = strtol( &hexColor[0], NULL, 16);
    FastLED.show();
  }
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue++, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  fill_rainbow( leds, NUM_LEDS, gHue++, 7);
  FastLED.show();
  addGlitter(80);
}

void ripple() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(gHue++, 255, 15);  // Rotate background colour.
  switch (step) {
    case -1:                                                          // Initialize ripple variables.
      center = random(NUM_LEDS);
      colour = random8();
      step = 0;
      break;

    case 0:
      leds[center] = CHSV(colour, 255, 255);                          // Display the first pixel of the ripple.
      step ++;
      break;

    case 16:                                                    
      step = -1;
      break;

    default:
      leds[(center + step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, 255/step*2);
      leds[(center - step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, 255/step*2);
      step ++;                                                        
      break;  
  }
}


void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

