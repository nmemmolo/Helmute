% Read the audio files
[counting, fs1] = audioread('NoisyCounting1.wav');
[filtered, fs2] = audioread('FilteredAudio5.wav');

% Ensure both audio files have the same sample rate
if fs1 ~= fs2
    filtered = resample(filtered, fs1, fs2);
end

% Calculate pause duration in samples
pauseDuration = 0.5; % seconds
pauseSamples = round(pauseDuration * fs1);

% Create pause segment (silence)
pauseSegment = zeros(pauseSamples, size(counting, 2));

% Combine the audio files with pause
combinedAudio = [counting; pauseSegment; filtered];

% Write the combined audio to a new file
audiowrite('CombinedAudio1.wav', combinedAudio, fs1);

% Create visualization
figure('Position', [100 100 1200 800]);

% Time series plot
subplot(2,1,1);
t = (0:length(combinedAudio)-1)/fs1;
plot(t, combinedAudio);
grid on;
title('Time Series of Combined Audio');
xlabel('Time (seconds)');
ylabel('Amplitude');

% Spectrogram
subplot(2,1,2);
window = round(0.05 * fs1); % 0.05 second window
noverlap = round(window * 0.5); % 50% overlap
nfft = 2^nextpow2(window); % Next power of 2 from window length

spectrogram(combinedAudio, window, noverlap, nfft, fs1, 'yaxis');
title('Spectrogram of Combined Audio');
colorbar;

% Adjust figure properties
colormap('jet');
set(gcf, 'Color', 'w');