#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_TPA2016.h>

// GUItool: begin automatically generated code
AudioInputUSB            usbAudio;           //xy=306.3333053588867,304.3333377838135
AudioInputI2SQuad        i2s_quadIn;      //xy=311.33333587646484,476.3333435058594
AudioAmplifier           ampLOut;           //xy=456.33331298828125,283.3333339691162
AudioAmplifier           ampROut;           //xy=467.3333435058594,336.33333587646484
AudioAmplifier           ampLB;           //xy=509.3333435058594,470.33334159851074
AudioAmplifier           ampLA;           //xy=510.3333435058594,431.33334159851074
AudioAmplifier           ampRA;           //xy=511.3333435058594,511.3333435058594
AudioAmplifier           ampRB;           //xy=512.3333282470703,551.3333282470703
AudioOutputI2S           i2sOut;           //xy=622.3333778381348,308.33330726623535
AudioAnalyzeFFT256       fft256_1;       //xy=698.3333282470703,424.3333282470703
AudioAnalyzeFFT256       fft256_4;       //xy=700.3333282470703,544.3333282470703
AudioAnalyzeFFT256       fft256_3;       //xy=704.3333282470703,499.3333282470703
AudioAnalyzeFFT256       fft256_2;       //xy=708.3333282470703,464.3333282470703
AudioConnection          patchCord1(usbAudio, 0, ampLOut, 0);
AudioConnection          patchCord2(usbAudio, 1, ampROut, 0);
AudioConnection          patchCord3(i2s_quadIn, 0, ampLA, 0);
AudioConnection          patchCord4(i2s_quadIn, 1, ampLB, 0);
AudioConnection          patchCord5(i2s_quadIn, 2, ampRA, 0);
AudioConnection          patchCord6(i2s_quadIn, 3, ampRB, 0);
AudioConnection          patchCord7(ampLOut, 0, i2sOut, 0);
AudioConnection          patchCord8(ampROut, 0, i2sOut, 1);
AudioConnection          patchCord9(ampLB, fft256_2);
AudioConnection          patchCord10(ampLA, fft256_1);
AudioConnection          patchCord11(ampRA, fft256_3);
AudioConnection          patchCord12(ampRB, fft256_4);
AudioControlSGTL5000     sgtl5000;     //xy=333.3333396911621,179.3333568572998
// GUItool: end automatically generated code

Adafruit_TPA2016 speakerAmp = Adafruit_TPA2016();

void setup() {
  Serial.begin(115200);
  AudioMemory(128);

  sgtl5000.enable();
  sgtl5000.volume(0.8);
  sgtl5000.lineOutLevel(29); //1.29 Vpp
  sgtl5000.unmuteLineout();

  //speaker preamp gain
  ampLOut.gain(1);
  ampROut.gain(10);

  //mic preamp gain
  ampLA.gain(10);
  ampLB.gain(10);
  ampRA.gain(10);
  ampRB.gain(10);

  speakerAmp.begin();
  speakerAmp.enableChannel(true, true); //enable left/right
  speakerAmp.setGain(30);
  speakerAmp.setLimitLevelOff();
}

void loop() {
  float vol = usbAudio.volume();
  ampLOut.gain(vol);
  ampROut.gain(vol);
  
  if (fft256_1.available() && fft256_2.available() && fft256_3.available() && fft256_4.available()) {
    float la = fft256_1.read(5); // 6th block of 172Hz increments = 1032Hz
    float lb = fft256_2.read(5);
    float ra = fft256_3.read(5);
    float rb = fft256_4.read(5);

    Serial.print("Low:0.000\tHigh:0.200\tLA:");
    Serial.print(la, 3);
    Serial.print("\tLB:");
    Serial.print(lb, 3);
    Serial.print("\tRA:");
    Serial.print(ra, 3);
    Serial.print("\tRB:");
    Serial.print(rb, 3);
    Serial.print("\n");
  }
}
