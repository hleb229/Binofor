#define SEP_BUT 5    // кнопка включения режима поочередного мигания световых лент
#define JOIN_BUT 6   // кнопка включения режима совместного мигания световых лент
#define WAIT_BUT 7   // кнопка включения режима ожидания
#define LAMP1 10     // левая светодиодная лента
#define LAMP2 11     // правая светодиодная лента
#define BEEPER 12    // пьезоизлучатель
#define BACKLIGHT 13 // подсветка
// пины энкодера
#define S1 9
#define S2 8
#define KEY 4

#include "GyverEncoder.h"       // подключение библиотеки для работы с энкодером
Encoder enc(S1, S2, KEY);

#include <LiquidCrystal_I2C.h>  // подключение библиотеки для работы с дисплеем по I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);
// создание "букв" для работы с дисплеем без поддержки русского языка
byte L[8] = {0b01111,0b01001,0b01001,0b01001,0b01001,0b01001,0b11001,0b00000};
byte YA[8] {0b01111,0b10001,0b10001,0b01111,0b00101,0b01001,0b10001,0b00000};
byte I[8] {0b10001,0b10001,0b10011,0b10101,0b11001,0b10001,0b10001,0b00000};
byte YI[8] {0b10101,0b10001,0b10011,0b10101,0b11001,0b10001,0b10001,0b00000};
byte M[8] {0b10000,0b10000,0b10000,0b11110,0b10001,0b10001,0b11110,0b00000};
byte Y[8] {0b10001,0b10001,0b10001,0b10001,0b10001,0b10000,0b10001,0b00000};
byte CH[8] {0b10001,0b10001,0b10001,0b11111,0b00001,0b00001,0b00001,0b00000};

#include <Servo.h>              // подключение библиотеки для работы с сервоприводом
Servo Set_pd;

#include <EEPROM.h>             // подключение библиотеки для работы с энергонезависимой памятью
struct { // создание структуры для сохранения в нее настроек
    double interval;
    int brigtness;
    int quantity;
    unsigned long time;
    int distance;
} set;
// создане глобальных переменных для настроек
double INTERVAL = 0.2;
int BRIGHTNESS = 100, QUANTITY = 2, DISTANCE = 60;
unsigned long TIME = 10;
// создание переменных-счетчиков для разных частей кода
long last_time = 0, last_time1 = 0;
int count = 0, last_count = 0;
// создание булевых переменных-флагов для разных частей кода
bool f = 0, sep_blink = 0, waiting = 0, join_blink = 0, a1 = 0, nastr = 0;
bool a0 = 1, flag = 1, flag1 = 1;

int8_t arrow = 0;     // позиция стрелки

void setup() {
  enc.setType(TYPE2); // определение типа энкодера
  // объявление пинов
  pinMode(SEP_BUT, INPUT_PULLUP);
  pinMode(WAIT_BUT, INPUT_PULLUP);
  pinMode(JOIN_BUT, INPUT_PULLUP);
  pinMode(LAMP1, OUTPUT);
  pinMode(LAMP2, OUTPUT);
  pinMode(BEEPER, OUTPUT);
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(LAMP2, 0);
  digitalWrite(LAMP1, 0);
  //Set_pd.attach(A0);
  // получение настроек из энергонезависимой памяти
  EEPROM.get(0, set);
  INTERVAL=set.interval; BRIGHTNESS = set.brigtness; QUANTITY = set.quantity; TIME = set.time; DISTANCE = set.distance;
  
  lcd.init();        // инициализация дисплея
  lcd.backlight();   // включение подсветки дисплея
  lcd.clear();       // очистка дисплея
  // загрузка русских букв в дисплей
  lcd.createChar(1, L); lcd.createChar(2, YA); lcd.createChar(3, I);  
  lcd.createChar(4, M); lcd.createChar(5, YI); lcd.createChar(6, CH);
  lcd.createChar(7, Y);
  Serial.begin(9600);
  analogWrite(LAMP1, 128);
}

void loop() {
  if (join_blink || sep_blink || waiting) digitalWrite(BACKLIGHT, 0);   // включение и отключение подсветки
  else digitalWrite(BACKLIGHT, 1);
  if (!digitalRead(JOIN_BUT)) {join_blink = !join_blink; delay(150);}   // считывание состояний кнопок режимов
  if (!digitalRead(SEP_BUT)) {sep_blink = !sep_blink; delay(150);}
  if (!digitalRead(WAIT_BUT)) {waiting = !waiting; delay(150);}
  enc.tick(); 
  if (enc.isClick()) flag = !flag;                                      // считывание состояния энкодера
  if (flag) {                                                           // если энкодер не был нажат
    if (nastr) {settings_are_saved();} 
    // сохранение настроек в энергонезависимую память и вывод надписи "настройки сохранены!"
    else interface(); // вывод времени и количества засветов
    if (f == false) {
      last_time = millis();
      last_count = count;
    }
    f = true;
    if (sep_blink) zasvet(1);                // выбор режимов тренировки или ожидания в зависимости от положения кнопок
    else if (join_blink) zasvet(0);
    else if (waiting) {
      if (millis()-last_time1 > TIME*1000) {
        tone(BEEPER, 500, 500);
        waiting = false;
      }
    }
    else {
      f = false;
      analogWrite(LAMP2, 0);
      analogWrite(LAMP1, 0);
      last_time1 = millis();
      last_time = millis();
    }
  }
  else {                                                                 //если энкодер был нажат
    last_time = millis(); nastr = true;
    if (!flag1) {
      lcd.clear();
      settings();
      flag1=true;
    }
    if (enc.isTurn()) { // при повороте ручки энкодера в любую сторону
      lcd.clear();
      if (enc.isRight()) { // при повороте вправо
        arrow++;
        if (arrow > 4) arrow = 0;
      }
      if (enc.isLeft()) { // при повороте влево
        arrow--;
        if (arrow < 0) arrow = 4;
      }
      // при нажатом повороте вправо меняем переменные в большую сторону
      if (enc.isRightH()) {
        if (arrow == 0) INTERVAL+=0.1;
        if (arrow == 1 && BRIGHTNESS != 100) BRIGHTNESS++;
        if (arrow == 2) QUANTITY++;
        if (arrow == 3) TIME++;
        if (arrow == 4 && DISTANCE<70) DISTANCE++;
      }
      // при нажатом повороте влево меняем переменные в меньшую сторону
      if (enc.isLeftH()) {
        if (arrow == 0 && INTERVAL > 0.1) INTERVAL-=0.1;
        if (arrow == 1 && BRIGHTNESS !=0) BRIGHTNESS--;
        if (arrow == 2 && QUANTITY!=0) QUANTITY--;
        if (arrow == 3 && TIME!=0) TIME--;
        if (arrow == 4 && DISTANCE>40) DISTANCE--;
      }
      settings();
    }
  }
}
void settings() {// функция для вывода меню настроек
  if (arrow<2) {
    printRussian("IHTEPBAL:", 1, 0); lcd.setCursor(13, 0); lcd.print(INTERVAL); 
    printRussian("1PKOCTS:", 1, 1); printRight(BRIGHTNESS, 0, 1);
  }
  else if (arrow>=2 && arrow<4) {
    printRussian("KOLI4ECTBO:", 1, 0);  printRight(QUANTITY, 1, 0);
    printRussian("BPEM1:", 1, 1); lcd.setCursor(12, 1); printTime(TIME);
    }
  else if (arrow>=4 && arrow<6) {
    lcd.clear(); printRussian("PACCTO1HIE:", 1, 0); printRight(DISTANCE, 1, 0);
  }
  if (arrow%2==0) lcd.setCursor(0, 0);
  else lcd.setCursor(0, 1);
  lcd.write(126);
}
void interface() {// функция для вывода интерфейса во время проведения засвета
    printRussian("BPEM1     KOL-BO", 0, 0);
    lcd.setCursor(0, 1);  printTime(TIME - (millis()-last_time)/1000); printRussian("          ", 4, 1); 
    if (count<10) lcd.print(' '); lcd.print(count);

    flag1=false;
}
// функция для сохранения настроек в энергонезависимую память и вывода соответствующей надписи на дисплей
void settings_are_saved() {
    nastr = false;
    
    printRussian("HACTPO8KI       ", 0, 0);
    printRussian("      COXPAHEHSY", 0, 1);

    set.interval=INTERVAL; set.brigtness=BRIGHTNESS; set.quantity=QUANTITY; set.time=TIME; set.distance=DISTANCE;

    int angle = map(DISTANCE, 40, 70, 0, 180);

    EEPROM.put(0, set);
    arrow = 0;
    Set_pd.attach(A0);
    Set_pd.write(angle);
    delay(1000);
    Set_pd.detach();
}
void printRussian(String s, byte symb, byte str) { // функция для вывода русских символов на дисплей
  byte len = s.length();
  for (int i = 0; i < len; i++) {
    lcd.setCursor(symb+i, str);
    if (s[i] =='L') lcd.print(char(1));
    else if (s[i] =='1') lcd.print(char(2));
    else if (s[i] =='I') lcd.print(char(3));
    else if (s[i] =='S') lcd.print(char(4));
    else if (s[i] =='8') lcd.print(char(5));
    else if (s[i] =='4') lcd.print(char(6));
    else if (s[i] =='Y') lcd.print(char(7));
    else lcd.print(s[i]);
  }
}
void printTime(int time) {                         // функция для вывода времени
  lcd.print(time/60); 
  lcd.print(":"); 
  if (time%60 < 10) lcd.print(0); 
  lcd.print(time%60);
}
void printRight(int num, bool type, bool str) {    // функция для вывода чисел в ближе к правой границе дисплея
  if (type == 0) {
    lcd.setCursor(15, str); lcd.print("%");
    if (num >= 100) lcd.setCursor(12, str);
    else if (num>9) lcd.setCursor(13, str);
    else lcd.setCursor(14, str);
    lcd.print(num);
  } 
  else {
    if (num >= 100) lcd.setCursor(13, str);
    else if (num>9) lcd.setCursor(14, str);
    else lcd.setCursor(15, str);
    lcd.print(num);
  }
}
void zasvet(bool type) {                                                // функция для основной тренировки
  if (millis() - last_time <= TIME*1000) {
    int INTER = INTERVAL*1000;
    int pwm = 255;                                                                                                           //ТУПЕЙШИЙ КОСТЫЛЬ
    if (INTER == 0) {analogWrite(LAMP1, pwm); analogWrite(LAMP2, pwm);} // если интервал равен 0, не отключать св. ленты
    else {
      if (millis()- last_time1 > INTERVAL*1000) {
        last_time1 = millis();
        if (type) {                                                     // включить режим тренировки поочередного мигания 
          if (a1 == true) {analogWrite(LAMP2, pwm); a1 = false;}
          else {analogWrite(LAMP2, 0); a1 = true;}
          if (a0 == true) {analogWrite(LAMP1, pwm); a0 = false;}
          else {analogWrite(LAMP1, 0); a0 = true;}
        }
        else {                                                          // включить режим тренировки совместного мигания 
          if (a1 == true) {analogWrite(LAMP2, pwm); analogWrite(LAMP1, pwm); a1 = false;}
          else {analogWrite(LAMP1, 0); analogWrite(LAMP2, 0); a1 = true;}
      }
    }
  }
    int last_count = count;
  }
  else {                                        // после прохождения определенного, настраиваемого в настройках времени
    digitalWrite(LAMP1, 0);
    digitalWrite(LAMP2, 0);
    if (last_count == count) {
      count++;
      // при достижении определенного количества выполненных засветов играть музыку с помощью пьезоизлучателя
      if (count == QUANTITY) {final_music();} 
      else tone(BEEPER, 500, 500); // иначе уведомлять оператора об окончании "засвета"
      sep_blink = false;
      join_blink = false;
    }
  }
}
void final_music() { //функция для вывода финальной музыки через пищалку
  tone(BEEPER, 523.25, 133);
  delay(133);
  tone(BEEPER, 523.25, 133);
  delay(133);
  tone(BEEPER, 523.25, 133);
  delay(133);
  tone(BEEPER, 523.25, 400);
  delay(400);
  tone(BEEPER, 415.30, 400);
  delay(400);
  tone(BEEPER, 466.16, 400);
  delay(400);
  tone(BEEPER, 523.25, 133);
  delay(266);
  tone(BEEPER, 466.16, 133);
  delay(133);
  tone(BEEPER, 523.25, 1200);
  delay(1200);
}