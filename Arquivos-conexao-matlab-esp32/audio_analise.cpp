#include <Arduino.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
    while (!Serial) {
    ; 
    }
    Serial.println("READY");
}

int leituraMatlab(){

  if(Serial.available() > 0) {
    
    int valor_recebido = Serial.parseInt();
    // limpa buffer
    while (Serial.available()) Serial.read();
    Serial.print("OK,");
    Serial.println(valor_recebido);
    return valor_recebido;
  }
  return -1;

}

void loop() { 

  int valor = leituraMatlab();  

  for(int i = 0; i < valor; i++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}