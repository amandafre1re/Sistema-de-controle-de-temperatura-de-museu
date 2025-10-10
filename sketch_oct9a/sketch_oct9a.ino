#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

#define DHTPIN 16       // Pino conectado ao DHT11
#define DHTTYPE DHT11   // Tipo do sensor DHT
#define LED_PIN 2       // LED do ESP32 (apenas para indicar acesso à página)

const char* ssid = "Visitantes";
const char* password = "";

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

void handleRoot() {
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  char html[512];
  if (isnan(temperatura) || isnan(umidade)) {
    snprintf(html, 512,
      "<html><head><meta http-equiv='refresh' content='5'><title>Monitor de Temperatura</title></head>"
      "<body style='font-family:Arial;text-align:center;padding-top:50px;'>"
      "<h1>Monitor de Temperatura</h1>"
      "<p>Erro ao ler o sensor</p>"
      "<p style='font-size:14px;color:gray'>Atualiza automaticamente a cada 5 segundos</p>"
      "</body></html>");
  } else {
    snprintf(html, 512,
      "<html><head><meta http-equiv='refresh' content='5'><title>Monitor de Temperatura</title></head>"
      "<body style='font-family:Arial;text-align:center;padding-top:50px;'>"
      "<h1>Monitor de Temperatura</h1>"
      "<p style='font-size:24px;'> Temperatura: %.1f &deg;C</p>"
      "<p style='font-size:24px;'> Umidade: %.1f %%</p>"
      "<p style='font-size:14px;color:gray'>Atualiza automaticamente a cada 5 segundos</p>"
      "</body></html>", temperatura, umidade);
  }

  server.send(200, "text/html", html);
  digitalWrite(LED_PIN, LOW);  // Pisca LED quando a página é acessada
}

void handleNotFound() {
  server.send(404, "text/plain", "Pagina não encontrada");
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");

  unsigned long startAttemptTime = millis();
  const unsigned long wifiTimeout = 60000; // 60 segundos

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("Conectado ao Wi-Fi!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Falha ao conectar ao Wi-Fi. Reiniciando...");
    delay(5000);
    ESP.restart();
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();
}