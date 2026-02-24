#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>

// Configurações WiFi
const char* ssid = "Barbosa 2.4";        // Substitua pelo nome da sua rede WiFi
const char* password = "VoGeralda135*";   // Substitua pela senha da sua rede WiFi

// Configurações do servidor TCP
const int TCP_PORT = 8888;                  // Porta TCP (pode alterar se necessário)
WiFiServer server(TCP_PORT);
WiFiClient client;

#define LED_AMARELO 25
#define LED_VERMELHO 33
#define LED_AZUL 32
#define MAX_CLIENTS 1

void setup() {
    Serial.begin(115200);
    pinMode(LED_AMARELO, OUTPUT);
    pinMode(LED_VERMELHO, OUTPUT);
    pinMode(LED_AZUL, OUTPUT);
    digitalWrite(LED_AMARELO, LOW);  // Começa com LED desligado
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_AZUL, LOW);
    
    // Conecta ao WiFi
    Serial.print("Conectando ao WiFi");
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi conectado!");
    Serial.print("Endereço IP do ESP32: ");
    Serial.println(WiFi.localIP());
    
    // Inicia o servidor TCP
    server.begin();
    Serial.printf("Servidor TCP iniciado na porta %d\n", TCP_PORT);
    Serial.println("Aguardando conexões...");
}

void loop() {
    // Verifica se há um cliente conectado
    if (!client.connected()) {
        // Aguarda nova conexão
        client = server.available();
        if (client) {
            Serial.println("Novo cliente conectado!");
            Serial.printf("IP do cliente: %s\n", client.remoteIP().toString().c_str());
        }
        delay(100);
        return;
    }
    
    // Verifica se há dados disponíveis para ler
    while (client.available()) {
        String data = client.readStringUntil('\n');  // Lê até encontrar nova linha
        data.trim();  // Remove espaços em branco e quebras de linha
        
        if (data.length() > 0) {
            Serial.print("Recebido: ");
            Serial.println(data);
            
            // Processa o comando recebido
            if (data == "amarelo") {
                digitalWrite(LED_AMARELO, HIGH);
                Serial.println("LED AMARELO LIGADO");
                client.println("LED AMARELO LIGADO");  // Confirmação para o cliente
            } 
            else if (data == "vermelho") {
                digitalWrite(LED_VERMELHO, HIGH);
                Serial.println("LED VERMELHO LIGADO");
                client.println("LED VERMELHO LIGADO");  // Confirmação para o cliente
            }
            else if (data == "azul") {
                digitalWrite(LED_AZUL, HIGH);
                Serial.println("LED AZUL LIGADO");
                client.println("LED AZUL LIGADO");  // Confirmação para o cliente
            }else if(data == "desligar") {
                digitalWrite(LED_AMARELO, LOW);
                digitalWrite(LED_VERMELHO, LOW);
                digitalWrite(LED_AZUL, LOW);
                Serial.println("TODOS OS LEDS DESLIGADOS");
                client.println("TODOS OS LEDS DESLIGADOS");  // Confirmação para o cliente
            }
            else {
                client.println("Comando não reconhecido");
            }
        }
    }
}