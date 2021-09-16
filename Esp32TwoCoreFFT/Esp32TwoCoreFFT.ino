#define DEBUG false
#define CLIENT_NAME "IRA-FFT"

#include <FastLED.h>
#include "net.h"
#include "sampling.h"
#include "pins.h"


#define COLOR_ORDER     GRB           // If colours look wrong, play with this
#define CHIPSET         WS2812B       // LED strip type


#define NUM_LEDS       300           // Total number of LEDs
#define BRIGHTNESS      100           //Brightness
#define FRAMES_PER_SECOND 120         //Used to insert delay


//Create our LED Array
CRGB leds[NUM_LEDS];

//Used to adjust hue of our beat bar
uint8_t colorTimer = 0;

int height = 0;
int prev = 0; //used for smoothing out the bar with weighted average
int total = 0;


void setup() {
  Serial.begin(115200);
  Serial.println("init");
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  setup_wifi();


  //Calculate Sampling Period
  
  bandLock = xSemaphoreCreateMutex();


  //Begin our audio sampling task on core 1
  xTaskCreatePinnedToCore(samplingLoop, "Sampling Task", 10000, NULL, 1, &samplingTask, 1);

}



void loop() {//Loop is defualt to core 1 on esp32

  //Fade the leds
  fadeToBlackBy(leds, NUM_LEDS, 50);

  //Take the lock
  xSemaphoreTake(bandLock, portMAX_DELAY);

  //Figure out how much total "Volume" is present by adding all band values together
  total = 0;
  for (int x = 0; x < NUM_BANDS; x++) {
    total += gbandValues[x];
  }
  //  Serial.println(total);
  //We are done with bandValues - Free the lock
  xSemaphoreGive(bandLock);

  //Adjust this constant to change how far out the bar goes
  height = total / SCALE;

  //Make sure the bar does not go outside our LED array
  height = min(height, (NUM_LEDS - 1) / 2);

  //Weighted Average to smooth out how the bar grows and shrinks
  height = (prev * 2 + 3 * height) / 5;
  prev = height;

  //Write the leds
  for (int x = 0; x < height; x++) {
    leds[(NUM_LEDS) / 2 + x] = CHSV(colorTimer, 255, 255);
    leds[(NUM_LEDS) / 2 - x] = CHSV(colorTimer, 255, 255);
  }

  // Cycle the color slowly
  EVERY_N_MILLISECONDS(200) {
    colorTimer++;
  }


  FastLED.show();

  if (DEBUG) {
    for (int x = 0; x < NUM_BANDS; x++) {
      Serial.print(gbandValues[x]);
      Serial.print(" ");
    }
    Serial.println();
  }

}
