#ifndef robo_logica__H
#define robo_logica__H

void setupHardware();
void executarSeguidor();
void processarWiFi();
void verificarNFC();
void controlarMotores(int pwmE, int pwmD);
float calcularPID(float erro);

#endif