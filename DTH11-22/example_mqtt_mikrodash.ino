/*
 Ejemplo del canal de Youtube "MikroTutoriales"
 https://www.youtube.com/@MikroTutoriales16
 
 Utilizando el broker MQTT de mqtt.mikrodash.com
 y la plataforma de https://app.mikrodash.com
 para crear una dashboard personalizada
*/

/**
 * Librerias de WiFi y utilidades para publicar en MQTT
*/

#include <ESP8266WiFi.h>  // Biblioteca para WiFi en ESP8266
#include <PubSubClient.h> // Biblioteca para cliente MQTT

// Inclusión de bibliotecas para la pantalla OLED
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// Definición de las dimensiones de la pantalla OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Creación del objeto display para la pantalla OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Inclusión de la biblioteca para el sensor DHT
#include "DHTesp.h"

DHTesp dht;

// Definición de pines
#define PIN_TEMP 2
#define PIN_SDA 4
#define PIN_SCL 5

// Tópicos MQTT (reemplaza "your_mikrodash_id" por tu ID real de MikroDash)
const char* topic_temp = "your_mikrodash_id/temperatura/value";
const char* topic_hum = "your_mikrodash_id/humedad/value";

// Configuración de la red WiFi y servidor MQTT
const char* ssid = "your_wifi_ssid";          // Nombre de la red WiFi
const char* password = "your_wifi_password";  // Contraseña de la red WiFi
const char* mqtt_server = "mqtt.mikrodash.com"; // Servidor MQTT
const int mqtt_port = 1883;                    // Puerto MQTT
const char* mqtt_user = "your_mqtt_username";  // Usuario MQTT
const char* mqtt_password = "your_mqtt_password"; // Contraseña MQTT

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: " + WiFi.localIP().toString());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESPClient", mqtt_user, mqtt_password)) {
      Serial.println("Conectado");
      // Aquí puedes suscribirte a tópicos si lo necesitas
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  dht.setup(PIN_TEMP, DHTesp::DHT11); // Cambiar DHT11 por DHT22 si necesario

  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("No se encuentra la pantalla OLED");
    for(;;); // Bucle infinito
  }

  // Configuración inicial de la pantalla OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publica lecturas del sensor DHT cada 15 minutos
  static unsigned long lastPublish = 0;
  if (millis() - lastPublish > WAIT_MIN || lastPublish == 0) {
    float temperature = dht.getTemperature();
    float humidity = dht.getHumidity();

    if (!isnan(temperature) && !isnan(humidity)) { // Verifica si las lecturas son válidas
      char tempStr[8], humStr[8];
      dtostrf(temperature, 1, 2, tempStr);
      dtostrf(humidity, 1, 2, humStr);

      client.publish(topic_temp, tempStr);
      client.publish(topic_hum, humStr);

      // Actualiza la pantalla OLED
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Temperatura: " + String(temperature) + " C");
      display.println("Humedad: " + String(humidity) + " %");
      display.display();
    }
    lastPublish = millis();
  }
}
