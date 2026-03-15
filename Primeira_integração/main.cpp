#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "Config.h"
#include "Robo_logica.h"

// Objetos Globais
WiFiServer server(TCP_PORT);
WiFiClient client;
Adafruit_PN532 nfc(PN532_SDA, PN532_SCL);

// Variáveis de Estado
float erroAnterior = 0; 
unsigned long lastNFCScan = 0;

void setup() {
    Serial.begin(115200);
    
    setupHardware();

    // Inicializa WiFi em modo Access Point
    WiFi.softAP(WIFI_SSID, WIFI_PASS);
    server.begin();
    Serial.println("WiFi AP Iniciado.");

    // Inicializa NFC
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (versiondata) {
        nfc.SAMConfig();
        Serial.println("NFC PN532 Detectado.");
    } else {
        Serial.println("Erro: PN532 nao encontrado.");
    }
}

void loop() {
    // 1. Gerencia comandos via WiFi (Servidor TCP)
    processarWiFi();

    // 2. Executa a lógica do Seguidor de Linha (PID)
    executarSeguidor();

    // 3. Verifica NFC a cada 500ms (para não travar o PID)
    if (millis() - lastNFCScan > 333) { // 3x por segundo
        verificarNFC();
        lastNFCScan = millis();
    }
}

/* ================================================================
 * IMPLEMENTAÇÃO DA LÓGICA
 * ================================================================ */

void setupHardware() {
    for (int i = 0; i < NUM_SENSORES; i++) {
        pinMode(PINS_SENSORES[i], INPUT);
    }
    pinMode(MOT_IN1, OUTPUT); pinMode(MOT_IN2, OUTPUT);
    pinMode(MOT_IN3, OUTPUT); pinMode(MOT_IN4, OUTPUT);
    
    analogReadResolution(10); // 0 a 1023
}

void executarSeguidor() {
    int s[NUM_SENSORES];
    bool linhaDetectada = false;

    // Leitura dos sensores
    for (int i = 1; i < NUM_SENSORES-1; i++) {
        int leitura = analogRead(PINS_SENSORES[i]);
        s[i] = (leitura <= ANALOG_THRESHOLD) ? 1 : 0;
        if (s[i]) linhaDetectada = true;
    }

    if (linhaDetectada) {
        // Cálculo do erro por média ponderada
        static const int pesos[NUM_SENSORES] = {0, -4, -2, -1, 0, 1, 2, 4, 0};
        float num = 0;
        int den = 0;

        for (int i = 1; i < NUM_SENSORES-1; i++) {
            num += s[i] * pesos[i];
            den += s[i];
        }

        float erro = num / den;
        erroAnterior = erro;

        float correcao = calcularPID(erro);
        controlarMotores(NOMINAL_SPEED + (int)correcao, NOMINAL_SPEED - (int)correcao);
    } 
    else {
        // Modo de Busca (Giro sobre o eixo)
        if (erroAnterior > 0) {
            controlarMotores(SEARCH_SPEED, -SEARCH_SPEED);
        } else if (erroAnterior < 0) {
            controlarMotores(-SEARCH_SPEED, SEARCH_SPEED);
        } else {
            controlarMotores(0, 0);
        }
    }
}

float calcularPID(float erro) {
    static float prevErro = 0, integral = 0;
    static unsigned long lastTime = 0;
    
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0;
    
    if (dt <= 0 || (now - lastTime) < PID_REFRESH_RATE) return 0;

    integral = constrain(integral + erro * dt, -MAX_INTEGRAL, MAX_INTEGRAL);
    float derivada = (erro - prevErro) / dt;
    
    prevErro = erro;
    lastTime = now;

    return (erro * KP) + (KI * integral) + (KD * derivada);
}

void controlarMotores(int pwmE, int pwmD) {
    pwmE = constrain(pwmE, -MAX_SPEED, MAX_SPEED);
    pwmD = constrain(pwmD, -MAX_SPEED, MAX_SPEED);

    // Motor Esquerdo
    if (pwmE >= 0) {
        analogWrite(MOT_IN1, pwmE);
        analogWrite(MOT_IN2, 0);
    } else {
        analogWrite(MOT_IN1, 0);
        analogWrite(MOT_IN2, abs(pwmE));
    }

    // Motor Direito
    if (pwmD >= 0) {
        analogWrite(MOT_IN3, pwmD);
        analogWrite(MOT_IN4, 0);
    } else {
        analogWrite(MOT_IN3, 0);
        analogWrite(MOT_IN4, abs(pwmD));
    }
}

void processarWiFi() {
    if (!client.connected()) {
        client = server.available();
        return;
    }

    if (client.available()) {
        String data = client.readStringUntil('\n');
        data.trim();
        Serial.println("Comando WiFi: " + data);
        
        if (data == "ligar") {
            // Exemplo: pisca o LED ou altera uma flag de movimento
            digitalWrite(2, HIGH); 
        }
    }
}

void verificarNFC() {
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    // Leitura não-bloqueante (timeout de 50ms)
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50)) {
        Serial.print("Tag NFC: ");
        nfc.PrintHex(uid, uidLength);
    }
}