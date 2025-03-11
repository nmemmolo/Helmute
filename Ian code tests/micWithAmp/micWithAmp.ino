#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_TPA2016.h>
#include <arm_math.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=162.20000076293945,388.00001525878906
AudioAmplifier           amp2;           //xy=313.20000076293945,555.0000762939453
AudioAmplifier           amp1;           //xy=360.2000045776367,322.00001430511475
AudioAnalyzeFFT1024      fft1024_2;      //xy=462.200008392334,399.20002603530884
AudioAnalyzeFFT1024      fft1024_1;      //xy=553.2000350952148,576.2000274658203
AudioOutputI2S           i2s2;           //xy=825.2000389099121,309.0000123977661
AudioConnection          patchCord1(i2s1, 0, amp1, 0);
AudioConnection          patchCord2(i2s1, 1, amp2, 0);
AudioConnection          patchCord3(amp2, fft1024_1);
AudioConnection          patchCord4(amp1, fft1024_2);
AudioConnection          patchCord5(amp1, 0, i2s2, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=391.1999969482422,713.9999847412109
// GUItool: end automatically generated code

Adafruit_TPA2016 amp = Adafruit_TPA2016();

void setup() {
  Serial.begin(115200);

  AudioMemory(50);
  amp1.gain(1.0);
  amp2.gain(1.0);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  sgtl5000_1.unmuteLineout();
  sgtl5000_1.lineOutLevel(29);
  sgtl5000_1.unmuteHeadphone();

//setting up TPA2016 amp

  if (amp.begin() == false) //Begin communication over I2C
  {
    Serial.println("The device did not respond. Please check wiring.");
    while (1); //Freeze
  }
  Serial.println("Device is connected properly.");

  amp.enableChannel(true, true);
  amp.setGain(30);
  amp.setLimitLevel(16);
}

void loop() {
   if (fft1024_1.available() && fft1024_2.available()) {
    float left = fft1024_1.read(5);
    float right = fft1024_2.read(5);
    Serial.print("0.200\t");
    Serial.print(left, 3);
    Serial.print("\t");
    Serial.print(right, 3);
    Serial.print("\n");

  }
}