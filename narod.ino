// отправка данных на народмон.ру с градусника ds18b20
// для ESP8266, ver0.3

// изменена работа с градусниками, теперь работа с 18b20 при помощи библиотеки DallasTemperature
// добавленна работа с несколькоими градусниками, данные отправляются со всех подключенных 18b20
// из основного цикла убран delay

// но как и прошлая версия, оно немного глючит. 

#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>  // https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <Ticker.h>


OneWire  ds(0);  // on pin 2 (a 4.7K resistor is necessary)
DallasTemperature sensors(&ds);

Ticker flipper;


const char* ssid     = "sinkhrofazatron7";
const char* password = "LOLytukbycrfz37IfhfirbyjAflttdf20";

const char* host = "narodmon.ru";
const int httpPort = 8283;
 
const int interval = 6*60; // 6 минут

int tm = interval;
int deviceCount = 0;
  
void flip(){
  tm--;
//  Serial.println(tm);
}

void setup() {
  Serial.begin(115200);
  delay(10);
  sensors.begin();
  delay(10);
  deviceCount = sensors.getDeviceCount();  // узнаем количество подключенных градусников
//  sensors.getAddress(Address18b20, 0);
  sensors.setResolution(10);               // устанавливаем разрешение градусника 9, 10, 11, или 12 bit
  sensors.requestTemperatures();           // забераем температуру с градусника  
  
  Send();                                  // при включении отправляем данные
  flipper.attach(1, flip);                 // запускаем таймер
}

/// функция отправляет данные на народмон.ру
void Send() { 

     // Подключаемся к wifi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();                       // отправляем в Serial данные о подключении
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
  
  // подключаемся к серверу 
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
     return;
  }
  
  // отправляем данные  
  Serial.println("Sending..."); 
      // заголовок
  client.print("#");
  client.print(WiFi.macAddress()); // отправляем МАС нашей ESP8266
  client.print("#");
  client.print("neglinskaya37"); // название устройства
  client.print("#");
  client.print("47.7218#40.291"); // координаты местонахождения датчика
  client.println();

      // в цикле отправляем данные с подключенных градусников.
   for (int i = 0; i <= deviceCount - 1; i++){ 
      DeviceAddress Address18b20;
      sensors.getAddress(Address18b20, i);
      float temp = sensors.getTempC(Address18b20);
      
      client.print("#"); 
      for(int i = 0; i < 8; i++) client.print(Address18b20[i], HEX); // номер 18b20  
      client.print("#");
      client.println(temp); 
   }  
   
   client.println("##");
    
  delay(10);

  // читаем ответ с и отправляем его в сериал
  // вообще на ответ нужно както реагировать
  Serial.print("Requesting: ");  
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  client.stop();
  Serial.println();
  Serial.println();
  Serial.println("Closing connection");

  WiFi.disconnect(); // отключаемся от сети
  Serial.println("Disconnect WiFi.");

    
}

void loop() {
 if (tm == 0){                      // если таймер отработал
    flipper.detach();                 // выключаем
    tm = interval;                    // сбрасываем переменную таймера
    sensors.requestTemperatures();    // забераем температуру с градусника 
    delay(10);   
    Send();                           // отправляем
    flipper.attach(1, flip);          // включаем прерывание по таймеру
  }  
  
 
}

