#include <Arduino.h>

/* ================================================================
 * CONFIGURAÇÕES DE HARDWARE - ESP32 DEV KIT V1
 * ================================================================ */
// Sequência linear de pinos ADC no lado esquerdo (Top -> Down)
#define SENSOR1 13  // VP
#define SENSOR2 12  // VN
#define SENSOR3 14  
#define SENSOR4 27  
#define SENSOR5 26  
#define SENSOR6 25  
#define SENSOR7 33  
#define SENSOR8 32  
#define SENSOR9 35  // ADC2_7

// Motores (Pinos com suporte a PWM/LEDC)
#define IN1 16  // RX2
#define IN2 17  // TX2
#define IN3 18  // D18
#define IN4 19  // D19

#define NUM_SENSORES 9

/* ================================================================
 * PARÂMETROS DE CONTROLE
 * ================================================================ */
// Threshold para 10 bits (0-1023). Ajuste conforme a cor da sua pista.
#define ANALOG_THRESHOLD 300 

// Constantes PID (Mantidas do seu projeto)
const float kp = 69;
const float kd = 345;
const float ki = 0.03;

#define MaxIntegral 650
#define PIDRefreshRate 15 // ESP32 é mais rápido, podemos baixar o refresh rate

// Velocidades
const int nominalSpeed = 180; 
const int MaxSpeed = 255;

float erroAnterior = 0;
const int sensores[NUM_SENSORES] = {
  SENSOR1, SENSOR2, SENSOR3, SENSOR4, SENSOR5, SENSOR6, SENSOR7, SENSOR8, SENSOR9
};

/* ================================================================
 * FUNÇÕES DE LEITURA E ATUAÇÃO
 * ================================================================ */

void lerSensores(int *outBits) {
  for (int i = 0; i < NUM_SENSORES; i++) {
    // Todos os pinos escolhidos no ESP32 aqui suportam leitura analógica
    int leitura = analogRead(sensores[i]);
    outBits[i] = (leitura <= ANALOG_THRESHOLD) ? 1 : 0;
  }
}

bool temLinha(const int *s) {
  for (int i = 0; i < NUM_SENSORES; i++) {
    if (s[i]) return true;
  }
  return false;
}

void controlarMotores(int pwmE, int pwmD) {
  pwmE = constrain(pwmE, -MaxSpeed, MaxSpeed);
  pwmD = constrain(pwmD, -MaxSpeed, MaxSpeed);

  // Lógica para Motor Esquerdo
  if (pwmE >= 0) {
    analogWrite(IN1, pwmE);
    analogWrite(IN2, 0);
  } else {
    analogWrite(IN1, 0);
    analogWrite(IN2, abs(pwmE));
  }

  // Lógica para Motor Direito
  if (pwmD >= 0) {
    analogWrite(IN3, pwmD);
    analogWrite(IN4, 0);
  } else {
    analogWrite(IN3, 0);
    analogWrite(IN4, abs(pwmD));
  }
}

float calcularErro(const int *s) {
  // Pesos balanceados para 9 sensores
  static const int pesos[NUM_SENSORES] = {-8, -4, -2, -1, 0, 1, 2, 4, 8};
  float num = 0;
  int den = 0;

  for (int i = 0; i < NUM_SENSORES; i++) {
    num += s[i] * pesos[i];
    den += s[i];
  }

  if (den != 0) {
    float e = num / den;
    erroAnterior = e;
    return e;
  }
  return erroAnterior;
}

float calcularPID(float erro) {
  static unsigned long prevTime = 0;
  static float prevErro = 0, integral = 0;
  
  unsigned long now = millis();
  unsigned long dt = now - prevTime;
  if (dt < PIDRefreshRate) return 0;

  float derivative = (erro - prevErro) / (float)dt;
  integral += erro * (float)dt;
  integral = constrain(integral, -MaxIntegral, MaxIntegral);

  prevErro = erro;
  prevTime = now;

  return (erro * kp) + (ki * integral) + (kd * derivative);
}

/* ================================================================
 * SETUP E LOOP
 * ================================================================ */

void setup() {
  Serial.begin(115200); // ESP32 usa 115200 por padrão

  // Configuração da resolução ADC para 10 bits (0-1023)
  analogReadResolution(10);

  for (int i = 0; i < NUM_SENSORES; i++) {
    pinMode(sensores[i], INPUT);
  }

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  Serial.println("ESP32 Line Follower Ready!");
}

void loop() {
  int s[NUM_SENSORES];
  lerSensores(s);

  if (temLinha(s)) {
    float erro = calcularErro(s);
    float correcao = calcularPID(erro);

    controlarMotores(nominalSpeed + (int)correcao, nominalSpeed - (int)correcao);
  } 
  else {
    // Lógica de busca baseada no último erro conhecido
    if (erroAnterior > 0) controlarMotores(nominalSpeed, -nominalSpeed);
    else if (erroAnterior < 0) controlarMotores(-nominalSpeed, nominalSpeed);
    else controlarMotores(0, 0);
  }
}
