int ldr = A0;
int ledVermelho = 8;
int ledAmarelo = 9;
int ledVerde = 10;
int buzzer = 11;

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT22
#define DHTPIN 2

DHT_Unified dht(DHTPIN, DHTTYPE);

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// VARIÁVEIS PARA MÉDIAS

float somaTemp = 0;
float somaUmid = 0;
long somaLdr = 0;

int contador = 0;

float mediaTemp = 0;
float mediaUmid = 0;
int mediaLdr = 0;

// CONTROLE DE TEMPO COM MILLIS

unsigned long ultimaLeitura = 0;
unsigned long ultimaTroca = 0;

const int intervaloLeitura = 1000; // 1 segundo
const int intervaloTela = 5000;    // 5 segundos

int telaAtual = 0;

void setup() {

  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.begin(9600);

  dht.begin();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
  delay(2000);

  lcd.clear();
}

void loop() {

  unsigned long agora = millis();

  // LEITURAS DOS SENSORES A CADA 1 SEGUNDO

  if (agora - ultimaLeitura >= intervaloLeitura) {

    ultimaLeitura = agora;

    int valorLdr = analogRead(ldr);

    sensors_event_t tempEvent, humEvent;

    dht.temperature().getEvent(&tempEvent);
    dht.humidity().getEvent(&humEvent);

    if (!isnan(tempEvent.temperature) &&
        !isnan(humEvent.relative_humidity)) {

      somaTemp += tempEvent.temperature;
      somaUmid += humEvent.relative_humidity;
      somaLdr += valorLdr;

      contador++;

      Serial.print("Leitura ");
      Serial.println(contador);
    }

    // CALCULA MÉDIA DE 5 LEITURAS

    if (contador == 5) {

      mediaTemp = somaTemp / 5;
      mediaUmid = somaUmid / 5;
      mediaLdr = somaLdr / 5;

      Serial.println("----- MEDIAS -----");

      Serial.print("Temp: ");
      Serial.println(mediaTemp);

      Serial.print("Umidade: ");
      Serial.println(mediaUmid);

      Serial.print("LDR: ");
      Serial.println(mediaLdr);

      somaTemp = 0;
      somaUmid = 0;
      somaLdr = 0;

      contador = 0;

      atualizarLEDs();
    }
  }

  // TROCA AS TELAS A CADA 5 SEGUNDOS

  if (agora - ultimaTroca >= intervaloTela) {

    ultimaTroca = agora;

    telaAtual++;

    if (telaAtual > 2) {
      telaAtual = 0;
    }

    mostrarTela();
  }
}

// LEDs E BUZZER

void atualizarLEDs() {

  digitalWrite(ledVermelho, LOW);
  digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledVerde, LOW);
  digitalWrite(buzzer, LOW);

  // UMIDADE OU LUZ MUITO CLARA
  if (mediaUmid > 70 || mediaUmid < 50 || mediaLdr > 900) {

    digitalWrite(ledVermelho, HIGH);
    digitalWrite(buzzer, HIGH);
  }

  // TEMPERATURA FORA OU MEIA LUZ
  else if ((mediaTemp > 15 || mediaTemp < 10) || (mediaLdr >= 500 && mediaLdr <= 900)) {

    digitalWrite(ledAmarelo, HIGH);
  }

  // TUDO OK
  else {

    digitalWrite(ledVerde, HIGH);
  }
}


// LCD

void mostrarTela() {

  lcd.clear();

  // TELA 1 - TEMPERATURA

  if (telaAtual == 0) {

    if (mediaTemp >= 10 && mediaTemp <= 15) {

      lcd.setCursor(0, 0);
      lcd.print("Temperatura OK");

    } else if (mediaTemp > 15) {

      lcd.setCursor(0, 0);
      lcd.print("Temp ALTA!");

    } else {

      lcd.setCursor(0, 0);
      lcd.print("Temp BAIXA!");
    }

    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(mediaTemp);
    lcd.print(" *C");
  }

  // TELA 2 - UMIDADE

  else if (telaAtual == 1) {

    if (mediaUmid >= 50 && mediaUmid <= 70) {

      lcd.setCursor(0, 0);
      lcd.print("Umidade OK");

    } else if (mediaUmid > 70) {

      lcd.setCursor(0, 0);
      lcd.print("Umidade ALTA");

    } else {

      lcd.setCursor(0, 0);
      lcd.print("Umidade BAIXA");
    }

    lcd.setCursor(0, 1);
    lcd.print("Umid: ");
    lcd.print(mediaUmid);
    lcd.print("%");
  }

  // TELA 3 - LUMINOSIDADE

  else {

    if (mediaLdr > 900) {

      lcd.setCursor(0, 0);
      lcd.print("Ambiente muito");
      lcd.setCursor(0, 1);
      lcd.print("claro!");

    } else if (mediaLdr >= 500) {

      lcd.setCursor(0, 0);
      lcd.print("Ambiente a meia");
      lcd.setCursor(0, 1);
      lcd.print("luz!");

    }
  }
}