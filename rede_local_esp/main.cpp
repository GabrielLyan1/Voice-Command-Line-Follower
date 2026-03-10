#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>

// Configurações do Access Point (rede criada pelo ESP32)
const char* ssid = "ESP_Recog";        // Nome da rede WiFi que o ESP32 vai criar
const char* password = "recog32follow"; // Senha da rede (mínimo 8 caracteres)

// Configurações do servidor TCP
const int TCP_PORT = 8888;              // Porta TCP
WiFiServer server(TCP_PORT);
WiFiClient client;

// Pinos dos LEDs
#define LED_AMARELO 25
#define LED_VERMELHO 33
#define LED_AZUL 32
#define LED_BUILTIN 2  // LED interno do ESP32 (opcional)

void setup() {
    Serial.begin(115200);
    
    // Configura LEDs
    pinMode(LED_AMARELO, OUTPUT);
    pinMode(LED_VERMELHO, OUTPUT);
    pinMode(LED_AZUL, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Estado inicial: todos desligados
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_AZUL, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    
    Serial.println("\n=================================");
    Serial.println("ESP32 - MODO ACCESS POINT");
    Serial.println("=================================");
    
    // Cria a rede WiFi (Access Point)
    Serial.println("Criando rede WiFi...");
    
    // Configura o ESP32 como Access Point
    // WiFi.softAP(ssid); // Versão sem senha (não recomendado)
    WiFi.softAP(ssid, password);  // Com senha
    
    // Obtém o IP do Access Point (sempre 192.168.4.1)
    IPAddress IP = WiFi.softAPIP();
    
    Serial.println("\n✅ REDE CRIADA COM SUCESSO!");
    Serial.println("=================================");
    Serial.print("📡 Nome da rede (SSID): ");
    Serial.println(ssid);
    Serial.print("🔑 Senha: ");
    Serial.println(password);
    Serial.print("🌐 Endereço IP do ESP32: ");
    Serial.println(IP);
    Serial.print("🔌 Porta TCP: ");
    Serial.println(TCP_PORT);
    Serial.println("=================================");
    Serial.println("\n📱 Conecte seu celular/PC à rede:");
    Serial.println("   WiFi: ESP_Recog");
    Serial.println("   Senha: recog32follow");
    Serial.println("\n💻 No Python, use o IP: 192.168.4.1");
    Serial.println("=================================\n");
    
    // Pisca LED interno para indicar que está pronto
    for(int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
    }
    
    // Inicia o servidor TCP
    server.begin();
    Serial.println("🎧 Servidor TCP iniciado! Aguardando conexões...");
}

void loop() {
    // Verifica se há um cliente conectado
    if (!client.connected()) {
        // Aguarda nova conexão
        client = server.available();
        if (client) {
            Serial.println("\n✅ NOVO CLIENTE CONECTADO!");
            Serial.printf("   IP do cliente: %s\n", client.remoteIP().toString().c_str());
            
            // Envia mensagem de boas-vindas
            client.println("Bem-vindo ao ESP32_Recog!");
            client.println("Comandos disponiveis: amarelo, vermelho, azul, desligar");
        }
        delay(100);
        return;
    }
    
    // Verifica se o cliente ainda está conectado
    if (!client.connected()) {
        Serial.println("❌ Cliente desconectado");
        client.stop();
        return;
    }
    
    // Verifica se há dados disponíveis para ler
    while (client.available()) {
        String data = client.readStringUntil('\n');  // Lê até encontrar nova linha
        data.trim();  // Remove espaços em branco e quebras de linha
        
        if (data.length() > 0) {
            Serial.print("📨 Recebido: '");
            Serial.print(data);
            Serial.println("'");
            
            // Processa o comando recebido
            if (data == "amarelo") {
                // Desliga todos primeiro
                digitalWrite(LED_AMARELO, HIGH);
                digitalWrite(LED_VERMELHO, LOW);
                digitalWrite(LED_AZUL, LOW);
                Serial.println("💡 LED AMARELO LIGADO");
                client.println("✅ LED AMARELO LIGADO");
            } 
            else if (data == "vermelho") {
                digitalWrite(LED_AMARELO, LOW);
                digitalWrite(LED_VERMELHO, HIGH);
                digitalWrite(LED_AZUL, LOW);
                Serial.println("💡 LED VERMELHO LIGADO");
                client.println("✅ LED VERMELHO LIGADO");
            }
            else if (data == "azul") {
                digitalWrite(LED_AMARELO, LOW);
                digitalWrite(LED_VERMELHO, LOW);
                digitalWrite(LED_AZUL, HIGH);
                Serial.println("💡 LED AZUL LIGADO");
                client.println("✅ LED AZUL LIGADO");
            }
            else if (data == "desligar" || data == "desliga" || data == "off") {
                digitalWrite(LED_AMARELO, LOW);
                digitalWrite(LED_VERMELHO, LOW);
                digitalWrite(LED_AZUL, LOW);
                Serial.println("💡 TODOS OS LEDS DESLIGADOS");
                client.println("✅ TODOS OS LEDS DESLIGADOS");
            }
            else if (data == "info" || data == "status") {
                client.println("📊 STATUS DOS LEDS:");
                client.println("   Amarelo: " + String(digitalRead(LED_AMARELO) ? "LIGADO" : "DESLIGADO"));
                client.println("   Vermelho: " + String(digitalRead(LED_VERMELHO) ? "LIGADO" : "DESLIGADO"));
                client.println("   Azul: " + String(digitalRead(LED_AZUL) ? "LIGADO" : "DESLIGADO"));
            }
            else if (data == "ajuda" || data == "help") {
                client.println("📋 COMANDOS DISPONIVEIS:");
                client.println("   'amarelo'  - Liga LED amarelo");
                client.println("   'vermelho' - Liga LED vermelho");
                client.println("   'azul'     - Liga LED azul");
                client.println("   'desligar' - Desliga todos os LEDs");
                client.println("   'info'     - Mostra status dos LEDs");
                client.println("   'ajuda'    - Mostra esta mensagem");
            }
            else {
                client.println("❌ Comando nao reconhecido. Digite 'ajuda'");
            }
        }
    }
    
    // Pequeno delay para não sobrecarregar
    delay(10);
}3
