#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=59.333335876464844,312.3333225250244
AudioAmplifier           amp1;           //xy=186.3333511352539,231.3333225250244
AudioInputUSB            usb1;           //xy=209.33333587646484,119.00002670288086
AudioAmplifier           amp2;           //xy=231.33333587646484,348.0000104904175
AudioMixer4              mixer2;         //xy=372.3333435058594,209.0000057220459
AudioMixer4              mixer1;         //xy=377.3333396911621,117.00001907348633
AudioOutputI2S           i2s2;           //xy=510.3333435058594,182.33334922790527
AudioConnection          patchCord1(i2s1, 0, amp1, 0);
AudioConnection          patchCord2(i2s1, 1, amp2, 0);
AudioConnection          patchCord3(amp1, 0, mixer1, 1);
AudioConnection          patchCord4(usb1, 0, mixer1, 0);
AudioConnection          patchCord5(usb1, 1, mixer2, 0);
AudioConnection          patchCord6(amp2, 0, mixer2, 1);
AudioConnection          patchCord7(mixer2, 0, i2s2, 1);
AudioConnection          patchCord8(mixer1, 0, i2s2, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=201.3333511352539,525.3333559036255
// GUItool: end automatically generated code



void setup() {
  // put your setup code here, to run once:
  AudioMemory(50);
  amp1.gain(10.0);
  amp2.gain(10.0);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6);
  sgtl5000_1.unmuteLineout();
  sgtl5000_1.lineOutLevel(13);
  sgtl5000_1.unmuteHeadphone();

}

void loop() {
// read the PC's volume setting
  float vol = usb1.volume();

  // use the scaled volume setting.  Delete this for fixed volume.
  mixer1.gain(0, vol);
  mixer2.gain(0, vol);

  delay(100);
}
