#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_TPA2016.h>
#include <String.h>

#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11
#define SDCARD_SCK_PIN   13
byte wavHeader[44];
File audio_file;
unsigned int wavDataSize;

// GUItool: begin automatically generated code
AudioInputI2SQuad        i2s_quadIn;     //xy=178.33339309692383,446.99999618530273
AudioFilterBiquad        bandpassErrorR; //xy=337.33336639404297,246.00000762939453
AudioFilterBiquad        bandpassRefR;   //xy=341.33336639404297,193.00003623962402
AudioAmplifier           ampRB;          //xy=515.3333740234375,554
AudioAmplifier           ampRA;          //xy=518.3333740234375,488.99999809265137
AudioAmplifier           ampLB;          //xy=561.3333778381348,398.99999618530273
AudioAmplifier           ampLA;          //xy=575.3333778381348,314.99999618530273
AudioRecordQueue         queueErrorL;         //xy=774.3333854675293,624.3333768844604
AudioRecordQueue         queueRefL;         //xy=776.3333854675293,574.3333759307861
AudioFilterBiquad        bandpassErrorL; //xy=821.333381652832,477.9999990463257
AudioFilterBiquad        bandpassRefL;   //xy=826.333381652832,393.99999809265137
AudioAmplifier           ampLOut;        //xy=926.3332786560059,218.00000762939453
AudioAmplifier           ampROut;        //xy=926.3332824707031,287.0000696182251
AudioAnalyzeRMS          rmsErrorL;      //xy=998.3333892822266,500
AudioAnalyzeRMS          rmsRefL;        //xy=1001.3333892822266,416.99999809265137
AudioOutputI2S           i2sOut;         //xy=1098.3332061767578,241.00003623962402
AudioConnection          patchCord1(i2s_quadIn, 0, ampLA, 0);
AudioConnection          patchCord2(i2s_quadIn, 1, ampLB, 0);
AudioConnection          patchCord3(i2s_quadIn, 2, ampRA, 0);
AudioConnection          patchCord4(i2s_quadIn, 3, ampRB, 0);
AudioConnection          patchCord5(ampLB, bandpassErrorL);
AudioConnection          patchCord6(ampLB, queueErrorL);
AudioConnection          patchCord7(ampLA, bandpassRefL);
AudioConnection          patchCord8(ampLA, queueRefL);
AudioConnection          patchCord9(bandpassErrorL, rmsErrorL);
AudioConnection          patchCord10(bandpassRefL, rmsRefL);
AudioConnection          patchCord11(ampLOut, 0, i2sOut, 0);
AudioConnection          patchCord12(ampROut, 0, i2sOut, 1);
AudioControlSGTL5000     sgtl5000;       //xy=665.3333778381348,215.00001049041748
// GUItool: end automatically generated code


Adafruit_TPA2016 speakerAmp = Adafruit_TPA2016();

float leftOutputGain = 1.0;
float rightOutputGain = 1.0;

unsigned int recordTime;

void setup() {
    Serial.begin(115200);
    AudioMemory(128);
    sgtl5000.enable();
    sgtl5000.volume(0.8);
    sgtl5000.lineOutLevel(29);       // Set line out to 1.29 Vpp
    sgtl5000.unmuteLineout();
    
    // Speaker and mic gain settings
    ampLOut.gain(1.0);
    ampROut.gain(1.0);
    ampLA.gain(10);
    ampLB.gain(10);
    ampRA.gain(10);
    ampRB.gain(10);
    
    // Configure speaker amplifier
    speakerAmp.begin();
    speakerAmp.enableChannel(true, true);
    speakerAmp.setGain(10);
    speakerAmp.setLimitLevelOff();

    // Configure bandpass filters for 200-2000 Hz
    bandpassRefL.setBandpass(0, 1000.0, 1.0);    // Reference left
    bandpassErrorL.setBandpass(0, 1000.0, 1.0);  // Error left

    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
    if (!SD.begin(SDCARD_CS_PIN)) {Serial.println("Started SD card.");}

    digitalWrite(13, HIGH);
    startAudioRecording();
    recordTime = millis();
}

void loop() {
    // Check if RMS values are available after bandpass filtering
    if (rmsRefL.available() && rmsErrorL.available()) {
        float reference_signal = rmsRefL.read();
        float error_signal = rmsErrorL.read();

        Serial.print(" LA (500Hz): ");
        Serial.print(reference_signal, 3);
        Serial.print("\t LB (500Hz): ");
        Serial.println(error_signal, 3);

        // Update the adaptive filter 
        float mu = 0.01;                // Step size for LMS
        float correction = mu * reference_signal * error_signal;
        
        leftOutputGain -= correction;
        rightOutputGain -= correction;
        
        // Update gains 
        leftOutputGain = constrain(1.0, 0.0, 1.0);
        rightOutputGain = constrain(1.0, 0.0, 1.0);
        ampLOut.gain(1.0);
        ampROut.gain(1.0);
    }
    
    if (millis() - recordTime < 10000) {
      continueAudioRecording();
    } else {
      stopAudioRecording();
      Serial.println("Finished recording.");
      digitalWrite(13, LOW);
      while (1) {}
    }
}

void startAudioRecording() {
  int fileNum = 0;
  char fileName[16];
  do {
    String fileNamePrefix = "audio";
    fileNamePrefix.concat(fileNum);
    fileNamePrefix.concat(".wav");
    fileNamePrefix.toCharArray(fileName, 16);
    fileNum++;
  } while (SD.exists(fileName));
  audio_file = SD.open(fileName, FILE_WRITE);
  if (!audio_file) {
    Serial.println("Error opening file");
    while (1) {}
  }
  audio_file.write(wavHeader, 44); // Outputs "blank" wave header
  wavDataSize = 0;
  queueRefL.begin();
  queueErrorL.begin();
}

void continueAudioRecording() {
  if (queueRefL.available() >= 1 && queueErrorL.available() >= 1) {
    byte buffer[512], lBuffer[256], rBuffer[256]; 
    memcpy(lBuffer, queueRefL.readBuffer(), 256);
    queueRefL.freeBuffer();
    memcpy(rBuffer, queueErrorL.readBuffer(), 256);
    queueErrorL.freeBuffer();
    for (int i = 0; i < 128; i++) {
      buffer[4*i] = lBuffer[2*i];
      buffer[4*i+1] = lBuffer[2*i+1];
      buffer[4*i+2] = rBuffer[2*i];
      buffer[4*i+3] = rBuffer[2*i+1];
    }
    audio_file.write(buffer, 512); // SD card library apparently is more efficient when writing 1 block (512b) at a time
    wavDataSize+=512;
  }
}

void stopAudioRecording() {
  queueRefL.end();
  queueErrorL.end();
   while (queueRefL.available() > 0 && queueErrorL.available() > 0) {
      continueAudioRecording();
  }
  CreateWavHeader(wavHeader, wavDataSize); // generate a valid wav header for a given amount of wav data
  audio_file.seek(0); // after finishing recording, go back to the beginning of the file where we put the blank wav header
  audio_file.write(wavHeader, 44); //and write the generated, correct wav header in its place
  audio_file.close(); // then close and complete the file to get ready for the new one
}

void CreateWavHeader(byte* header, int waveDataSize){
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSizeMinus8 = waveDataSize + 44 - 8;
  header[4] = (byte)(fileSizeMinus8 & 0xFF);
  header[5] = (byte)((fileSizeMinus8 >> 8) & 0xFF);
  header[6] = (byte)((fileSizeMinus8 >> 16) & 0xFF);
  header[7] = (byte)((fileSizeMinus8 >> 24) & 0xFF);
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 0x10;  // linear PCM
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;
  header[20] = 0x01;  // linear PCM
  header[21] = 0x00;
  header[22] = 0x02;  // stereo
  header[23] = 0x00;
  header[24] = 0x44;  // sampling rate 44100
  header[25] = 0xAC;
  header[26] = 0x00;
  header[27] = 0x00;
  header[28] = 0x10;  // Byte/sec = 44100x2x2 = 176400
  header[29] = 0xB1;
  header[30] = 0x02;
  header[31] = 0x00;
  header[32] = 0x04;  // 16bit stereo
  header[33] = 0x00;
  header[34] = 0x10;  // 16bit
  header[35] = 0x00;
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  header[40] = (byte)(waveDataSize & 0xFF);
  header[41] = (byte)((waveDataSize >> 8) & 0xFF);
  header[42] = (byte)((waveDataSize >> 16) & 0xFF);
  header[43] = (byte)((waveDataSize >> 24) & 0xFF);
}