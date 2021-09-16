#include <arduinoFFT.h>
#include "pins.h"


#define SAMPLES         512          // Must be a power of 2
#define SAMPLING_FREQ   40000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define NUM_BANDS       16            // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands

#define NOISE           4000           // Used as a crude noise filter, values below this are ignored. Try lowering this if you dont pick up audio data.
#define SCALE           4000

// Sampling and FFT stuff
unsigned int sampling_period_us;
// The length of these arrays must be >= NUM_BANDS
int bandValues[NUM_BANDS]; 
int gbandValues[NUM_BANDS];
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime;
arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);


TaskHandle_t samplingTask;
SemaphoreHandle_t bandLock;

void samplingLoop(void * parameter) { //Our function to sample audio data and run repeatedly on core 0
  for (;;) {
    // Reset bandValues[]
    for (int i = 0; i < NUM_BANDS; i++) {
      bandValues[i] = 0;
    }

    // Sample the audio pin
    for (int i = 0; i < SAMPLES; i++) {
      newTime = micros();
      vReal[i] = analogRead(AUDIO_IN_PIN); // A conversion takes about 9.7uS on an ESP32
      vImag[i] = 0;
      while ((micros() - newTime) < sampling_period_us) {
        /* chill */
      }
    }

    // Compute FFT
    FFT.DCRemoval();
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(FFT_FORWARD);
    FFT.ComplexToMagnitude();

    // Analyse FFT results
    for (int i = 2; i < (SAMPLES / 2); i++) {    // Don't use sample 0 and only first SAMPLES/2 are usable. Each array element represents a frequency bin and its value the amplitude.
      if (vReal[i] > NOISE) {                    // Add a crude noise filter

        //16 bands, 12kHz top band
        if (i <= 2 )           bandValues[0]  += (int)vReal[i];
        if (i > 2   && i <= 3  ) bandValues[1]  += (int)vReal[i];
        if (i > 3   && i <= 5  ) bandValues[2]  += (int)vReal[i];
        if (i > 5   && i <= 7  ) bandValues[3]  += (int)vReal[i];
        if (i > 7   && i <= 9  ) bandValues[4]  += (int)vReal[i];
        if (i > 9   && i <= 13 ) bandValues[5]  += (int)vReal[i];
        if (i > 13  && i <= 18 ) bandValues[6]  += (int)vReal[i];
        if (i > 18  && i <= 25 ) bandValues[7]  += (int)vReal[i];
        if (i > 25  && i <= 36 ) bandValues[8]  += (int)vReal[i];
        if (i > 36  && i <= 50 ) bandValues[9]  += (int)vReal[i];
        if (i > 50  && i <= 69 ) bandValues[10] += (int)vReal[i];
        if (i > 69  && i <= 97 ) bandValues[11] += (int)vReal[i];
        if (i > 97  && i <= 135) bandValues[12] += (int)vReal[i];
        if (i > 135 && i <= 189) bandValues[13] += (int)vReal[i];
        if (i > 189 && i <= 264) bandValues[14] += (int)vReal[i];
        if (i > 264          ) bandValues[15] += (int)vReal[i];
      }
    }

    //Copy values to global - By using another array to hold these values our LED writing task on core 1 doesn't have to wait for this task to finish sampling. It will only have to wait for these values to be copied which should be faster
    xSemaphoreTake(bandLock, portMAX_DELAY);
    for (int x = 0; x < NUM_BANDS; x++) {
      gbandValues[x] = bandValues[x];
    }
    xSemaphoreGive(bandLock);
  }
}
