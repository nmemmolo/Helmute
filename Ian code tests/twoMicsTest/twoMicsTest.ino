#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

//code (somewhat) based on SPH0645 test code example

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=115.33333587646484,196.33334827423096
AudioAmplifier           amp1;           //xy=277.3333396911621,168.33335494995117
AudioAmplifier           amp2;           //xy=280.3333282470703,248.6666615009308
AudioAnalyzeFFT256       fft256_2;       //xy=416.33333587646484,257.66667127609253
AudioAnalyzeFFT256       fft256_1;       //xy=422.3333282470703,149.6666615009308
AudioConnection          patchCord1(i2s1, 0, amp1, 0);
AudioConnection          patchCord2(i2s1, 1, amp2, 0);
AudioConnection          patchCord3(amp1, fft256_1);
AudioConnection          patchCord4(amp2, fft256_2);
// GUItool: end automatically generated code

void setup() {
  // put your setup code here, to run once:
  //idk i think 256 bytes of memory should be good
  AudioMemory(256);
  //make these both gain of 10 because whatever
  amp1.gain(10);
  amp2.gain(10);
}

//fft256 is in 172Hz increments so use 6th block (5th index) for 1032Hz

void loop() {
  if (fft256_1.available() && fft256_2.available()) {
    float left = fft256_1.read(5);
    float right = fft256_2.read(5);
    Serial.print("0.200\t");
    Serial.print(left, 3);
    Serial.print("\t");
    Serial.print(right, 3);
    Serial.print("\n");
  }
}
