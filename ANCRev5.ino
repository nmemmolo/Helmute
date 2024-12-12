//ANC using fxLMS, must be used with SecondaryPathRev10Streamlined.ino

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Structure for complex numbers
struct Complex {
    float real;
    float imag;
    
    Complex(float r = 0.0f, float i = 0.0f) : real(r), imag(i) {}
    
    Complex operator*(const Complex& other) const {
        return Complex(real * other.real - imag * other.imag,
                      real * other.imag + imag * other.real);
    }
    
    Complex conj() const {
        return Complex(real, -imag);
    }
};

// Structure to hold/store transfer function data HAS TO MATCH STRUCTURE IN SECONDARY PATH PROGRAM
struct TransferFunctionData {
    float frequency;
    float magnitude;
    float phase;              //wrapped phase (constrains phase angle between 0 and 2pi, susectible to phase jump, which causes dicontinuities at multiples to 2pi, i.e. 4pi becomes 0)
    float unwrappedPhase;     // unwrapped phase (often used in developing transfer functions [why we are using it here], avoids discontinuities described above)
};

// GUItool: begin automatically generated code
AudioInputI2SQuad        i2s_quadIn;      // xy=159.19998931884766,279.9999885559082
AudioFilterBiquad        filterRefL;      // xy=432.2000045776367,218.9999885559082     High-pass filter for left reference mic  
AudioFilterBiquad        filterErrL;      // xy=387.2000045776367,284.9999895095825     High-pass filter for left error mic
AudioAmplifier           ampRefL;         // xy=600.2000350952148,231.0000057220459     Left reference mic amplifier
AudioAmplifier           ampErrL;         // xy=572.2000350952148,290.99999046325684    Left error mic amplifier
AudioSynthNoiseWhite     noise;           // xy=104.19998931884766,179.00000381469727   Noise source for anti-noise   
AudioFilterBiquad        bandpass;        // xy=275.1999740600586,154.00000381469727    Bandpass filter for noise 
AudioAmplifier           ampNoise;        // xy=445.1999740600586,132.00000476837158    Noise amplitude control
AudioMixer4              mixer;           // xy=646.2000350952148,128.00000381469727    Mixer for combining signals
AudioAmplifier           ampOut;          // xy=797.2000389099121,118.00000953674316    Final output amplifier
AudioOutputI2S           i2sOut;          // xy=948.2000427246094,97.00000476837158
AudioAnalyzeFFT256       fftRefL;          // xy=737.2000389099121,234.00000381469727    FFT for reference signal
AudioAnalyzeFFT256       fftErrL;          // xy=736.2000389099121,290.99999046325684    FFT for error signal
AudioConnection          patchCord1(i2s_quadIn, 0, filterRefL, 0);  // Left reference mic to DC filter
AudioConnection          patchCord2(i2s_quadIn, 1, filterErrL, 0);  // Left error mic to DC filter
AudioConnection          patchCord3(filterRefL, ampRefL);           // DC Filter to amp
AudioConnection          patchCord4(filterErrL, ampErrL);           // DC Filter to amp
AudioConnection          patchCord5(ampRefL, fftRefL);               // L mic amp to FFT
AudioConnection          patchCord6(ampErrL, fftErrL);               // R mic amp to FFT
AudioConnection          patchCord7(noise, bandpass);               // antioise to bandpass
AudioConnection          patchCord8(bandpass, ampNoise);            // Bandpass to antinoise amp
AudioConnection          patchCord9(ampNoise, 0, mixer, 0);         // antinoise amp to mixer
AudioConnection          patchCord10(mixer, ampOut);                // Mixer to output amp
AudioConnection          patchCord11(ampOut, 0, i2sOut, 0);         // Output amp to left speaker
AudioControlSGTL5000     sgtl5000;           //xy=152.19998931884766,47.000003814697266   Teensy Audio shield


#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define NUM_BINS 256
#define MAX_SECONDARY_PATH_POINTS 10
#define TARGET_FREQ 516       //Frequency ANC is focused on
#define TARGET_BIN 3          //Bin where 516 Hz in in FFT256
#define NOISE_AMP 0.5      
#define MU 0.0001f          // Step size, how much weights are adjusted each loop, 0.0001 conservative, reduces instability
#define MAX_WEIGHT 5.0f     //Max weight value, prevents system gain from getting out of control
#define REF_THRESHOLD 4.0f  // Ignore signals below this
#define MAX_GAIN 0.8f      // Maximum gain limit
#define CONTROL_SCALE 0.1f // Scaling factor for control signal

// Function to limit weight values
void limitWeight(Complex& w) {
    // Limit real component
    if (w.real > MAX_WEIGHT) w.real = MAX_WEIGHT;
    else if (w.real < -MAX_WEIGHT) w.real = -MAX_WEIGHT;
    
    // Limit imaginary component
    if (w.imag > MAX_WEIGHT) w.imag = MAX_WEIGHT;
    else if (w.imag < -MAX_WEIGHT) w.imag = -MAX_WEIGHT;
}

// Global variables
TransferFunctionData secondaryPath[MAX_SECONDARY_PATH_POINTS];
int numSecondaryPathPoints = 0;

// FxLMS variables
Complex w(0.0f, 0.0f);    // Filter weight for target frequency
Complex spf;              // Secondary path for target frequency

// Function to load secondary path data from SD card
bool loadSecondaryPath() {
    if (!SD.exists("SecondaryPath.txt")) {
        Serial.println("Secondary path file not found!");
        return false;
    }

    File dataFile = SD.open("SecondaryPath.txt");
    if (!dataFile) {
        Serial.println("Error opening secondary path file!");
        return false;
    }

    // Skip header line
    dataFile.readStringUntil('\n');

    // Read secondary path data from SecondaryPath.txt
    numSecondaryPathPoints = 0;
    while (dataFile.available() && numSecondaryPathPoints < MAX_SECONDARY_PATH_POINTS) {
        String line = dataFile.readStringUntil('\n');
        
        // Parse tab-separated values
        int tabIndex = line.indexOf('\t');
        line = line.substring(tabIndex + 1);
        
        secondaryPath[numSecondaryPathPoints].frequency = line.toFloat();
        tabIndex = line.indexOf('\t');
        line = line.substring(tabIndex + 1);
        
        secondaryPath[numSecondaryPathPoints].magnitude = line.toFloat();
        tabIndex = line.indexOf('\t');
        line = line.substring(tabIndex + 1);
        
        secondaryPath[numSecondaryPathPoints].phase = line.toFloat();
        tabIndex = line.indexOf('\t');
        line = line.substring(tabIndex + 1);
        
        secondaryPath[numSecondaryPathPoints].unwrappedPhase = line.toFloat();
        
        numSecondaryPathPoints++;
    }
    
    dataFile.close();
    
    Serial.print("Loaded ");
    Serial.print(numSecondaryPathPoints);
    Serial.println("secondary path points");
    
    return true;
}

// Convert secondary path magnitude and phase to complex form
void initializeSecondaryPath() {
    // Find secondary path data for target bin (516 = bin 3)
    for (int i = 0; i < numSecondaryPathPoints; i++) {
        if (abs(secondaryPath[i].frequency - TARGET_FREQ) < 1.0f) {
            float mag = secondaryPath[i].magnitude;
            float phase = secondaryPath[i].phase;
            spf.real = mag * cos(phase);
            spf.imag = mag * sin(phase);
            Serial.println("Secondary path ready");
            Serial.print("Magnitude: "); Serial.println(mag, 6);
            Serial.print("Phase: "); Serial.println(phase, 6);
            return;
        }
    }
    Serial.println("Warning: Target frequency not found in secondary path data!");
}

// Get complex form of FFT
Complex getComplexFFT(AudioAnalyzeFFT256& fft, int bin) {
    return Complex(fft.output[bin*2], fft.output[bin*2 + 1]);
}

void setup() {
    Serial.begin(115200);
    AudioMemory(512);
    
    // Initialize SD card
    if (!SD.begin(SDCARD_CS_PIN)) {
        Serial.println("SD card initialization failed!");
        while (1);
    }
    Serial.println("SD card initialized successfully");
    
    // Configure audio shield
    sgtl5000.enable();
    sgtl5000.volume(0.7);
    sgtl5000.lineOutLevel(29);
    
    // Configure high-pass filters (100 Hz) to remove DC and better operate within speaker specifications
    filterRefL.setHighpass(0, 100, 0.707);
    filterErrL.setHighpass(0, 100, 0.707);
    
    // Configure bandpass filter to focus antinoise at 516 Hz
    bandpass.setBandpass(0, TARGET_FREQ, 10);
    
    // Configure amps and mixer
    ampRefL.gain(1);    // Set appropriate mic gain, Ian said something about recording working best when mic gain =1
    ampErrL.gain(1);
    ampNoise.gain(0);    // Start with noise off
    ampOut.gain(1.0);
    mixer.gain(0, 1.0);  // Noise input channel
    
    // Initialize noise source
    noise.amplitude(NOISE_AMP);
    
    // Set window function for FFTs: prevents spectral leakage, where energy from one frequency spreads into neighboring frequencies
    fftRefL.windowFunction(AudioWindowHanning256);
    fftErrL.windowFunction(AudioWindowHanning256);
    
    // Load secondary path data
    if (!loadSecondaryPath()) {
        Serial.println("Failed to load secondary path data!");
        while(1);
    }
    
    initializeSecondaryPath();
    delay(1000);  // System stabilization
}


// For Loop: x=reference signal, xf = filtered reference signal (filter w/ secondary path), e = error signal, y = control signal, w = filter weights, spf = secondary path transfer function
void loop() {
    if (fftRefL.available() && fftErrL.available()) {
        // Get complex FFT data
        Complex x = getComplexFFT(fftRefL, TARGET_BIN);
        Complex e = getComplexFFT(fftErrL, TARGET_BIN);
        
        // Calculate signal magnitudes |x|^2
        float refLevel = sqrt(x.real * x.real + x.imag * x.imag);
        float errLevel = sqrt(e.real * e.real + e.imag * e.imag);
        
        // Declare control variable outside the if statement
        float limitedControl = 0;  // Initialize to 0

        // Only process if reference is above threshold, avoids system generating antinoise when no external noise is present
        if (refLevel > REF_THRESHOLD) {
            // Calculate filtered reference signal
            Complex xf = x * spf;
            
            // Calculate control signal
            Complex y = x * w;
            
            // Calculate control magnitude and scale
            float controlMag = sqrt(y.real * y.real + y.imag * y.imag);
            float scaledControl = controlMag * CONTROL_SCALE;
            
            // Limit gain, to prevent system discomfort/ causing hearing loss
            limitedControl = min(scaledControl, MAX_GAIN);
            
            ampNoise.gain(limitedControl);     // Update antinoise output
            
            // Update weights
            Complex xf_conj = xf.conj();
            Complex update = e * xf_conj;
            w.real -= MU * update.real;
            w.imag -= MU * update.imag;
            limitWeight(w);
        } else {
            ampNoise.gain(0);  // Turn off anti-noise if external noise below threshold
        }
        
        // Print process info for debugging
        Serial.print("Ref:"); Serial.print(refLevel, 6);        //Ref signal
        Serial.print("\tErr:"); Serial.print(errLevel, 6);      //Error signal
        Serial.print("\tW_real:"); Serial.print(w.real, 6);     // Real part of weights
        Serial.print("\tW_imag:"); Serial.print(w.imag, 6);     //imaginary part of weights

        Serial.println();
    }
}

