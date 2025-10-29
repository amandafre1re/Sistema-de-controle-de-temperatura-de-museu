#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "DHT.h"

#define DHTPIN 14
#define DHTTYPE DHT11
#define BUZZER_PIN 33
#define LED_PIN 25
#define LED_PIN_AZUL 27

const char* ssid = "Studio 604";
const char* password = "quesenha604";

typedef struct {
  float temperatura;
  float umidade;
  unsigned long tempoLerSensor;
  unsigned long tempoLerTemperatura;
  unsigned long tempoLerUmidade;
  unsigned long tempoLigarAlarme;
  unsigned long tempoDesligarAlarme;
  char situacao[64];
  int status_buzina;
  int estado_led;
  int estado_led_azul;
} SensorData;

QueueHandle_t sensorQueue;
WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

unsigned long _tmp_tempoLigarAlarme = 0;
unsigned long _tmp_tempoDesligarAlarme = 0;

void TaskLeituraSensor(void *pvParameters);
void TaskWebServer(void *pvParameters);
void handleDados();
void handleToggleBuzzer();
void handleToggleLed();
void handleToggleLedAzul();

volatile int status_buzina = 0;
volatile int estado_led = 0;
volatile int estado_led_azul = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN_AZUL, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_PIN_AZUL, LOW);
  noTone(BUZZER_PIN);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());

  if (!LittleFS.begin(true)) {
    Serial.println("Falha ao montar LittleFS!");
  } else {
    Serial.println("LittleFS montado com sucesso.");
  }

  sensorQueue = xQueueCreate(1, sizeof(SensorData));

  server.serveStatic("/", LittleFS, "/index.html");
  server.serveStatic("/style.css", LittleFS, "/style.css");
  server.serveStatic("/script.js", LittleFS, "/script.js");
  server.on("/dados", HTTP_GET, handleDados);
  server.on("/toggleBuzzer", HTTP_GET, handleToggleBuzzer);
  server.on("/toggleLed", HTTP_GET, handleToggleLed);
  server.on("/toggleLedAzul", HTTP_GET, handleToggleLedAzul);

  server.onNotFound([](){
    if (LittleFS.exists("/index.html")) {
      server.sendHeader("Location", "/");
      server.send(302, "text/plain", "");
    } else {
      server.send(404, "text/plain", "Not found");
    }
  });

  server.begin();
  Serial.println("Servidor web iniciado.");

  xTaskCreatePinnedToCore(TaskLeituraSensor, "LeituraSensor", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskWebServer, "WebServer", 8192, NULL, 1, NULL, 0);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}

void TaskLeituraSensor(void *pvParameters) {
  (void) pvParameters;
  SensorData latest;
  for (;;) {
    unsigned long t0 = micros();

    unsigned long tTempStart = micros();
    float temperatura = dht.readTemperature();
    unsigned long tTemp = micros() - tTempStart;

    unsigned long tUmidStart = micros();
    float umidade = dht.readHumidity();
    unsigned long tUmid = micros() - tUmidStart;

    unsigned long tSensor = micros() - t0;

    char situacao[64] = "";
    if (isnan(temperatura)) {
      strncpy(situacao, "Erro ao ler a temperatura.", sizeof(situacao));
    } else if (temperatura >= 18 && temperatura < 20) {
      strncpy(situacao, "Temperatura boa", sizeof(situacao));
    } else if ((temperatura < 18) || (temperatura >= 20 && temperatura <= 24)) {
      strncpy(situacao, "Temperatura fora do ideal", sizeof(situacao));
    } else if (temperatura > 24 && temperatura < 57) {
      strncpy(situacao, "Temperatura crítica!!", sizeof(situacao));
    } else if (temperatura >= 57) {
      strncpy(situacao, "ALERTA INCENDIO!!", sizeof(situacao));
      unsigned long tInicioLigar = micros();
      if (!status_buzina) {
        tone(BUZZER_PIN, 2000);
        digitalWrite(LED_PIN, HIGH);
        status_buzina = 1;
        estado_led = 1;
      }
      _tmp_tempoLigarAlarme = micros() - tInicioLigar;
    }

    if (!isnan(umidade)) {
      if (umidade >= 45 && umidade < 55) {
        digitalWrite(LED_PIN_AZUL, LOW);
        estado_led_azul = 0;
      } else {
        digitalWrite(LED_PIN_AZUL, HIGH);
        estado_led_azul = 1;
      }
    }

    latest.temperatura = isnan(temperatura) ? 0.0f : temperatura;
    latest.umidade = isnan(umidade) ? 0.0f : umidade;
    latest.tempoLerSensor = tSensor;
    latest.tempoLerTemperatura = tTemp;
    latest.tempoLerUmidade = tUmid;
    latest.tempoLigarAlarme = _tmp_tempoLigarAlarme;
    latest.tempoDesligarAlarme = _tmp_tempoDesligarAlarme;
    strncpy(latest.situacao, situacao, sizeof(latest.situacao));
    latest.status_buzina = status_buzina;
    latest.estado_led = estado_led;
    latest.estado_led_azul = estado_led_azul;

    xQueueOverwrite(sensorQueue, &latest);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void TaskWebServer(void *pvParameters) {
  (void) pvParameters;
  while (1) {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void handleDados() {
  SensorData data;
  if (xQueuePeek(sensorQueue, &data, 0) == pdTRUE) {
    String json = "{";
    json += "\"temperatura\":" + String(data.temperatura, 1) + ",";
    json += "\"umidade\":" + String(data.umidade, 1) + ",";
    json += "\"tempoLerSensor\":" + String(data.tempoLerSensor) + ",";
    json += "\"tempoLerTemperatura\":" + String(data.tempoLerTemperatura) + ",";
    json += "\"tempoLerUmidade\":" + String(data.tempoLerUmidade) + ",";
    json += "\"tempoLigarAlarme\":" + String(data.tempoLigarAlarme) + ",";
    json += "\"tempoDesligarAlarme\":" + String(data.tempoDesligarAlarme) + ",";
    json += "\"situacao\":\"" + String(data.situacao) + "\",";
    json += "\"alarme\":" + String(data.status_buzina) + ",";
    json += "\"led\":" + String(data.estado_led) + ",";
    json += "\"led_azul\":" + String(data.estado_led_azul);
    json += "}";
    server.send(200, "application/json", json);
  } else {
    server.send(200, "application/json", "{\"erro\":\"sem dados\"}");
  }
}

void handleToggleBuzzer() {
  unsigned long tInicio = micros();
  if (status_buzina) {
    noTone(BUZZER_PIN);
    status_buzina = 0;
    _tmp_tempoDesligarAlarme = micros() - tInicio;
    digitalWrite(LED_PIN, LOW);
    estado_led = 0;
  } else {
    tone(BUZZER_PIN, 2000);
    status_buzina = 1;
    _tmp_tempoLigarAlarme = micros() - tInicio;
    digitalWrite(LED_PIN, HIGH);
    estado_led = 1;
  }
  server.send(200, "text/plain", "ok");
}

void handleToggleLed() {
  estado_led = !estado_led;
  digitalWrite(LED_PIN, estado_led ? HIGH : LOW);
  server.send(200, "text/plain", "ok");
}

void handleToggleLedAzul() {
  estado_led_azul = !estado_led_azul;
  digitalWrite(LED_PIN_AZUL, estado_led_azul ? HIGH : LOW);
  server.send(200, "text/plain", "ok");
}
