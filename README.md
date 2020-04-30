# radioreceiver-fd
Digital Radio Receiver for SW

В наши дни радиоприёмник - вещь, считающаяся устаревшей, без которой можно жить полноценно. А ведь когда-то радио было чуть ли не единственным средством передачи информации быстро и далеко. Сейчас место радио занял интернет и телевидение. Обсуждая момент бытового применения радио мы упускаем тот факт, что радио - это не только способ услышать ведущего с новостями или музыкальную композицию, но и способ передачи информации в широком смысле. Телевидение, интернет, спутниковые системы - всё это не может существовать без радио. 

Сейчас радиоприёмник, а тем более широкодиапазонный, можно применить для разных целей. Можно послушать на радиолюбительских диапазонах чью-либо речь; послушать аэропорты (можно слушать АТИС (автоматическая передача информации в районе аэродрома), там иногда интересные сводки погодные говорят) и самое интересное, на мой взгляд, половить спутники. Можно записать сигнал с метеорологического спутника, и потом расшифровать его, получив красивые изображения нашей планеты в разных диапазонах, начиная от видимого, кончая дальним инфракрасным.

<div class="row">
  <div class="col-xl-6 offset-xl-3 col-sm-12 text-center">
    <img alt="В Петербурге бывает солнечно. На фото видны помехи передачи" src="https://github.com/DushkaF/radioreceiver-fd/blob/master/Питер%20с%20космоса.jpg" class="full-width"/><br/>
  </div>
</div>
<br>
<br>

Я планирую сделать радиоприёмник на микросхеме от Silicon Labs Si7435. Это SW(Short Wave)/LW(Long Wave), AM(Amplitude modulation)/FM(Frequency modulation) приёмник, имеющий следующие поддиапазоны частот:
* LW: 153–279 kHz
* AM: 520–1710 kHz
* SW: 2.3–26.1 MHz
* FM: 64–108 MHz

(на SW и LW амплитудная модуляция)

Приёмник программно управляемым, то есть в связке с микроконтроллером мы можем настраивать в нём многочисленные параметры.
Я предлагаю использовать для программирования приёмника микроконтроллеры серии Atmega от компании Atmel с установленным загрузчиком от Arduino, в моём случае это отладочная плата Arduino Nano. Протокол передачи данных - I<sup>2</sup>C

Схема подключения этого приёмника из даташита:
<div class="row">
  <div class="col-xl-6 offset-xl-3 col-sm-12 text-center">
    <img alt="" src="https://raw.githubusercontent.com/DushkaF/radioreceiver-fd/master/Si4735%20схема%20подключения.PNG" class="full-width"/><br/>
  </div>
</div>
<br>
<br>

Конкретные номиналы компонентов можно найти на страницах 18-20 общего datasheet`а  
Для простейшего запуска приёмника нам понадобится следующая обвязка: конденсаторы c1, c4, три резистора по 10кОм для подтяжки шины I<sup>2</sup>C и подтяжки Reset. Логика приёмника рассчитана на напряжение 3.3В, что необходимо учесть.

#### Немного о работе с протоколом  I<sup>2</sup>C

I<sup>2</sup>C (Inter-Integrated Circuit) - двухпроводная шина передачи данных (не считая земляной шины).  Имеется 2 линии: тактирования (SCLK) и данных (SDA/SDIO). Подключение устройств к линии выглядит следующим образом:


<div class="row">
  <div class="col-xl-6 offset-xl-3 col-sm-12 text-center">
    <img alt="" src="https://raw.githubusercontent.com/el-pths/radioreceiver-fd/master/images/I2C.png" class="full-width"/><br/>
  </div>
</div>
<br>
<br>

На линии есть ведущие, в нашем случае это Arduino, и ведомые, микросхема приёмника, устройства. Обычный уровень сигнала на линиях этой шины - высокий: 5/3,3В, и этот уровень устанавливается подтяжкой их резистором 10кОм к шине нужного напряжения.  Как уже было сказано, Si4735 имеет 3,3В логику, поэтому мы делаем подтяжку к этому напряжению. 

Передача данных может проходить следующим образом: либо чтение, либо запись. Между операциями происходит перезагрузка соединения.

Для работы с этой шиной с помощью Arduino будем использовать стандартную библиотеку ``Wire.h``. Рассмотрим примеры записи и чтения, используя эту библиотеку.

Запись значений в регистр:
```c++
Wire.beginTransmission(address);   // открываем соединение с устройством с адресом address
Wire.write(0x01); // Производим установку номера регистра, в который производим запись
Wire.write(16);   // Записываем n аргументов (это первый)
Wire.write(5);
Wire.endTransmission(false);  // закрываем соединение. true - освобождаем шину. false - перезагружаем slave без потери соединения
``` 

После записи байтов может вернуться ответ, который можно побайтово прочитать, но желательно перед этим сделать небольшую задержку, иначе первый байт может не успеть прочитаться.
Чтение ответа:
```c++
delay(10);
Wire.requestFrom(address, 8, true);  // запрос 8 байтов с устройства с адресом address
while (Wire.available())  // проверяем наличие непрочитанных байтов ответа
{
   Serial.println(Wire.read(), HEX);  // вывод прочитанных байтов в порт в шестнадцатиричном виде
}
``` 
