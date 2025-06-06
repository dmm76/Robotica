#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Pinos dos motores
#define IN1 32
#define IN2 33
#define IN3 22
#define IN4 23

// Pinos do sensor ultrassônico
#define TRIG 26
#define ECHO 27

char ultimoComando = 'S';
bool autorizado = false;

void setup() {
  Serial.begin(9600);
  SerialBT.setPin("2102", 4); // PIN Bluetooth
  SerialBT.begin("CarrinhoDouglas");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
}

void loop() {
  if (SerialBT.available()) {
    char comando = SerialBT.read();
    SerialBT.flush(); // Limpa o buffer após ler

    // Verifica se é comando permitido
    if (!strchr("FBLRSAU", comando)) return;

    // Autenticação obrigatória
    if (!autorizado) {
      if (comando == 'U') {
        autorizado = true;
        Serial.println("✅ Acesso autorizado!");
        SerialBT.println("✅ Acesso autorizado!");
      } else {
        Serial.println("❌ Acesso negado. Envie 'U' para liberar.");
        SerialBT.println("❌ Acesso negado. Envie 'U' para liberar.");
        return;
      }
    }

    // Segurança ao inverter frente ↔ ré
    bool mudouSentido = 
      (ultimoComando == 'F' && comando == 'B') || 
      (ultimoComando == 'B' && comando == 'F');

    if (mudouSentido) {
      parar();
      delay(150);
    }

    // Executa o comando
    switch (comando) {
      case 'F': andarFrente(); ultimoComando = 'F'; break;
      case 'B': andarTras(); ultimoComando = 'B'; break;
      case 'L': girarEsquerda(); ultimoComando = 'L'; break;
      case 'R': girarDireita(); ultimoComando = 'R'; break;
      case 'S': parar(); ultimoComando = 'S'; break;
      case 'A': iniciarModoAutonomo(); ultimoComando = 'A'; break;
    }
  }
}

// ------------------ Movimento ------------------ //
void setMotor(int in1, int in2, int in3, int in4) {
  digitalWrite(IN1, in1);
  digitalWrite(IN2, in2);
  digitalWrite(IN3, in3);
  digitalWrite(IN4, in4);
}

void andarFrente()     { setMotor(HIGH, LOW, HIGH, LOW); }
void andarTras()       { setMotor(LOW, HIGH, LOW, HIGH); }
void girarDireita()    { setMotor(HIGH, LOW, LOW, HIGH); }
void girarEsquerda()   { setMotor(LOW, HIGH, HIGH, LOW); }
void parar()           { setMotor(LOW, LOW, LOW, LOW); }

// ------------------ Sensor Ultrassônico ------------------ //
long lerDistanciaCM() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  long duracao = pulseIn(ECHO, HIGH, 25000);
  long distancia = duracao * 0.034 / 2;
  return distancia;
}

// ------------------ Modo Autônomo ------------------ //
void iniciarModoAutonomo() {
  Serial.println("🚗 Modo Autônomo ativado");
  SerialBT.println("🚗 Modo Autônomo ativado");

  while (true) {
    long distancia = lerDistanciaCM();
    Serial.println("Distância: " + String(distancia) + " cm");

    if (distancia > 25 && distancia < 300) {
      andarFrente();
    } else {
      parar();
      delay(200);
      girarDireita();
      delay(400);
      parar();
      delay(200);
    }

    if (SerialBT.available()) {
      char novoComando = SerialBT.read();
      if (novoComando == 'S') {
        parar();
        Serial.println("🛑 Saindo do modo autônomo.");
        SerialBT.println("🛑 Saindo do modo autônomo.");
        break;
      }
    }

    delay(100);
  }
}
