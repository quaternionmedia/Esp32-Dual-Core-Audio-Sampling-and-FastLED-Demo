#define DEBUG false
#define CLIENT_NAME "IRA-FFT"


#include "net.h"
#include "sampling.h"
#include "pins.h"
#include "fx.h"

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

void loop() {
  fft_mirrored_bars();
}
