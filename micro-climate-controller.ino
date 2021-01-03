#include <LiquidCrystal.h>
#include "src/DHT/DHT.h"

#define DHTPIN 13
#define DHTTYPE DHT11
#define HUMIDIFIERPIN 8
#define HEATERPIN 9
#define AVGCOUNT 5
#define TEMPSETPIN A1
#define HUMIDITYSETPIN A0
#define MINTEMP 68
#define MAXTEMP 80
#define MINHUMIDITY 30
#define MAXHUMIDITY 40

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
DHT dht(DHTPIN, DHTTYPE);

byte upArrow[8] = {
  B00000,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00000
};

uint8_t targetHumidity = 0;
uint8_t targetTemp = 0;
uint8_t humidifierOn = 0;
uint8_t heaterOn = 0;
uint8_t frame = 0;
float rollingHumidity[AVGCOUNT];
float rollingTemp[AVGCOUNT];
uint8_t prevSetTempAnalog = 0;
uint8_t setTempAnalog = 0;
uint8_t prevSetHumidityAnalog = 0;
uint8_t setHumidityAnalog = 0;
unsigned long currMillis = 0;
unsigned long prevMillis = 0;
long readInterval = 2000;

void printClimate(float humidity, float temp);
float rollingAvg(float arr[]);
uint8_t changeTarget(char type);

void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, upArrow);
  dht.begin();

  memset(rollingHumidity, 0, sizeof rollingHumidity);
  memset(rollingTemp, 0, sizeof rollingTemp);

  pinMode(HUMIDIFIERPIN, OUTPUT);
  pinMode(HEATERPIN, OUTPUT);

  prevSetTempAnalog = map(analogRead(TEMPSETPIN), 1015, 15, MINTEMP, MAXTEMP);
  prevSetHumidityAnalog = map(analogRead(HUMIDITYSETPIN), 1015, 15, MINHUMIDITY, MAXHUMIDITY);
  targetHumidity = prevSetHumidityAnalog;
  targetTemp = prevSetTempAnalog;
  
  lcd.setCursor(0, 0);
  lcd.print("Initializing....");
  lcd.setCursor(0, 1);
  lcd.print("       0 %      ");
  delay(500);
  do {
    currMillis = millis();
  
    if (currMillis - prevMillis >= readInterval) {
      prevMillis = currMillis;
      rollingHumidity[frame] = dht.readHumidity();
      rollingTemp[frame] = dht.readTemperature(true);
      frame++;
    }
    if (frame < AVGCOUNT) {
      lcd.setCursor(6, 1);
    } else {
      lcd.setCursor(5, 1);
    }
    lcd.print((int) ((frame) * (10 / AVGCOUNT)));
  } while (frame < AVGCOUNT);
  
  frame = 0;
  delay(500);
  lcd.clear();
}

void loop() {
  currMillis = millis();
  
  if (currMillis - prevMillis >= readInterval) {
    prevMillis = currMillis;
    rollingHumidity[frame] = dht.readHumidity();
    rollingTemp[frame] = dht.readTemperature(true);
    frame++;
    frame %= AVGCOUNT;
  }
  
  float h = rollingAvg(rollingHumidity);
  float t = rollingAvg(rollingTemp);
  printClimate(h, t);

  if (round(h) < targetHumidity && !humidifierOn) {
    digitalWrite(HUMIDIFIERPIN, HIGH);
    humidifierOn = 1;
  } else if (round(h) > targetHumidity && humidifierOn) {
    digitalWrite(HUMIDIFIERPIN, LOW);
    humidifierOn = 0;
  }

  if (round(t) < targetTemp && !heaterOn) {
    digitalWrite(HEATERPIN, HIGH);
    heaterOn = 1;
  } else if (round(t) > targetTemp && heaterOn) {
    digitalWrite(HEATERPIN, LOW);
    heaterOn = 0;
  }

  setTempAnalog = map(analogRead(TEMPSETPIN), 1015, 15, MINTEMP, MAXTEMP);
  setHumidityAnalog = map(analogRead(HUMIDITYSETPIN), 1015, 15, MINHUMIDITY, MAXHUMIDITY);

  if (prevSetTempAnalog != setTempAnalog) {
    targetTemp = changeTarget('t');
    setTempAnalog = map(analogRead(TEMPSETPIN), 1015, 15, MINTEMP, MAXTEMP);
  }

  if (prevSetHumidityAnalog != setHumidityAnalog) {
    targetHumidity = changeTarget('h');
    setHumidityAnalog = map(analogRead(HUMIDITYSETPIN), 1015, 15, MINHUMIDITY, MAXHUMIDITY);
  }

  prevSetTempAnalog = setTempAnalog;
  prevSetHumidityAnalog = setHumidityAnalog;
}

void printClimate(float humidity, float temp) {
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.setCursor(0, 1);
  lcd.print("Humidity:");
  
  lcd.setCursor(6, 0);
  lcd.print(temp);
  lcd.setCursor(14, 0);
  lcd.print(char(223));
  lcd.print("F");
  lcd.setCursor(10, 1);
  lcd.print(humidity);
  lcd.setCursor(15, 1);
  lcd.print("%");

  if (heaterOn) {
    lcd.setCursor(5, 0);
    lcd.write(byte(0));
  } else {
    lcd.setCursor(5, 0);
    lcd.print(" ");
  }

  if (humidifierOn) {
    lcd.setCursor(9, 1);
    lcd.write(byte(0));
  } else {
    lcd.setCursor(9, 1);
    lcd.print(" ");
  }

  return;
}

float rollingAvg(float arr[]) {
  float avg = 0;
  for (int i = 0; i < AVGCOUNT; i++) {
    avg += arr[i];
  }
  return avg/AVGCOUNT;
}

uint8_t changeTarget(char type) {
  if (type == 'h') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Humidity:");
    lcd.setCursor(3, 1);
    lcd.print("%");
    for (int i = 0; i < 40; i++) {
      lcd.setCursor(0, 1);
      lcd.print(map(analogRead(HUMIDITYSETPIN), 1015, 15, MINHUMIDITY, MAXHUMIDITY));
      delay(200);
    }
    lcd.clear();
    return map(analogRead(HUMIDITYSETPIN), 1015, 15, MINHUMIDITY, MAXHUMIDITY);
  } else if (type == 't') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Temp:");
    lcd.setCursor(3, 1);
    lcd.print(char(223));
    lcd.print("F");
    for (int i = 0; i < 40; i++) {
      lcd.setCursor(0, 1);
      lcd.print(map(analogRead(TEMPSETPIN), 1015, 15, MINTEMP, MAXTEMP));
      delay(200);
    }
    lcd.clear();
    return map(analogRead(TEMPSETPIN), 1015, 15, MINTEMP, MAXTEMP);
  }
}
