//пины энкодера
#define CLK 9
#define DT 8
#define SW 0

#include "GyverEncoder.h" //подключение библиотеки для работы с энкодером
Encoder enc1(CLK, DT, SW);

#include <LiquidCrystal_I2C.h>  // подключаем библиотеку для работы с интерфейсом i2c
LiquidCrystal_I2C lcd(0x3f, 16, 2);

#include <EEPROM.h> //подключение библиотеки для работы с энергонезависимой памятью

long last_time = 0;
long last_time1 = 0;
int last_count = 0;
int count = 0;
boolean f = false;
boolean zasvet = false;
boolean notzasvet = false;
boolean mig = false;
boolean a1 = false;
boolean a0 = true;
boolean flag = true;
boolean flag1 = true;
boolean nastr = false;
double INT = 0.0;
int brigh = 0;
int kol = 0;
unsigned long vrem = 0;

int8_t position = 0;  // позиция стрелки
//создание "букв" для работы с дисплеем без поддержки русского языка
byte L[8] = {0b01111,0b01001,0b01001,0b01001,0b01001,0b01001,0b11001,0b00000};
byte YA[8] {0b01111,0b10001,0b10001,0b01111,0b00101,0b01001,0b10001,0b00000};
byte I[8] {0b10001,0b10001,0b10011,0b10101,0b11001,0b10001,0b10001,0b00000};
byte YI[8] {0b10101,0b10001,0b10011,0b10101,0b11001,0b10001,0b10001,0b00000};
byte M[8] {0b10000,0b10000,0b10000,0b11110,0b10001,0b10001,0b11110,0b00000};
byte Y[8] {0b10001,0b10001,0b10001,0b10001,0b10001,0b10000,0b10001,0b00000};
byte D[8] {0b00000,0b00100,0b00000,0b00000,0b00000,0b00100,0b00000,0b00000};

struct { // создание структуры для сохранения в нее настроек
    double IN;
    int brig;
    int ko;
    unsigned long vre;
} set;

void setup() {
    lcd.init();           // инициализация
    lcd.backlight();      // включить подсветку
    lcd.clear();

    pinMode(A5, INPUT_PULLUP); //кнопка моргание
    pinMode(A4, INPUT_PULLUP); //кнопка без моргания
    pinMode(A3, INPUT_PULLUP); //кнопка совместного моргания
    pinMode(10, OUTPUT); //лампа 1
    pinMode(11, OUTPUT); //лампа 2
    pinMode(12, OUTPUT); //пищалка
    pinMode(13, OUTPUT); //подсветка
    digitalWrite(11, 0);
    digitalWrite(10, 0);
    EEPROM.get(0, set);
    Serial.begin(9600);
    INT=set.IN; brigh = set.brig; kol = set.ko; vrem = set.vre;

    lcd.createChar(1, L);
    lcd.createChar(2, YA);
    lcd.createChar(3, I);
    lcd.createChar(4, M);
    lcd.createChar(5, YI);
    lcd.createChar(7, Y);

    enc1.setType(TYPE2);
    lcd.begin(16, 2);
    interface();   // вывод интерфейса
}

void loop() {
  if (!zasvet && !notzasvet && !mig) digitalWrite(13, 1);
  else digitalWrite(13, 0);
  if (!digitalRead(A3)) {mig = !mig; delay(150);}
  if (!digitalRead(A4)) {zasvet = !zasvet; delay(150);}
  if (!digitalRead(A5)) {notzasvet = !notzasvet; delay(150);}
  enc1.tick(); 
  if (enc1.isClick()) flag = !flag;
  if (flag) {
    if (nastr) setting();
    //сохранение настроек в энергонезависимую память и вывод надписи "настройки сохранены!"
    else notnastr();
    // вывод времени и количества засветов
    if (f == false) {
      last_time = millis();
      last_count = count;
    }
    f = true;
    if (zasvet) {
      if (millis() - last_time <= vrem*1000) {
        int INTER = INT*1000;
        int shim = map(brigh, 0, 100, 0, 255);
        shim = constrain(shim, 0, 255);
        if (INTER == 0) {
          analogWrite(10, shim); 
          analogWrite(11, shim);
        } else {
          if (millis()- last_time1 > INT*1000) {
            last_time1 = millis();
            if (a1 == true) {
                analogWrite(11, shim); 
                a1 = false;
            } else {
                digitalWrite(11, 0); 
                a1 = true;
            }
            if (a0 == true) {
              analogWrite(10, shim);
              a0 = false;
            } else {
              digitalWrite(10, 0);
              a0 = true;
            }
          }
        }
        int last_count = count;
      } else {
        digitalWrite(10, 0);
        digitalWrite(11, 0);
        if (last_count == count) {
          count++;
          if (count == kol) {
            fanfare();
            //при достижении определенного настраиваемого количества выполненных засветов играть музыку с помощью пищалки
          } else tone(12, 500, 500); //иначе уведомлять оператора об окончании "засвета"
          zasvet = false;
        }
      }
    }
    else if (notzasvet) {
      if (millis()-last_time1 > vrem*1000) {
        tone(12, 500, 500);
        notzasvet = false;
      }
    } else {
      f = false;
      digitalWrite(11, 0);
      digitalWrite(10, 0);
      last_time1 = millis();
      last_time = millis();
    }
  } else {
    last_time = millis(); nastr = true;
    if (!flag1) {
      lcd.clear();
      interface();
      flag1=true;
    } if (enc1.isTurn()) { //при повороте ручки энкодера в любую сторону
      lcd.clear();
      if (enc1.isRight()) {
        position++;
        if (position >= 4) position = 0;
      } 
      if (enc1.isLeft()) {
        position--;
        if (position < 0) position = 3;
      }
      if (enc1.isRightH()) {
        // при нажатом повороте вправо меняем переменные в большую сторону
        if (position == 0) INT+=0.1;
        if (position == 1 && brigh != 100) brigh++;
        if (position == 2) kol++;
        if (position == 3) vrem++;
      }
      if (enc1.isLeftH()) {
        if (position == 0 && INT > 0.1) INT-=0.1;
        if (position == 1 && brigh !=0) brigh--;
        if (position == 2 && kol!=0) kol--;
        if (position == 3 && vrem!=0) vrem--;
      }
      interface();   // выводим интерфейс
    }
  }
}
void interface() {
  //функция для вывода интерфейса
  lcd.setCursor(0, 0);
  lcd.print(char(3));
  lcd.setCursor(1, 0);
  lcd.print("HT:");
  lcd.print(INT);

  lcd.setCursor(7, 0);
  lcd.print(" ");
  lcd.setCursor(8, 0);
  lcd.print(char(2));
  lcd.setCursor(9, 0);
  lcd.print("PK:");
  lcd.print(brigh);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("K-BO:");
  lcd.print(kol);

  lcd.setCursor(8, 1);
  lcd.print("BPM:");

  lcd.setCursor(12, 1);
  lcd.print(vrem/60);
  lcd.setCursor(13, 1);
  lcd.print(":");
  lcd.setCursor(14, 1);
  if (vrem%60 < 10) {
    lcd.print(0); 
    lcd.setCursor(15, 1);
  } 
  lcd.print(vrem%60);
    
  //выводим стрелку
  if (position == 0) lcd.setCursor(3, 0);
  if (position == 1) lcd.setCursor(11, 0);
  if (position == 2) lcd.setCursor(4, 1);
  if (position == 3) lcd.setCursor(11, 1);
  lcd.write(126);
}
void fanfare() {//функция для вывода "фанфар" через пищалку
  pinMode(12, OUTPUT);
  tone(12, 523.25, 133);
  delay(133);
  tone(12, 523.25, 133);
  delay(133);
  tone(12, 523.25, 133);
  delay(133);
  tone(12, 523.25, 400);
  delay(400);
  tone(12, 415.30, 400);
  delay(400);
  tone(12, 466.16, 400);
  delay(400);
  tone(12, 523.25, 133);
  delay(266);
  tone(12, 466.16, 133);
  delay(133);
  tone(12, 523.25, 1200);
  delay(1200);
}
void notnastr() {//функция для вывода интерфейса во время проведения засвета
  lcd.setCursor(0, 0);
  lcd.print("BPEM");
  lcd.setCursor(4, 0); 
  lcd.print(char(2));
  lcd.setCursor(5, 0); lcd.print("     KO");
  lcd.setCursor(12,0); lcd.print(char(1));
  lcd.setCursor(13, 0); lcd.print("-BO");
  if (count<10) {
    lcd.setCursor(14, 1);
    lcd.print(" ");
    lcd.setCursor(15, 1);
  }
  else lcd.setCursor(14, 1);
  lcd.print(count);
  int pr = vrem - (millis()-last_time)/1000;
  lcd.setCursor(0, 1); lcd.print(pr/60);
  lcd.setCursor(1, 1); lcd.print(":"); lcd.setCursor(2, 1);
  if (pr%60 < 10) {
    lcd.print(0); 
    lcd.setCursor(3, 1);
  }
  else lcd.setCursor(2, 1);
  lcd.print(pr%60); lcd.setCursor(4, 1); lcd.print("          ");
  flag1=false;
}
void setting() {
    //функция для сохранения настроек в энергонезависимую память и вывода соответствующей надписи на дисплей
    nastr = false;
    lcd.setCursor(0, 0); lcd.print("HACTPO"); 
    lcd.setCursor(6,0); lcd.print(char(5));
    lcd.setCursor(7, 0); lcd.print("K");
    lcd.setCursor(8, 0); lcd.print(char(3));
    lcd.setCursor(9, 0); lcd.print("       ");
    lcd.setCursor(0, 1); lcd.print("      "); 
    lcd.setCursor(6, 1); lcd.print("COXPAHEH");
    lcd.setCursor(14, 1); lcd.print(char(4));
    lcd.setCursor(15, 1); lcd.print(char(7));
    set.IN=INT; set.brig=brigh; set.ko=kol; set.vre=vrem;
    EEPROM.put(0, set);
    delay(1000);
}
