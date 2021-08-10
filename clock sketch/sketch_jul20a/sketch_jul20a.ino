  #include <Adafruit_GFX.h>
  #include <Max72xxPanel.h>
  #include <Wire.h>             // Подключаем бибилиотеку для работы с I2C устройствами
#include <DS3231.h>           // Подключаем библиотеку для работы с RTC DS3231
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>// Библиотека для работы с мп3-плеером
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


  RF24 radio(9, 10);
  const uint64_t pipe = 123456789;
  SoftwareSerial mySerial(2,3); // RX, TX
  DS3231 clock;                 // Связываем объект clock с библиотекой DS3231
  RTCDateTime dt;
  RTCDateTime DateTime;         // Определяем сущность структуры RTCDateTime (описанной в библиотеке DS3231) для хранения считанных с часов даты и времени
  int in_H;
  int in_M;
  int jnoch;
  long clkTime = 0;
  long dotTime = 0;
  boolean but_flag = false;
  boolean but_pred = false;
  uint32_t ms_button = 0;     // переменная для устранения дребезга контактов тактовой кнопки
  int kn_mode = 6;
  int kn_plus = 4;
  int kn_minus = 5;
  int in_day;
  String mont;
  String weatherString;
  int xt;
  int xn;
  int but_11;
  int but_tec;
  int vol;
  int a,xs,xd;
  int in_mon;
  int but_plus;
  int in_S;
  int in_temp_home;
  int in_d;
  int in_year;
  int in_temp;
  int but_minus;
  int updCnt = 0;
  long interval = 13000;
  long previousMillis = 0; 
  int dots = 0;
  String year1;
  String year2;
  int jday;
  byte del=0;
  int offset=1,refresh=0;
  int mode = 0;
  int pinCS = 8; // Подключение пина CS
  int numberOfHorizontalDisplays = 4; // Количество светодиодных матриц по Горизонтали
  int numberOfVerticalDisplays = 1; // Количество светодиодных матриц по Вертикали
  Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
  int wait = 50; // скорость бегущей строки
  int spacer = 2;
  int width = 5 + spacer; // Регулируем расстояние между символами

  
  void setup() {
    Serial.begin (9600);
    radio.begin();
    //delay(2000);
    radio.setDataRate(RF24_1MBPS); // скорость обмена данными RF24_1MBPS или RF24_2MBPS
    radio.setCRCLength(RF24_CRC_16); // длинна контрольной суммы 8-bit or 16-bit
    radio.setPALevel(RF24_PA_MAX); // уровень питания усилителя RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX                             // соответствует уровням:    -18dBm,      -12dBm,      -6dBM,           0dBm
    radio.setChannel(115);         // уствновка канала
    radio.setAutoAck(false);       // - автоответ.
    radio.powerUp();               // в
    radio.openReadingPipe(1, pipe);
    radio.startListening();
    mySerial.begin (9600);
    mp3_set_serial (mySerial); 
     mp3_set_volume (12);
    clock.begin(); 
    clock.setDateTime(__DATE__, __TIME__);
    //matrix.setIntensity(13); // Яркость матрицы от 0 до 15

  // начальные координаты матриц 8*8
    matrix.setRotation(0, 1);        // 1 матрица
    matrix.setRotation(1, 1);        // 2 матрица
    matrix.setRotation(2, 1);        // 3 матрица
    matrix.setRotation(3, 1);        // 4 матрица
  // matrix.setRotation(4, 1);        // 5 матрица
  // matrix.setRotation(5, 1);        // 6 матрица
  // matrix.setRotation(6, 1);        // 7 матрица
  // matrix.setRotation(7, 1);        // 8 матрица
  }

  void loop() {
    data();
    speak();
    but_tec = digitalRead(kn_mode);
    but_plus = digitalRead(kn_plus);
    but_minus = digitalRead(kn_minus);
    
    if (but_tec == HIGH && but_flag == false) {
      but_flag = true;
    }
    
    if (but_tec == LOW && but_flag == true) {
      mode ++;
      but_flag = false;
    }
    
    if (mode > 10) {
      mode = 0;
    }
    
    switch(mode) {
      case 0:Serial.println ("идут часы");
        if(millis()-clkTime > 45000 && !del && dots) { //каждые 45 секунд запускаем бегущую строку
          ScrollText(utf8rus(weatherString)); //тут текст строки, потом будет погода и т.д.
          updCnt--;
          clkTime = millis();
        }
        oclock(); // функция часов
        if(millis()-dotTime > 500) {
          dotTime = millis();
          dots = !dots;
        }
        break;
      case 1:Serial.println ("вход в насторйки");
        clock_mode();
        break;
      case 2:Serial.println ("настройки часов");
        huor();
        break;
      case 3:Serial.println ("настройки минут");
        minut();
        break;
      case 4:Serial.println ("сход в настройки даты");
        clock_data ();
        break;
      case 5:Serial.println ("настройки дня");
        my_day();
        break;
      case 6:Serial.println ("настройки месяца");
        my_month();
        break;
      case 7:Serial.println ("настройки года");
        my_year();
        break;
      case 8:Serial.println ("С 8 до 22");
        jarkost_day();
        break;
      case 9:Serial.println ("С 22 до 8");
        jarkost_noch();
        break;
      case 10:Serial.println ("настройки звука");
        my_volume();
        break;
    }
  }
/******************************************/
  void oclock() {
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    dt = clock.getDateTime();
    in_H = dt.hour;
    in_M = dt.minute;
    //каждую четную секунду печатаем двоеточие по центру (чтобы мигало)
    if(dt.second & 1){
      matrix.drawChar(14, y, (String(":"))[0], HIGH, LOW, 1);
      } else {
      matrix.drawChar(14, y, (String(" "))[0], HIGH, LOW, 1);
    }
    String hour1 = String (in_H/10);
    String hour2 = String (in_H%10);
    String min1 = String (in_M/10);
    String min2 = String (in_M%10);
    int xh = 2;
    int xm = 19;
    matrix.drawChar(xh, y, hour1[0], HIGH, LOW, 1);
    matrix.drawChar(xh+6, y, hour2[0], HIGH, LOW, 1);
    matrix.drawChar(xm, y, min1[0], HIGH, LOW, 1);
    matrix.drawChar(xm+6, y, min2[0], HIGH, LOW, 1);
    if (dt.hour > 5 && dt.hour < 22) {
      matrix.setIntensity(jday);
    } else {
      matrix.setIntensity(jnoch);
    }
     matrix.write(); // Вывод на диспл
  }
/*********************************************/
  void clock_mode() {
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    dt = clock.getDateTime();
    in_H = dt.hour;
    in_M = dt.minute;
    String hour1 = String (in_H/10);
    String hour2 = String (in_H%10);
    String min1 = String (in_M/10);
    String min2 = String (in_M%10);
    int xh = 2;
    int xm = 19;
    if(dt.second & 1){
      matrix.drawChar(14, y, (String(":"))[0], HIGH, LOW, 1);
      matrix.drawChar(xh, y, hour1[0], HIGH, LOW, 1);
      matrix.drawChar(xh+6, y, hour2[0], HIGH, LOW, 1);
      matrix.drawChar(xm, y, min1[0], HIGH, LOW, 1);
      matrix.drawChar(xm+6, y, min2[0], HIGH, LOW, 1);
    } else {
      matrix.drawChar(14, y, (String(" "))[0], HIGH, LOW, 1);
      matrix.drawChar(xh, y, (String(" "))[0], HIGH, LOW, 1);
      matrix.drawChar(xh+6, y, (String(" "))[0], HIGH, LOW, 1);
      matrix.drawChar(xm, y, (String(" "))[0], HIGH, LOW, 1);
      matrix.drawChar(xm+6, y, (String(" "))[0], HIGH, LOW, 1);
    }
    matrix.write(); // Вывод на диспл
  }
/*********************************************/
  void huor() {
    uint32_t ms = millis();
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    dt = clock.getDateTime();
    in_H = dt.hour;
    String hour1 = String (in_H/10);
    String hour2 = String (in_H%10);
    int xh = 2;
    matrix.drawChar(xh, y, hour1[0], HIGH, LOW, 1);
    matrix.drawChar(xh+6, y, hour2[0], HIGH, LOW, 1);
    if ( but_plus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      dt.second = 0;                         // сбрасываем секунды
      dt.hour++;                             // пребавляем единицу к часам
        if (dt.hour > 23) dt.hour = 0;         // если вылезли за границы присваеваем 0
      clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // запоминаем состояние последних настроек
    }
    if ( but_minus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      dt.second = 0;                         // сбрасываем секунды
      dt.hour--;                             // пребавляем единицу к часам
      if (dt.hour < 1) dt.hour = 23;         // если вылезли за границы присваеваем 0
      clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // запоминаем состояние последних настроек
    }
    matrix.write(); // Вывод на диспл
  }
/****************************************************/
  void minut() {
    uint32_t ms = millis();
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    dt = clock.getDateTime();
    in_H = dt.hour;
    in_M = dt.minute;
    String min1 = String (in_M/10);
    String min2 = String (in_M%10);
    int xm = 19;
    matrix.drawChar(xm, y, min1[0], HIGH, LOW, 1);
    matrix.drawChar(xm+6, y, min2[0], HIGH, LOW, 1);
    if ( but_plus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      dt.second = 0;                         // сбрасываем секунды
      dt.minute++;                             // пребавляем единицу к часам
      if (dt.minute > 59) dt.minute = 0;         // если вылезли за границы присваеваем 0
      clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // запоминаем состояние последних настроек
    }
    if ( but_minus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      dt.second = 0;                         // сбрасываем секунды
      dt.minute--;                             // пребавляем единицу к часам
      if (dt.minute < 1) dt.minute = 59;         // если вылезли за границы присваеваем 0
      clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // запоминаем состояние последних настроек
    }
    matrix.write(); // Вывод на диспл
  }
  
    void ScrollText (String text){
      for ( int i = 0 ; i < width * text.length() + matrix.width() - 1 - spacer; i++ ) {
        if (refresh==1) i=0;
          refresh=0;
          matrix.fillScreen(LOW);
          int letter = i / width;
          int x = (matrix.width() - 1) - i % width;
          int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
 
          while ( x + width - spacer >= 0 && letter >= 0 ) {
            if ( letter < text.length() ) {
              matrix.drawChar(x, y, text[letter], HIGH, LOW, 1);
            }
           letter--;
           x -= width;
         }
        matrix.write(); // Вывод на дисплей
      delay(wait);
    } 
  }
  
  String utf8rus(String source){
    int i,k;
    String target;
    unsigned char n;
    char m[2] = { '0', '\0' };
    k = source.length(); i = 0;
    while (i < k) {
      n = source[i]; i++;
      if (n >= 0xC0) {
        switch (n) {
          case 0xD0: {
            n = source[i]; i++;
            if (n == 0x81) { n = 0xA8; break; }
            if (n >= 0x90 && n <= 0xBF) n = n + 0x30-1;
            break;
          }
          case 0xD1: {
            n = source[i]; i++;
            if (n == 0x91) { n = 0xB8; break; }
            if (n >= 0x80 && n <= 0x8F) n = n + 0x70-1;
            break;
          }
        }
      }
      m[0] = n; target = target + String(m);
    }
    return target;
  }

  void data() {
    matrix.fillScreen(LOW);
    clock.getDateTime();
    int r = (matrix.height() - 8) / 2; // Центри
    mont = dt.month;
      if (mont == "1") mont = "Января";
      if (mont == "2") mont = "Февраля";
      if (mont == "3") mont = "Марта";
      if (mont == "4") mont = "Апреля";
      if (mont == "5") mont = "Мая";
      if (mont == "6") mont = "Июня";
      if (mont == "7") mont = "Июля";
      if (mont == "8") mont = "Августа";
      if (mont == "9") mont = "Сентября";
      if (mont == "10") mont = "Октября";
      if (mont == "11") mont = "Ноября";
      if (mont == "12") mont = "Декабря";
    in_day = dt.day;
       String data1 = String (in_day/10);
       String data2 = String (in_day%10);
    in_year = dt.year;
       String year1 = String (in_year/10);
       String year2 = String (in_year%10);
    in_temp_home = clock.readTemperature();
       String temp1 = String (in_temp_home/10);
       String temp2 = String (in_temp_home%10);
       //weatherString = "Всем привет. Я бегущая строка";
    weatherString = String(data1)+(data2)+" "+(mont)+" "+String(year1)+(year2)+" t: "+String(temp1)+(temp2)+" дома";
//       if (radio.available()) {
//int temperature = 0;
//if (!radio.read(&temperature, sizeof(int))) {
////Serial.println("ACK not received by client.");
//}
////Serial.print("Temperature : ");
////Serial.println(temperature);
//    String street1 = String (temperature/10);
//    String street2 = String (temperature%10);
//    //weatherString = String(data1)+(data2)+" "+(mont)+" "+String(year1)+(year2)+" t: "+String(temp1)+(temp2)+" дома"+" и "+String(street1)+(street2)+" на улице";
//    weatherString = "Всем привет. Я бегущая строка";
//   // weatherString = String(data1)+(data2)+" "+(mont)+" "+String(year1)+(year2)+" t: "+String(temp1)+(temp2)+" дома";
//    
//     //matrix.write(); // Вывод на диспл
//       } 
}

  void clock_data () {
    uint32_t ms = millis();
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    dt = clock.getDateTime();
    in_d = dt.day;
    in_mon = dt.month;
    String day1 = String (in_d/10);
    String day2 = String (in_d%10);
    String mon1 = String (in_mon/10);
    String mon2 = String (in_mon%10);
    int xh = 2;
    int xm = 19;
    if(dt.second & 1){
   // matrix.drawChar(14, y, (String(":"))[0], HIGH, LOW, 1);
      matrix.drawChar(xh, y, day1[0], HIGH, LOW, 1);
      matrix.drawChar(xh+6, y, day2[0], HIGH, LOW, 1);
      matrix.drawChar(xm, y, mon1[0], HIGH, LOW, 1);
      matrix.drawChar(xm+6, y, mon2[0], HIGH, LOW, 1);
    } else {
   // matrix.drawChar(14, y, (String(" "))[0], HIGH, LOW, 1);
      matrix.drawChar(xh, y, (String(" "))[0], HIGH, LOW, 1);
      matrix.drawChar(xh+6, y, (String(" "))[0], HIGH, LOW, 1);
      matrix.drawChar(xm, y, (String(" "))[0], HIGH, LOW, 1);
      matrix.drawChar(xm+6, y, (String(" "))[0], HIGH, LOW, 1);
    }
    matrix.write(); // Вывод на диспл
  }

  void my_day() {
    uint32_t ms = millis();
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    dt = clock.getDateTime();
    in_d = dt.day;
    String day1 = String (in_d/10);
    String day2 = String (in_d%10);
    int xh = 2;
    matrix.drawChar(xh, y, day1[0], HIGH, LOW, 1);
    matrix.drawChar(xh+6, y, day2[0], HIGH, LOW, 1);
    if ( but_plus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      dt.second = 0;                         // сбрасываем секунды
      dt.day++;                             // пребавляем единицу к часам
      if (dt.day > 31) dt.day = 0;         // если вылезли за границы присваеваем 0
        clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // запоминаем состояние последних настроек
    }
    if ( but_minus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      dt.second = 0;                         // сбрасываем секунды
      dt.day--;                             // пребавляем единицу к часам
      if (dt.day < 1) dt.day = 23;         // если вылезли за границы присваеваем 0
      clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // запоминаем состояние последних настроек
    }
     matrix.write(); // Вывод на диспл
  }
  
  void my_month() {
    uint32_t ms = millis();
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    dt = clock.getDateTime();
    in_mon = dt.month;
    String mon1 = String (in_mon/10);
    String mon2 = String (in_mon%10);
    int xm = 19;
    matrix.drawChar(xm, y, mon1[0], HIGH, LOW, 1);
    matrix.drawChar(xm+6, y, mon2[0], HIGH, LOW, 1);
    if ( but_plus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      dt.second = 0;                         // сбрасываем секунды
      dt.month++;                             // пребавляем единицу к часам
      if (dt.month > 12) dt.month = 0;         // если вылезли за границы присваеваем 0
      clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // запоминаем состояние последних настроек
    }
    if ( but_minus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      dt.second = 0;                         // сбрасываем секунды
      dt.month--;                             // пребавляем единицу к часам
      if (dt.month < 1) dt.month = 12;         // если вылезли за границы присваеваем 0
      clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // запоминаем состояние последних настроек
    }
    matrix.write(); // Вывод на диспл
  }

  void jarkost_day() {
    uint32_t ms = millis();
    dt = clock.getDateTime();
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    int xh = 11;
    int xm = 19;
    matrix.drawChar(3, y, (String("6"))[0], HIGH, LOW, 1);
    matrix.drawChar(xh, y, (String("-"))[0], HIGH, LOW, 1);
    matrix.drawChar(xm, y, (String("2"))[0], HIGH, LOW, 1);
    matrix.drawChar(xm+6, y, (String("2"))[0], HIGH, LOW, 1);
    matrix.write(); // Вывод на диспл
    if (jday > 7) {
      jday = 0;
    }
    
    if ( but_plus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      jday ++;
    }
    switch(jday) {
      case 0:jday == 3;
        break;
      case 1:jday == 5;
        break;
      case 2:jday == 7;
        break;
      case 3:jday == 9;
        break;
      case 4:jday == 11;
        break;
      case 5:jday == 13;
        break;
      case 6:jday == 15;
        break;
    }
    matrix.setIntensity(jday);
  }
  
  void jarkost_noch() {
    uint32_t ms = millis();
    dt = clock.getDateTime();
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    int xh = 9;
    int xm = 17;
    matrix.drawChar(3, y, (String("2"))[0], HIGH, LOW, 1);
    matrix.drawChar(xh, y, (String("2"))[0], HIGH, LOW, 1);
    matrix.drawChar(xm, y, (String("-"))[0], HIGH, LOW, 1);
    matrix.drawChar(xm+8, y, (String("6"))[0], HIGH, LOW, 1);
    matrix.write(); // Вывод на диспл
    
    if (jnoch > 7) {
      jnoch = 0;
    }
    
    if ( but_plus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      jnoch ++;
    }
    
    switch(jnoch) {
      case 0:jnoch == 3;
        break;
      case 1:jnoch == 5;
        break;
      case 2:jnoch == 7;
        break;
      case 3:jnoch == 9;
        break;
      case 4:jnoch == 11;
        break;
      case 5:jnoch == 13;
        break;
      case 6:jnoch == 15;
        break;
    }
    matrix.setIntensity(jnoch);
  }

  void speak() {
    in_S = dt.second;
    in_H = dt.hour;
    in_M = dt.minute;
    if (in_H == 7 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (1);
        previousMillis = millis(); 
      }
    }
    if (in_H == 8 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (2);
        previousMillis = millis(); 
      }
    }
    if (in_H == 9 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (3);
        previousMillis = millis(); 
      }
    }
    if (in_H == 10 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (4);
        previousMillis = millis(); 
      }
    }
    if (in_H == 11 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (5);
        previousMillis = millis(); 
      }
    }
    if (in_H == 12 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (6);
        previousMillis = millis(); 
      }
    }
    if (in_H == 13 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (7);
        previousMillis = millis(); 
      }
    }
    if (in_H == 14 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (8);
        previousMillis = millis(); 
      }
    }
    if (in_H == 15 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (9);
        previousMillis = millis(); 
      }
    }
    if (in_H == 16 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (10);
        previousMillis = millis(); 
      }
    }
    if (in_H == 17 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (11);
        previousMillis = millis(); 
      }
    }
    if (in_H == 18 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (12);
        previousMillis = millis(); 
      }
    }
    if (in_H == 19 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (13);
        previousMillis = millis(); 
      }
    }
    if (in_H == 20 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (14);
        previousMillis = millis(); 
      }
    }
    if (in_H == 21 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (15);
        previousMillis = millis(); 
      }
    }
    if (in_H == 22 && in_M == 0 && in_S == 0) {
      if (millis() - previousMillis > interval) {
        mp3_play (16);
        previousMillis = millis(); 
      }
    }
  }


  void my_year(){
    uint32_t ms = millis();
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    dt = clock.getDateTime();
    in_year = dt.year;
    int year1 = int (in_year/10);
    int year2 = int (in_year%10);
    String year1_1 = String (year1/10);
    String year2_1 = String (year1%10);
    String year1_2 = String (year2/10);
    String year2_2 = String (year2%10);
    int xt = 4;
    int xn = 17;
    matrix.drawChar(xt, y, year1_1[0], HIGH, LOW, 1);
    matrix.drawChar(xt+6, y, year1_2[0], HIGH, LOW, 1);
    matrix.drawChar(xn, y, year2_1[0], HIGH, LOW, 1);
    matrix.drawChar(xn+6, y, year2_2[0], HIGH, LOW, 1);

    if ( but_plus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      dt.second = 0;                         // сбрасываем секунды
      dt.year++;                             // пребавляем единицу к часам
      if (dt.year > 3000) dt.year = 0;         // если вылезли за границы присваеваем 0
      clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // запоминаем состояние последних настроек
    }
    if ( but_minus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      dt.second = 0;                         // сбрасываем секунды
      dt.year--;                             // пребавляем единицу к часам
      if (dt.year < 1800) dt.year = 3000;         // если вылезли за границы присваеваем 0
        clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // запоминаем состояние последних настроек
    }
    matrix.write(); // Вывод на диспл
  }

  void my_volume() {
    uint32_t ms = millis();
    matrix.fillScreen(LOW);
    int a = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    int xs = 3;
    int xd = 17;
    String vol1 = String (vol/10);
    String vol2 = String (vol%10);
    matrix.drawChar(xs, a, (String("v"))[0], HIGH, LOW, 1);
    matrix.drawChar(xs+6, a, (String("o"))[0], HIGH, LOW, 1);
    matrix.drawChar(xs+11, a, (String("l"))[0], HIGH, LOW, 1);
    matrix.drawChar(xd+3, a, vol1[0], HIGH, LOW, 1);
    matrix.drawChar(xd+9, a, vol2[0], HIGH, LOW, 1);
    matrix.write(); // Вывод на диспл
     
    if ( but_plus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      vol ++;
      if (vol > 30) {
        vol = 0;
      }
    }
    if ( but_minus == HIGH && but_pred == false && (ms - ms_button)>250) {
      ms_button = ms;
      vol --;
      if (vol < 1) {
        vol = 30;
      }
    }
    mp3_set_volume (vol);
  }

