#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- WiFi ---
extern const char* WIFI_SSID;
extern const char* WIFI_PASS;
#define TCP_PORT 8888

// --- PID e Motores ---
#define ANALOG_THRESHOLD 300
const float KP = 69.0, KD = 345.0, KI = 0.03;
#define NOMINAL_SPEED 175
#define MAX_SPEED 255
#define SEARCH_SPEED 130
#define PID_REFRESH_RATE 20
#define MAX_INTEGRAL 650

// --- Pinagem ---
#define NUM_SENSORES 9
const int PINS_SENSORES[NUM_SENSORES] = {13, 12, 14, 27, 26, 25, 33, 32, 35};
#define MOT_IN1 16 //conferir esses pinos no ESP
#define MOT_IN2 17
#define MOT_IN3 18
#define MOT_IN4 19
#define PN532_SDA 21
#define PN532_SCL 22

#endif