% Load audio
[x, Fs] = audioread('CountingWithFan3.m4a');

% LMS Parameters
filterLength = 2048;
mu = 0.001;
refDuration = 4; % seconds
numRefSamples = round(refDuration * Fs);

% Extract reference noise (first 4 seconds)
noiseRef = x(1:numRefSamples);

% Signal to be filtered (after 4 seconds)
signalToFilter = x(numRefSamples+1:end);

% Initializeweights
w = zeros(filterLength, 1);

% Initialize filtered signal array
filteredSignal = zeros(size(signalToFilter));

% Create buffer for reference signal
circularRef = repmat(noiseRef, 3, 1);  % Repeat reference signal 3 times
refLength = length(noiseRef);

% LMS 
for n = filterLength:length(signalToFilter)
    startIdx = mod(n, refLength) + 1;
    if startIdx <= filterLength
        x_n = circularRef(startIdx+refLength-1:-1:startIdx+refLength-filterLength);
    else
        x_n = circularRef(startIdx-1:-1:startIdx-filterLength);
    end
    
    % Calculate filter output
    y = w' * x_n;
    
    % Calculate error
    e = signalToFilter(n) - y;
    
    % Update weights
    w = w + 2 * mu * e * x_n;
    
    % Store filtered sample
    filteredSignal(n) = e;
end

% Combine unfiltered with filtered signal
outputSignal = [x(1:numRefSamples); filteredSignal];

% Save 
audiowrite('FilteredAudio4.wav', outputSignal, Fs);


% Plot time series
t = (0:length(x)-1) / Fs;
figure;
plot(t, outputSignal);
xlabel('Time (seconds)');
ylabel('Amplitude');
title('Filtered Audio Signal');
grid on;

% Plot spectrogram 
windowSize = round(0.05 * Fs);  
overlap = round(windowSize/2);  
nfft = windowSize;

figure;
spectrogram(outputSignal, hamming(windowSize), overlap, nfft, Fs, 'yaxis');
ylim([0 10]); % Limit frequency axis to 10 kHz
title('Spectrogram of Filtered Signal (0-10 kHz)');
colorbar;