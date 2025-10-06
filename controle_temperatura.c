#include "DHT.h"

#define DHTPIN 13
#define DHTTYPE DHT11
#define BUZZER_PIN 21
#define LED_PIN 22

DHT dht(DHTPIN, DHTTYPE);

int status_buzina = 0;

void setup() {
  Serial.begin(115200);  
  dht.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  delay(2000);  

  float temperatura = dht.readTemperature();

  if (isnan(temperatura)) {
    Serial.println("Erro ao ler a temperatura.");
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  if (temperatura >= 18 && temperatura < 20) {
    Serial.println("Temperatura boa");
    desligarAlarme();
  } 
  else if ((temperatura < 18) || (temperatura > 20 && temperatura <= 24)) {
    Serial.println("Temperatura fora do ideal");
    desligarAlarme();
  } 
  else if (temperatura > 24 && temperatura < 57) {
    Serial.println("Temperatura Crítica!!");
    desligarAlarme();
  } 
  else if (temperatura >= 57) {
    Serial.println("ALERTA INCÊNDIO!!");
    ligarAlarme();
  }
}

void ligarAlarme() {
  if (status_buzina == 0) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    status_buzina = 1;
  }
}

void desligarAlarme() {
  if (status_buzina == 1) {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    status_buzina = 0;
  }
}
