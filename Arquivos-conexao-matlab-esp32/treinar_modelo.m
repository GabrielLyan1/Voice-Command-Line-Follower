clear; clc;

fs = 16000;

files = dir("treino/*.wav");

X = [];
Y = [];

afe = audioFeatureExtractor( ...
    'SampleRate', fs, ...
    'mfcc', true, ...
    'mfccDelta', true, ...
    'mfccDeltaDelta', true);

for k = 1:length(files)
    [audio, fs] = audioread(fullfile(files(k).folder, files(k).name));

    audio = audio - mean(audio);
    audio = audio / max(abs(audio));

    feats = extract(afe, audio);
    featVec = mean(feats, 1);

    X = [X; featVec];

    % extrai o número do nome do arquivo
    if contains(files(k).name, "um")
        Y = [Y; 1];
    elseif contains(files(k).name, "dois")
        Y = [Y; 2];
    elseif contains(files(k).name, "tres")
        Y = [Y; 3];
    end
end

knnModel = fitcknn(X, Y, 'NumNeighbors', 3);

save modeloNumeros.mat knnModel

disp("Modelo treinado e salvo com sucesso!");
