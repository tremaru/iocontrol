[![](http://iocontrol.ru/images/logo.svg?1)](http://iocontrol.ru/)

# iocontrol

Библиотека Arduino IDE для работы с сервисом IoT iocontrol.ru

Библиотека совместима со всеми устройствами, которые используют базовый класс Client для работы с сетевыми соединениями.

Проверено на: Arduino UNO, Arduino MEGA 2560, Piranha UNO, Piranha ULTRA, ESP32 (Metro ESP-32), ESP8266 (Wemos D1 R1).
Для работы с Arduino требуется аппаратная возможность подключения (например, [Ethernet шилд]()) и маршрутизатор со свободным разъёмом LAN, подключённый к сети Интернет. Для ESP32 и ESP8266 требуется точка доступа WiFi с подключением к сети Интернет.

Пример/Example (Ethernet Shield):

``` C++
#include <iocontrol.h>
#include <SPI.h>
#include <Ethernet.h>

// Данные Вашей панели
const char* myPanelName = "название_панели";
const char* myVarName = "название_переменной";

// Если панель использует ключ
// const char* key = "ключ";

// MAC адрес Ethernet шилда
byte mac[] = {
	0xFE, 0xED, 0xBE, 0xEF, 0xFA, 0xCC
};

// Настройки клиента (клиент должен быть передан в конструктор объекта панели)
EthernetClient client;
iocontrol mypanel(myPanelName, client);

// Если панель использует ключ
// iocontrol mypanel(myPanelName, key, client);

void setup()
{
	Serial.begin(9600);
	Ethernet.begin(mac);
	mypanel.begin();
}

void loop()
{
	mypanel.write(myVarName, millis());
	mypanel.writeUpdate();

	int status = mypanel.readUpdate();

	if (status == OK) {

		Serial.println(mypanle.readInt(myVarName));

	}
}
```

Пример/Example (ESP32):

``` C++
#include <iocontrol.h>
#include <WiFi.h>

// Настройки для точки доступа
const char* mySSID = "название_точки_доступа_WiFi";
const char* mySSIDpass = "пароль_точки_доступа_WiFi";

// Данные Вашей панели
const char* myPanelName = "название_панели";
const char* myVarName = "название_переменной";

// Если панель использует ключ
// const char* key = "ключ";

// Настройки клиента (клиент должен быть передан в конструктор объекта панели)
WiFiClient client;
iocontrol mypanel(myPanelName, client);

// Если панель использует ключ
// iocontrol mypanel(myPanelName, key, client);

void setup()
{
	Serial.begin(9600);
	WiFi.begin(mySSID, mySSIDpass);

	// Ждём подключения
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println();
	mypanel.begin();
}

void loop()
{
	mypanel.write(myVarName, millis());
	mypanel.writeUpdate();

	int status = mypanel.readUpdate();

	if (status == OK) {

		Serial.println(mypanel.readInt(myVarName));

	}
}
```
