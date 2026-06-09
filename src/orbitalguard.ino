#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===================== CONFIGURAÇÕES =====================

const char* SSID          = "Wokwi-GUEST";
const char* PASSWORD      = "";

const char* MQTT_BROKER   = "0c79b338958a441080e6b89fdb8a8015.s1.eu.hivemq.cloud";
const int   MQTT_PORT     = 8883;
const char* MQTT_USER     = "petvital";
const char* MQTT_PASSWORD = "PetVital123";
const char* MQTT_CLIENT   = "orbitalguard-esp32";

const char* TOPIC_NIVEL   = "orbitalguard/nivel";
const char* TOPIC_RISCO   = "orbitalguard/risco";
const char* TOPIC_ALERTA  = "orbitalguard/alerta";

const int TRIG_PIN     = 5;
const int ECHO_PIN     = 18;
const int BTN_PIN      = 19;
const int LED_VERDE    = 26;
const int LED_VERMELHO = 27;
const int BUZZER_PIN   = 14;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===================== VARIÁVEIS =====================

WiFiClientSecure espClient;
PubSubClient     mqtt(espClient);

float    distanciaCm  = 0;
String   nivelRisco   = "BAIXO";
bool     alertaManual = false;
bool     btnAnterior  = HIGH;
bool     mqttConectado = false;

unsigned long ultimoEnvio    = 0;
unsigned long ultimoReconect = 0;
const int INTERVALO_MS       = 3000;
const int RECONECT_MS        = 5000;

// ===================== FUNÇÕES =====================

float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duracao = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duracao == 0) return 999;
  return duracao * 0.034 / 2.0;
}

String calcularRisco(float distancia, bool alerta) {
  if (alerta)           return "ALTO";
  if (distancia < 30.0) return "ALTO";
  if (distancia < 60.0) return "MEDIO";
  return "BAIXO";
}

void atualizarLEDs(String risco) {
  if (risco == "ALTO") {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
    tone(BUZZER_PIN, 1000);
  } else if (risco == "MEDIO") {
    digitalWrite(LED_VERMELHO, LOW);
    noTone(BUZZER_PIN);
    digitalWrite(LED_VERDE, HIGH);
    delay(300);
    digitalWrite(LED_VERDE, LOW);
    delay(300);
  } else {
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_VERMELHO, LOW);
    noTone(BUZZER_PIN);
  }
}

void atualizarDisplay(float distancia, String risco, bool alerta, bool mqtt_ok) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(20, 0);
  display.println("OrbitalGuard");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setCursor(0, 14);
  display.print("Nivel: ");
  display.print(distancia, 1);
  display.println(" cm");

  display.setCursor(0, 28);
  display.print("Risco: ");
  display.setTextSize(2);
  display.setCursor(0, 38);
  if (risco == "ALTO")       display.println("ALTO!");
  else if (risco == "MEDIO") display.println("MEDIO");
  else                       display.println("BAIXO");

  display.setTextSize(1);
  if (alerta) {
    display.setCursor(0, 56);
    display.println("!! PANICO !!");
  } else {
    display.setCursor(0, 56);
    display.print("MQTT: ");
    display.println(mqtt_ok ? "OK" : "...");
  }

  display.display();
}

void tentarConectarMQTT() {
  if (mqtt.connected()) {
    mqttConectado = true;
    return;
  }
  mqttConectado = false;

  unsigned long agora = millis();
  if (agora - ultimoReconect < RECONECT_MS) return;
  ultimoReconect = agora;

  Serial.print("Conectando MQTT...");
  if (mqtt.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println(" conectado!");
    mqttConectado = true;
  } else {
    Serial.printf(" falhou (rc=%d)\n", mqtt.state());
  }
}

void publicarDados(float distancia, String risco, bool alerta) {
  if (!mqtt.connected()) return;

  char bufNivel[10];
  dtostrf(distancia, 4, 1, bufNivel);
  mqtt.publish(TOPIC_NIVEL,  bufNivel);
  mqtt.publish(TOPIC_RISCO,  risco.c_str());
  mqtt.publish(TOPIC_ALERTA, alerta ? "1" : "0");

  Serial.printf("[MQTT] nivel=%.1fcm | risco=%s | alerta=%d\n",
                distancia, risco.c_str(), alerta);
}

// ===================== SETUP =====================

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN,     OUTPUT);
  pinMode(ECHO_PIN,     INPUT);
  pinMode(BTN_PIN,      INPUT_PULLUP);
  pinMode(LED_VERDE,    OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER_PIN,   OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED nao encontrado!");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("OrbitalGuard");
  display.setCursor(10, 35);
  display.println("Conectando...");
  display.display();

  // Wi-Fi
  Serial.printf("Conectando Wi-Fi: %s\n", SSID);
  WiFi.begin(SSID, PASSWORD);
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWi-Fi OK! IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWi-Fi falhou, rodando sem rede.");
  }

  // MQTT
  espClient.setInsecure();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
}

// ===================== LOOP =====================

void loop() {
  tentarConectarMQTT();
  mqtt.loop();

  // Botao de panico
  bool btnAtual = digitalRead(BTN_PIN);
  if (btnAnterior == HIGH && btnAtual == LOW) {
    alertaManual = !alertaManual;
    Serial.printf("Panico: %s\n", alertaManual ? "ATIVO" : "DESATIVADO");
  }
  btnAnterior = btnAtual;

  // Leituras
  distanciaCm = medirDistancia();
  nivelRisco  = calcularRisco(distanciaCm, alertaManual);

  // Saidas
  atualizarLEDs(nivelRisco);
  atualizarDisplay(distanciaCm, nivelRisco, alertaManual, mqttConectado);

  // Publica MQTT
  unsigned long agora = millis();
  if (agora - ultimoEnvio >= INTERVALO_MS) {
    publicarDados(distanciaCm, nivelRisco, alertaManual);
    ultimoEnvio = agora;
  }

  delay(100);
}
