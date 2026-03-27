#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "config.h"
#include "Robo_logica.h"

// Objetos Globais
WiFiServer server(TCP_PORT);
WiFiClient client;
Adafruit_PN532 nfc(PN532_SDA, PN532_SCL);

// Variáveis de Estado
float erroAnterior = 0; 
unsigned long lastNFCScan = 0;
uint8_t tagParada1[] = {0x6E, 0x04, 0xF9, 0x03};
uint8_t tagParada2[] = {0x0B, 0x01, 0xC9, 0x01};

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
    pinMode(2, OUTPUT);
    
    analogReadResolution(10); // 0 a 1023
}

void executarSeguidor() {
    int s[NUM_SENSORES];
    bool linhaDetectada = false;

    s[0] = !digitalRead(PINS_SENSORES[1]);
    //Serial.println(s[0]); Serial.print(" ");
    // Sensores 1 a 6 (Pinos 36 a 33) - Leitura Analógica ADC1
    for (int i = 2; i < NUM_SENSORES-1; i++) {
        int leitura = analogRead(PINS_SENSORES[i]);
        s[i] = (leitura <= ANALOG_THRESHOLD) ? 1 : 0;
        //Serial.print(leitura); Serial.print(" ");
        if (s[i] == 1) linhaDetectada = true;
    }
    //Serial.println();

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
    
    // Tenta ler a tag com timeout de 50ms para não prejudicar o PID
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50)) {
        
        // Verifica se a tag lida tem 4 bytes (como as suas)
        if (uidLength == 4) {
            
            // Compara com a Tag 1 OU Tag 2
            if (memcmp(uid, tagParada1, 4) == 0 || memcmp(uid, tagParada2, 4) == 0) {
                Serial.println("!!! Tag de Parada Detectada !!!");
                
                // Para os motores imediatamente
                controlarMotores(0, 0);
                
                // Aguarda 3 segundos (3000ms)
                delay(3000);
                
                Serial.println("Retomando percurso...");
            }
        }
    }
}
