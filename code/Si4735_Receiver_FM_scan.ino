#include <Wire.h>

#define RSTB 2        // пин подключения вывода RSTB
#define ADDRESS 0x11

boolean tune_flag = false;
String out[3];

void get_rev() {    // функция получения информации о чипе
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x10);
  Wire.endTransmission();
  delay(1);
  Wire.requestFrom(ADDRESS, 15, true);
  Serial.println("REVISION BYTES:");
  while (Wire.available())
  {
    Serial.println(Wire.read(), HEX);
  }
  Serial.println();
}

void fm_seek_start() {
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x21);
  Wire.write(4);
  Wire.endTransmission();
  //Serial.println("SEEK");
}

unsigned long fm_tune_status() {    // эта функция будет возвращать частоту
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x22);
  Wire.write(0);
  Wire.endTransmission();
  delay(1);
  Wire.requestFrom(ADDRESS, 8, true);   // запрашиваем ответ
  byte response[8];
  for (int i = 0; i < 8; i++)
  {
    response[i] = Wire.read();
  }
  if (response[0] == 0x81) {           // после того, как нашли радиостанцию, выводим информацию
    unsigned long req = (unsigned long)((response[2] << 8 | response[3])) * 10000;
    return req;
  } else {
    return 0;
  }
}

void power_up() {
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x01);
  Wire.write(16);
  Wire.write(5);
  Wire.endTransmission(false);
  delay(100);
  Wire.requestFrom(ADDRESS, 1, true);   // запрашиваем байт STATUS
  Serial.print("Status: ");
  Serial.println(Wire.read(), HEX);
  Serial.println("POWER_UP \n");
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");

  pinMode(RSTB, OUTPUT);         // тут операция перезагрузки Si4735
  digitalWrite(RSTB, LOW);
  delay(100);
  pinMode(RSTB, INPUT);

  Wire.begin();                  // инициализируем библиотеку

  power_up();    // включаем приёмник (POWER_UP)
  get_rev();
}

void loop()
{
  if (Serial.available()) {
    unsigned long lastFreq;
    if (Serial.read() == 'p') {
      Serial.println("Scanning...");
      boolean firstScan = true;
      unsigned long freq;
      for (int i = 0; i < 3; i++) {
        Serial.print((String)(i+1) + ": ");
        while (true) {
          lastFreq = freq;
          if (!firstScan) {
            Serial.print(freq);
            Serial.print("  ");
          }
          
          fm_seek_start();
          delay(300);

          do {
            freq = fm_tune_status();
          } while (freq == 0);

          if (freq > lastFreq && !firstScan)
            break;
            
          if (firstScan) {
            firstScan = false;
          }
        }
        Serial.println();
      }
    }
  }
}
