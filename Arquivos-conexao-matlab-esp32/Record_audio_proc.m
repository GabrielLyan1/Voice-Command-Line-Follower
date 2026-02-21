clc; close all; clear;

%set de taxa de amostragem típica para fala e duração do audio
fs = 16000;
duracao = 1.5;

% gravação do áudio e salvamento local
rec = audiorecorder(fs, 16, 1);
disp("Fale um número...");
recordblocking(rec, duracao);

audio = getaudiodata(rec);
audiowrite("fala.wav", audio, fs);

[audio, fs] = audioread("fala.wav");

% pré-processamento do audio
audio = audio - mean(audio);
audio = audio / max(abs(audio));

% ------PLOTS comparativos--------

t = (0:length(audio)-1)/fs;

figure
plot(t, audio)
xlabel("Tempo (s)")
ylabel("Amplitude")
title("Sinal de áudio no tempo")
grid on

N = length(audio);
Y = fft(audio);
f = (0:N-1)*(fs/N);

figure
plot(f(1:N/2), abs(Y(1:N/2)))
xlabel("Frequência (Hz)")
ylabel("Magnitude")
title("Espectro de frequência")
grid on

figure
spectrogram(audio, hamming(256), 200, 512, fs, 'yaxis')
title("Espectrograma do sinal de fala")

% Extraindo os coeficientes mel cepstrais do audio
afe = audioFeatureExtractor( ...
    'SampleRate', fs, ...
    'mfcc', true, ...
    'mfccDelta', true, ...
    'mfccDeltaDelta', true);

features = extract(afe, audio);
featVec = mean(features, 1);

%classificação via modelo treinado
load modeloNumeros.mat   % modelo treinado antes
numero = predict(knnModel, featVec);

disp("Número reconhecido: " + numero);


% ===== COMUNICAÇÃO SERIAL =====
delete(serialportfind);
pause(1);

s = serialport("COM4", 115200);
configureTerminator(s, "LF");
flush(s);

% Aguarda Arduino ficar pronto
disp("Aguardando Arduino...");
while true
    msg = readline(s);
    if contains(msg, "READY")
        disp("Arduino pronto!");
        break;
    end
end

% Envia número
write(s, string(numero) + newline, "string");

% Aguarda confirmação
resposta = readline(s);
disp("Resposta do Arduino: " + resposta);

if startsWith(resposta, "OK")
    numeroRecebido = extractAfter(resposta, ",");
    disp("Número recebido com sucesso: " + numeroRecebido);
else
    disp("Erro na confirmação!");
end