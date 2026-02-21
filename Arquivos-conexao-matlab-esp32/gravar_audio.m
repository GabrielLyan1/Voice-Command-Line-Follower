clear; clc;

fs = 16000;       % taxa de amostragem
duracao = 1.2;    % segundos
numero = "dois";    % <-- MUDE AQUI
rec = audiorecorder(fs, 16, 1);

for repeticao=1:10
    disp("Fale o número: " + numero);
    recordblocking(rec, duracao);

    audio = getaudiodata(rec);

    nomeArquivo = "treino/" + numero + "_" + repeticao + ".wav";
    audiowrite(nomeArquivo, audio, fs);

    disp("Áudio salvo em: " + nomeArquivo);
    pause(0.5)
    clc;
end
