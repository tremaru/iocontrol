#include <iocontrol.h>
#include <SPI.h>
#include <WiFi.h>

const char* ssid = "ssid_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

// Устанавливаем кол-во знаков после точки
// (для правильной работы на сайте в настройках
// переменной должно стоять такое же количество)
const uint8_t prec = 5;
// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";
// Название переменной с типом int на сайте iocontrol.ru
const char* myPanelVar = "название_переменной";

// Создаём объект клиента класса WiFiClient
WiFiClient client;
// Создаём объект iocontrol, передавая в конструктор название панели и клиента
iocontrol mypanel(myPanelName, client);


void setup()
{
	Serial.begin(9600);
	WiFi.begin(ssid, password);

	// Вызываем функцию первого запроса к сервису
	mypanel.begin();
}

void loop()
{
	// Обновляем переменные в памяти и записываем
	// статус запроса в переменную
	int status = mypanel.readUpdate();

	// Если статус равен константе OK...
	if (status == OK) {

		// Записываем значение в переменную
		bool myBool = mypanel.readBool(myPanelVar);

		// Выводим значение в монитор последовательного порта
		Serial.println(myBool);
	}
}
