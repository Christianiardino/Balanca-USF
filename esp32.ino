#include "BluetoothSerial.h"
#include "HX711.h"
#define DT 5
#define SCK 14

BluetoothSerial SerialBT;
HX711 scale;

int a = 33, b = 13, c = 19, d = 22, e = 23, f = 25, g = 18;// Pins para o segmento do display
int d1 = 32, d2 = 26, d3 = 27, d4 = 4, dp = 21;// Pins para ativação
int led = 2;//Led azul interno da placa ESP32
int generalDelay = 5;// Delay padrão para o display
int recebido;// Código ASCII recebido por bluetooth
int calib = 86;// Peso do cubo magico em gramas
int firstDigit = 8, secondDigit = 8, thirdDigit = 8, fourthDigit = 8;//Digitos iniciais do display
long maxChange = 2000;// Valor máximo de mudança de leitura da HX711
long lastValue;// Ultimo valor medido pela HX711
long realValue = 0;// Valor real
long factor = 1;// Fator de divisão
double rememberValue = 0;// Copia do valor
bool lastChange = false;// Medição fina de peso

void setup() {
  Serial.begin(57600);// Serial de comunicação entre a ESP32 e o computador
  SerialBT.begin("balanca ACGT nova");// Nome bluetooth da balança

  scale.begin(DT, SCK);// Iniciando a placa HX711
  lastValue = scale.get_value(3);

  pinMode(a, OUTPUT);// Declaração de pin a como um output
  pinMode(b, OUTPUT);// Declaração de pin b como um output
  pinMode(c, OUTPUT);// Declaração de pin c como um output
  pinMode(d, OUTPUT);// Declaração de pin d como um output
  pinMode(e, OUTPUT);// Declaração de pin e como um output
  pinMode(f, OUTPUT);// Declaração de pin f como um output
  pinMode(g, OUTPUT);// Declaração de pin g como um output
  pinMode(d1, OUTPUT);// Declaração de pin d1 como um output
  pinMode(d2, OUTPUT);// Declaração de pin d2 como um output
  pinMode(d3, OUTPUT);// Declaração de pin d3 como um output
  pinMode(d4, OUTPUT);// Declaração de pin d4 como um output
  pinMode(dp, OUTPUT);// Declaração de pin dp como um output
  pinMode(led, OUTPUT);// Declaração de pin led como um output
  digitalWrite(led, HIGH);// Ativando saida de tensão pelo pin led
}

void loop() {
  if (scale.is_ready()) { // Verificando se a placa HX711 está transmitindo dados
    long readingValue = scale.get_units(1);// Salvando valor transmitido pela HX711
    if (abs(readingValue - lastValue) > maxChange) {// Verificando se o valor de saida está estável
      delay(2500);// Aguardar 2.5 segundos para estabilizar o valor de saida da HX711
      realValue = readingValue - lastValue;
      if (lastChange == true) {
        // Se ainda houver variação somar o valor da variação no valor atual do peso
        rememberValue += realValue;
      } else {
        rememberValue = realValue;
      }
      if (rememberValue / factor < 9999 && rememberValue / factor > 0) {
        // Digitos do display
        fourthDigit = int(rememberValue / factor) % 10;
        thirdDigit = ((int(rememberValue / factor) % 100) - fourthDigit) / 10;
        secondDigit = ((int(rememberValue / factor) % 100) - (fourthDigit + thirdDigit)) / 100;
        firstDigit = (int(rememberValue / factor) - (fourthDigit + thirdDigit + secondDigit)) / 1000;
      }
      lastChange = true;
    } else {
      lastChange = false;
      realValue = 0;
    }
    lastValue = readingValue;
    if (rememberValue / factor > 0) { //Enviando o peso pelo serial bluetooth
      SerialBT.println(rememberValue / factor);
    } else {
      SerialBT.println(0.000);
    }
  }

  if (SerialBT.available()) { // Verificando se a placa está recebendo dados via bluetooth
    recebido = SerialBT.read();
    blueToothSinal(recebido);
  }

  for (int i = 0; i < 10; i++) {// Logica para acender o display
    clearLed();
    selectorQuad(1);
    ledLigthUp(firstDigit);
    delay(generalDelay);

    clearLed();
    selectorQuad(2);
    ledLigthUp(secondDigit);
    delay(generalDelay);

    clearLed();
    selectorQuad(3);
    ledLigthUp(thirdDigit);
    delay(generalDelay);

    clearLed();
    selectorQuad(4);
    ledLigthUp(fourthDigit);
    delay(generalDelay);

    clearLed();
  }

}

void blueToothSinal(int sinal) {// Verifica o código ASCII recebido pela ESP32
  if (sinal == 99) { // Código de calibragem 
    for (int i = 0; i < 20; i++) {
      clearLed();
      selectorQuad(1);
      calibragemPrinter(1);
      delay(generalDelay);

      clearLed();
      selectorQuad(2);
      calibragemPrinter(2);
      delay(generalDelay);

      clearLed();
      selectorQuad(3);
      calibragemPrinter(3);
      delay(generalDelay);

      clearLed();
      selectorQuad(4);
      calibragemPrinter(4);
      delay(generalDelay);

      clearLed();
    }
    factor = rememberValue / calib;
    fourthDigit = 6;
    thirdDigit = 8;
    secondDigit = 0;
    firstDigit = 0;
  }
}

void selectorQuad(int q) {// Seletor de quadrante do display
  digitalWrite(d1, HIGH);
  digitalWrite(d2, HIGH);
  digitalWrite(d3, HIGH);
  digitalWrite(d4, HIGH);
  digitalWrite(dp, LOW);
  switch (q) {
    case 1:
      digitalWrite(d1, LOW);
      digitalWrite(dp, HIGH);
      break;
    case 2:
      digitalWrite(d2, LOW);
      break;
    case 3:
      digitalWrite(d3, LOW);
      break;
    case 4:
      digitalWrite(d4, LOW);
      break;
  }
}

void clearLed() { // Apagar todos os segmentos do display
  digitalWrite(a, LOW);
  digitalWrite(b, LOW);
  digitalWrite(c, LOW);
  digitalWrite(d, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
  digitalWrite(dp, LOW);
}

void ledLigthUp(int code) {
  switch (code) {
    case 0:// Acender 0 no display
      digitalWrite(a, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, HIGH);
      digitalWrite(e, HIGH);
      digitalWrite(f, HIGH);
      digitalWrite(g, LOW);
      break;
    case 1:// Acender 1 no display
      digitalWrite(a, LOW);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);
      digitalWrite(e, LOW);
      digitalWrite(f, LOW);
      digitalWrite(g, LOW);
      break;
    case 2:// Acender 2 no display
      digitalWrite(a, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(c, LOW);
      digitalWrite(d, HIGH);
      digitalWrite(e, HIGH);
      digitalWrite(f, LOW);
      digitalWrite(g, HIGH);
      break;
    case 3:// Acender 3 no display
      digitalWrite(a, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, HIGH);
      digitalWrite(e, LOW);
      digitalWrite(f, LOW);
      digitalWrite(g, HIGH);
      break;
    case 4:// Acender 4 no display
      digitalWrite(a, LOW);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);
      digitalWrite(e, LOW);
      digitalWrite(f, HIGH);
      digitalWrite(g, HIGH);
      break;
    case 5:// Acender 5 no display
      digitalWrite(a, HIGH);
      digitalWrite(b, LOW);
      digitalWrite(c, HIGH);
      digitalWrite(d, HIGH);
      digitalWrite(e, LOW);
      digitalWrite(f, HIGH);
      digitalWrite(g, HIGH);
      break;
    case 6:// Acender 6 no display
      digitalWrite(a, HIGH);
      digitalWrite(b, LOW);
      digitalWrite(c, HIGH);
      digitalWrite(d, HIGH);
      digitalWrite(e, HIGH);
      digitalWrite(f, HIGH);
      digitalWrite(g, HIGH);
      break;
    case 7:// Acender 7 no display
      digitalWrite(a, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);
      digitalWrite(e, LOW);
      digitalWrite(f, LOW);
      digitalWrite(g, LOW);
      break;
    case 8:// Acender 8 no display
      digitalWrite(a, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, HIGH);
      digitalWrite(e, HIGH);
      digitalWrite(f, HIGH);
      digitalWrite(g, HIGH);
      break;
    case 9:// Acender 9 no display
      digitalWrite(a, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, HIGH);
      digitalWrite(e, LOW);
      digitalWrite(f, HIGH);
      digitalWrite(g, HIGH);
      break;
  }
}

void calibragemPrinter(int digitPrinter) {
  switch (digitPrinter) {
    case 1:// Acender c no display
      digitalWrite(a, HIGH);
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, HIGH);
      digitalWrite(e, HIGH);
      digitalWrite(f, HIGH);
      digitalWrite(g, LOW);
      break;
    case 2:// Acender a no display
      digitalWrite(a, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);
      digitalWrite(e, HIGH);
      digitalWrite(f, HIGH);
      digitalWrite(g, HIGH);
      break;
    case 3:// Acender l no display
      digitalWrite(a, LOW);
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, HIGH);
      digitalWrite(e, HIGH);
      digitalWrite(f, HIGH);
      digitalWrite(g, LOW);
      break;
    case 4:// Acender i no display
      digitalWrite(a, LOW);
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);
      digitalWrite(e, HIGH);
      digitalWrite(f, HIGH);
      digitalWrite(g, LOW);
      break;
  }
}
