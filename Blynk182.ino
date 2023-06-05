#define BLYNK_TEMPLATE_ID "TMPL2s9z6clzw"  // ID do template no Blynk
#define BLYNK_TEMPLATE_NAME "Johnny TEST"  // Nome do template no Blynk
#define BLYNK_AUTH_TOKEN "8fXJpoK00BnHVy9d_Hhzx1GAb53tUpz4"  // Token de autenticação do Blynk
#define BLYNK_PRINT Serial  // Configuração de saída de depuração para a porta Serial

#include <Wire.h>  // Biblioteca para comunicação I2C
#include <Adafruit_Sensor.h>  // Biblioteca para sensores da Adafruit
#include <Adafruit_BMP280.h>  // Biblioteca para o sensor de pressão e temperatura BMP280
#include <WiFi.h>  // Biblioteca para comunicação Wi-Fi
#include <WiFiClient.h>  // Biblioteca para cliente Wi-Fi
#include <BlynkSimpleEsp32.h>  // Biblioteca para o Blynk no ESP32
#include <BH1750.h>  // Biblioteca para o sensor de luminosidade BH1750
#include <DHT.h>  // Biblioteca para o sensor de umidade e temperatura DHT
#include <DHT_U.h>  // Biblioteca para o sensor de umidade e temperatura DHT unificado
#include <math.h>  // Biblioteca matemática

char auth[] = BLYNK_AUTH_TOKEN;  // Variável para armazenar o token de autenticação do Blynk

const char* ssid = "Vasto";  // SSID da rede Wi-Fi
const char* password = "p4ngu1nh4123";  // Senha da rede Wi-Fi

#define SDA 22  // Pino SDA para comunicação I2C
#define SCL 21  // Pino SCL para comunicação I2C
#define DHT_PIN 4  // Pino de dados para o sensor DHT11
#define DHT_TYPE DHT11  // Tipo do sensor DHT11

Adafruit_BMP280 bmp;  // Objeto para o sensor BMP280
BH1750 lightMeter;  // Objeto para o sensor BH1750
DHT_Unified dht(DHT_PIN, DHT_TYPE);  // Objeto para o sensor DHT11

const int pino_rpm = 34;  // Pino para o sensor de RPM
int rpm;  // Variável para armazenar o valor de RPM
int rpmAntigo = 0;  // Variável para armazenar o valor antigo de RPM
volatile byte pulsos;  // Variável para contar os pulsos do encoder
unsigned long timeold;  // Variável para armazenar o tempo antigo

unsigned int numero_encoder = 10; // Número de pulsos por rotação do encoder

// Função que incrementa o contador de pulsos
void contador() {
  pulsos++;
}

const int sensorHall1 = 32; // Pino do sensor Hall 1
const int sensorHall2 = 33; // Pino do sensor Hall 2
const int numLeituras = 5; // Número de leituras para média dos valores

float hall1Leituras[numLeituras]; // Array para armazenar as leituras do sensor Hall 1
float hall2Leituras[numLeituras]; // Array para armazenar as leituras do sensor Hall 2
int indexAtualHall1 = 0; //Valor atual do array do sensor Hall 1
int indexAtualHall2 = 0; //Valor atual do array do sensor Hall 2
float hall1; // Variável para armazenar a média das leituras do sensor Hall 1
float hall2; // Variável para armazenar a média das leituras do sensor Hall 2
int angulo; // Variável para armazenar o ângulo do vento

unsigned long tempoAtual = 0; //Variável para armazenar o tempo atual
int pressaoAtual; //Variável para armazenar a pressão atual
int pressaoAntiga = 0; //Variável para armazenar a pressão antiga
int temperaturaAtual; //Variável para armazenar a temperatura atual
int temperaturaAntiga = 0; //Variável para armazenar a temperatura antiga
int umidadeAtual; //Variável para armazenar a umidade atual
int umidadeAntiga = 0; //Variável para armazenar a umidade antiga
int altitudeAtual; //Variável para armazenar a altitude atual
int altitudeAntiga = 0; //Variável para armazenar a altitude antiga
int anguloAntigo = 0; //Variável para armazenar o angulo antigo

String rosa_ventos; //Variável para armazenar o nome de qual ponto nas Rosa dos Ventos está apontando

sensors_event_t event; //Cria um objeto na memoria para armazenar os resultados

unsigned long timerDirecao = millis(); //Define o tempo que a função direcao() ira demorar para terminar de rodar
unsigned long timerSender = millis(); //Define o tempo que a função blynkSender() ira demorar para terminar de rodar
unsigned long timerSensores = millis(); //Define o tempo que a função sensores() ira demorar para terminar de rodar

void setup() {
  // Inicializa a comunicação serial
  Serial.begin(115200);

  // Conecta ao Wi-Fi
  Serial.println();
  Serial.print("[WiFi] Conectando á ");
  Serial.println(ssid);

  // Inicializa o sensor BMP280
  if (!bmp.begin(0x76)) {
    Serial.println(" Não foi possível encontrar um sensor BMP280 válido, verifique a fiação ou "
                   "tente outro endereço!");
  }

  // Conecta-se à rede Wi-Fi
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  Blynk.begin(auth, ssid, password); // Inicializa o Blynk com o token de autenticação

  pinMode(pino_rpm, INPUT);  // Configura o pino do sensor de RPM como entrada
  attachInterrupt(34, contador, FALLING);  // Habilita a interrupção
  pulsos = 0;
  rpm = 0;
  timeold = 0;

  for (int i = 0; i < numLeituras; i++) {
    hall1Leituras[i] = 0; // Inicializa o elemento 'i' da matriz hall1Leituras com zero
    hall2Leituras[i] = 0; // Inicializa o elemento 'i' da matriz hall2Leituras com zero
  }

  Wire.begin(SDA, SCL); //Inicia a comunicação do barramento I2C utilizando os pinos SDA e SCL especificados.
  lightMeter.begin(); //Inicia a biblioteca do medidor de luz
}

void loop() {
  tempoAtual = millis(); //Armazena o tempo atual em millisegundos

  Blynk.run(); // Executa o Blynk 

  direcao(); //Chama a função direcao() responsavel por ler e tratar os valores lidos pelos sensores Hall

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

  if (hall1 >= 2.29 && hall1 <= 2.45 && hall2 >= 2.60 && hall2 <= 2.88) {
    angulo = 270;
    rosa_ventos = "OESTE";
  }

  if (hall1 >= 2.47 && hall1 <= 2.60 && hall2 >= 2.75 && hall2 <= 3.04) {
    angulo = 225;
    rosa_ventos = "SUDOSTE";
  }

  sensores(); //Chama a função sensores() responsavel por ler e tratar os valores lidos pelos sensores BH1750, BMP280, DHT11 e LM393
  blynkSender(); //Chama a função direcao() responsavel por ler e tratar os valores lidos pelos sensores Hall
}

void direcao() {
    // Sensor de Efeito Hall
    float sensorHallY = analogRead(sensorHall1);
    float sensorHallX = analogRead(sensorHall2);

    // Armazena as leituras atuais nos arrays hall1Leituras e hall2Leituras
    hall1Leituras[indexAtualHall1] = sensorHallY;
    hall2Leituras[indexAtualHall2] = sensorHallX;

    // Incrementa os índices atuais
    indexAtualHall1++;
    indexAtualHall2++;

    // Verifica se é necessário reiniciar os índices
    if (indexAtualHall1 >= numLeituras && indexAtualHall2 >= numLeituras) {
      indexAtualHall1 = 0;  // Reinicia o índice para a próxima leitura
      indexAtualHall2 = 0;  // Reinicia o índice para a próxima leitura
    }

    // Calcula a média das leituras armazenadas nos arrays hall1Leituras e hall2Leituras
    float sum1 = 0;
    float sum2 = 0;
    for (int i = 0; i < numLeituras; i++) {
      sum1 += hall1Leituras[i];
      sum2 += hall2Leituras[i];
    }

    // Calcula a média final
    float mediaHall1 = sum1 / numLeituras;
    float mediaHall2 = sum2 / numLeituras;

    // Aplica a fórmula para converter tensão em voltagem
    float hall1Filtrado = mediaHall1 * (5.0 / 4095.0);
    float hall2Filtrado = mediaHall2 * (5.0 / 4095.0);

    // Atualiza as variáveis hall1 e hall2 com os valores filtrados
    hall1 = hall1Filtrado;
    hall2 = hall2Filtrado;
}

void blynkSender() {
  //Caso a valor atual seja diferente do valor antigo, ele atualiza os valores que serão enviados para o Blynk
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
    detachInterrupt(34); // Desanexa a interrupção do pino 34
    rpm = (60 * 1000 / numero_encoder) / (millis() - timeold) * pulsos; // Calcula a velocidade em RPM
    timeold = millis(); // Atualiza o valor do tempo antigo para o tempo atual
    pulsos = 0; // Reinicia a contagem de pulsos
    attachInterrupt(34, contador, FALLING); // Anexa a interrupção do pino 34 ao manipulador de interrupção 'contador' no evento de borda de descida

    pressaoAtual = bmp.readPressure() / 100; // Lê a pressão atual em hPa e armazena em 'pressaoAtual'

    temperaturaAtual = bmp.readTemperature(); // Lê a temperatura atual em graus Celsius e armazena em 'temperaturaAtual'

    Blynk.virtualWrite(V2, lightMeter.readLightLevel());  // Lê o nível de luz atual do medidor de luz e envia para o pino virtual V2 no aplicativo Blynk

    dht.humidity().getEvent(&event); // Lê o evento de umidade atual do sensor DHT e armazena em 'event'
    umidadeAtual = event.relative_humidity; // Extrai o valor da umidade relativa atual do 'event' e armazena em 'umidadeAtual'

    altitudeAtual = bmp.readAltitude(); // Lê a altitude atual em metros e armazena em 'altitudeAtual'
  }
  if ((millis() - timerSensores) > 1000) {
    timerSensores = millis(); // Atualiza o valor do temporizador para o tempo atual
  }
}