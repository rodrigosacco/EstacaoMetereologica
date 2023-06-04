#define BLYNK_TEMPLATE_ID "TMPL2s9z6clzw"
#define BLYNK_TEMPLATE_NAME "Johnny TEST"
#define BLYNK_AUTH_TOKEN "8fXJpoK00BnHVy9d_Hhzx1GAb53tUpz4"
#define BLYNK_PRINT Serial

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <BH1750.h>
#include <DHT.h>
#include <DHT_U.h>
#include <math.h>

char auth[] = BLYNK_AUTH_TOKEN;

const char* ssid = "Vasto";
const char* password = "p4ngu1nh4123";

#define SDA 22
#define SCL 21
#define DHT_PIN 4
#define DHT_TYPE DHT11

Adafruit_BMP280 bmp;
BH1750 lightMeter;
DHT_Unified dht(DHT_PIN, DHT_TYPE);

const int pino_rpm = 34;
int rpm;
int rpmAntigo = 0;
volatile byte pulsos;
unsigned long timeold;

unsigned int numero_encoder = 10;

void contador() {
  pulsos++;
}

const int sensorHall1 = 32;
const int sensorHall2 = 33;
const int numLeituras = 5;

float hall1Leituras[numLeituras];
float hall2Leituras[numLeituras];
int indexAtualHall1 = 0;
int indexAtualHall2 = 0;
float hall1;
float hall2;
int angulo;

unsigned long tempoAtual = 0;
int pressaoAtual;
int pressaoAntiga = 0;
int temperaturaAtual;
int temperaturaAntiga = 0;
int umidadeAtual;
int umidadeAntiga = 0;
int altitudeAtual;
int altitudeAntiga = 0;
int anguloAntigo = 0;

String rosa_ventos;

sensors_event_t event;

unsigned long timerDirecao = millis();
unsigned long timerSender = millis();
unsigned long timerSensores = millis();

void setup() {
  Serial.begin(115200);

  // Conecta ao Wi-Fi
  Serial.println();
  Serial.print("[WiFi] Conectando á ");
  Serial.println(ssid);

  //Define o protocolo I2C como 0x76
  if (!bmp.begin(0x76)) {
    Serial.println(" Não foi possível encontrar um sensor BMP280 válido, verifique a fiação ou "
                   "tente outro endereço!");
  }

  //Tentando até conectar
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  Blynk.begin(auth, ssid, password);

  pinMode(pino_rpm, INPUT);
  attachInterrupt(34, contador, FALLING);
  pulsos = 0;
  rpm = 0;
  timeold = 0;

  pinMode(sensorHall1, INPUT);

  for (int i = 0; i < numLeituras; i++) {
    hall1Leituras[i] = 0;
    hall2Leituras[i] = 0;
  }

  Wire.begin(SDA, SCL);
  lightMeter.begin();
}

void loop() {
  tempoAtual = millis();

  Blynk.run();

  direcao();


  Serial.print("Tensão: = ");
  Serial.print(hall1);
  Serial.print(", Tensão 2 = ");
  Serial.println(hall2);

  if (hall1 >= 2.61 && hall1 <= 2.64 && hall2 >= 2.17 && hall2 <= 2.43) {
    angulo = 180;
    rosa_ventos = "SUL";
  }

  if (hall1 >= 2.53 && hall1 <= 2.61 && hall2 >= 2.15 && hall2 <= 2.19) {
    angulo = 135;
    rosa_ventos = "SUDESTE";
  }

  if (hall1 >= 2.35 && hall1 <= 2.48 && hall2 >= 1.71 && hall2 <= 2.08) {
    angulo = 90;
    rosa_ventos = "LESTE";
  }

  if (hall1 >= 2.17 && hall1 <= 2.29 && hall2 >= 1.51 && hall2 <= 1.89) {
    angulo = 45;
    rosa_ventos = "NORDESTE";
  }

  if (hall1 >= 2.13 && hall1 <= 2.14 && hall2 >= 2.29 && hall2 <= 2.58) {
    angulo = 0;
    rosa_ventos = "NORTE";
  }

  if (hall1 >= 2.16 && hall1 <= 2.24 && hall2 >= 2.54 && hall2 <= 2.55) {
    angulo = 315;
    rosa_ventos = "NOROESTE";
  }

  if (hall1 >= 2.29 && hall1 <= 2.40 && hall2 >= 2.60 && hall2 <= 2.88) {
    angulo = 270;
    rosa_ventos = "OESTE";
  }

  if (hall1 >= 2.47 && hall1 <= 2.58 && hall2 >= 2.78 && hall2 <= 3.04) {
    angulo = 225;
    rosa_ventos = "SUDOSTE";
  }

  sensores();
  blynkSender();
}

void direcao() {
    // Sensor de Efeito Hall
    float sensorHallY = analogRead(sensorHall1);
    float sensorHallX = analogRead(sensorHall2);

    hall1Leituras[indexAtualHall1] = sensorHallY;
    hall2Leituras[indexAtualHall2] = sensorHallX;

    indexAtualHall1++;
    indexAtualHall2++;

    if (indexAtualHall1 >= numLeituras && indexAtualHall2 >= numLeituras) {
      indexAtualHall1 = 0;  // Reinicia o índice para a próxima leitura
      indexAtualHall2 = 0;  // Reinicia o índice para a próxima leitura
    }

    float sum1 = 0;
    float sum2 = 0;
    for (int i = 0; i < numLeituras; i++) {
      sum1 += hall1Leituras[i];
      sum2 += hall2Leituras[i];
    }

    float mediaHall1 = sum1 / numLeituras;
    float mediaHall2 = sum2 / numLeituras;

    float hall1Filtrado = mediaHall1 * (5.0 / 4095.0);
    float hall2Filtrado = mediaHall2 * (5.0 / 4095.0);
    hall1 = hall1Filtrado;
    hall2 = hall2Filtrado;
}

void blynkSender() {
  if (rpm != rpmAntigo) {
    rpmAntigo = rpm;
    Blynk.virtualWrite(V5, rpm);
    attachInterrupt(34, contador, FALLING);
  }

  if (altitudeAtual != altitudeAntiga) {
    Blynk.virtualWrite(V4, bmp.readAltitude());
  }

  if (pressaoAtual != pressaoAntiga) {
    pressaoAntiga = pressaoAtual;
    Blynk.virtualWrite(V0, pressaoAtual);  // Pressão
  }
  if (temperaturaAtual != temperaturaAntiga) {
    temperaturaAntiga = temperaturaAtual;
    Blynk.virtualWrite(V1, temperaturaAtual);  // Pressão
  }

  if (umidadeAtual != umidadeAntiga) {
    Blynk.virtualWrite(V3, event.relative_humidity);
  }

  if (altitudeAtual != altitudeAntiga) {
    Blynk.virtualWrite(V4, bmp.readAltitude());
  }

  if (angulo != anguloAntigo) {
    anguloAntigo = angulo;
    Blynk.virtualWrite(V7, angulo);
    Blynk.virtualWrite(V6, rosa_ventos);
  }
}

void sensores() {
  if ((millis() - timerSensores) < 500) {
    //Sensor Barreira + Encoder
    detachInterrupt(34);
    rpm = (60 * 1000 / numero_encoder) / (millis() - timeold) * pulsos;
    timeold = millis();
    pulsos = 0;
    attachInterrupt(34, contador, FALLING);

    pressaoAtual = bmp.readPressure() / 100;

    temperaturaAtual = bmp.readTemperature();

    Blynk.virtualWrite(V2, lightMeter.readLightLevel());  // Luminosidade

    dht.humidity().getEvent(&event);
    umidadeAtual = event.relative_humidity;

    altitudeAtual = bmp.readAltitude();
  }
  if ((millis() - timerSensores) > 1000) {
    timerSensores = millis();
  }
}