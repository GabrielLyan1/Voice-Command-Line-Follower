# 🎤➡️📶 Voice Command Line Follower — MATLAB + Bluetooth + ESP32 + RFID

Este projeto faz parte da proposta de desenvolvimento de protótipos didáticos voltados a **robótica, instrumentação e processamento de sinais**, com foco em integração entre **áudio, MATLAB, microcontroladores e sistemas embarcados**.

O sistema permite que um **comando de voz capturado no notebook** seja processado no **MATLAB**, convertido em valor numérico e transmitido para um **microcontrolador cliente**, que envia via **Bluetooth (BLE)** para um **ESP32 servidor** responsável por um **robô seguidor de linha**.
O robô executa a navegação e **para em pontos específicos da pista** com base na leitura de **tags RFID (MFRC522)**.

O repositório é organizado por **etapas de versionamento**, permitindo evolução modular e testes independentes.

---

# 📌 Objetivo do Protótipo

* Demonstrar integração entre:

  * Processamento de áudio (MATLAB)
  * Comunicação serial
  * Bluetooth dos ESP32 (BLE)
  * ESP32
  * Seguidor de linha
  * RFID
* Criar documentação **didática, modular e versionável**
* Permitir testes por blocos independentes
* Servir como base para projetos PETEE / ensino de sistemas embarcados

---

# 🧭 Arquitetura do Sistema

```
Microfone (HW-484)
      ↓
Notebook / MATLAB
(processamento de áudio)
      ↓ serial
Microcontrolador CLIENTE
      ↓ Bluetooth (BLE)
ESP32 SERVIDOR
      ↓
Seguidor de linha + RFID
```

---

# 🧠 Conceito Cliente–Servidor

## 💻 CLIENTE

Responsável por:

* Receber comando interpretado pelo MATLAB
* Encapsular mensagem
* Enviar via Bluetooth

**Hardware:**

* Microcontrolador
* Interface serial USB

---

## 🤖 SERVIDOR (ESP32)

Responsável por:

* Receber comando
* Controlar seguidor de linha
* Ler RFID
* Decidir parada na pista

---

# 🎙️ Parte de Áudio (MATLAB)

Funções do MATLAB:

* Aquisição de áudio do microfone
* Pré-processamento
* Extração de características
* Classificação do comando
* Conversão para código numérico
* Envio via serial

Exemplo de saída:

```
CMD,3
```

---

# 📡 Protocolo de Comunicação

Mensagens textuais via serial/Bluetooth:

```
READY
CMD,3
ACK,3
TAG,3
STOP
```

## Significados

| Mensagem | Origem  | Significado         |
| -------- | ------- | ------------------- |
| READY    | ESP32   | sistema pronto      |
| CMD,x    | Cliente | comando reconhecido |
| ACK,x    | ESP32   | confirmação         |
| TAG,x    | ESP32   | RFID detectada      |
| STOP     | ESP32   | robô parado         |

---

# ⚙️ Módulos de Hardware

## 🎤 Entrada de Áudio

* Microfone Nativo do PC
* Entrada notebook

## 📶 Comunicação

* Bluetooth (BLE)
* UART serial

## 🤖 Controle

* ESP32
* Sensores de linha
* Drivers de motor

## 🏷️ Identificação

* MFRC522 RFID 13.56 MHz

---

# 🔄 Versionamento por Etapas

## ✅ V1 — Aquisição de Áudio

* Captura via MATLAB
* Plot de waveform
* Teste de taxa de amostragem

## ✅ V2 — Processamento de Áudio

* Filtros
* Normalização
* Segmentação

## ✅ V3 — Classificação

* Extração de features
* Classificador numérico
* Saída CMD,x

## ✅ V4 — Serial MATLAB → Cliente

* Envio serial
* Teste loopback

## ✅ V5 — Cliente → Bluetooth

* Ponte serial → (BLE)

## ✅ V6 — ESP32 Bluetooth 

* Parser de mensagens

## ✅ V7 — Seguidor de Linha

* Controle isolado
* PID opcional

## ✅ V8 — RFID

* Leitura UID
* Mapeamento de tags

## ✅ V9 — Integração Total

* Comando → pista → parada

---


# 🛠️ Processo de Desenvolvimento

1. Testar áudio no MATLAB
2. Classificar comandos
3. Serial MATLAB → cliente
4. Cliente → Bluetooth
5. ESP32 recebe
6. Seguidor de linha isolado
7. RFID isolado
8. Integração
9. Protocolo completo
10. Documentação
