/*
 * Пример чтения текстовой переменной с сайта iocontrol.ru
 * ESP32. Необходимо заполнить название панели
 * myPanelName и название переменной myVarName,
 * на название переменной созданной на сайте iocontrol.ru
 * Подробнее https://iocontrol.ru/blog/ESP32-example-4
 */

#include <iocontrol.h>
#include <WiFi.h>

const char* ssid = "ssid_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";
// Название переменной строки на сайте iocontrol.ru
const char* myVarName = "название_переменной";

// Если панель использует ключ
// const char* key = "ключ";

// Создаём объект клиента класса WiFiClient
WiFiClient client;
// Создаём объект iocontrol, передавая в конструктор название панели и клиента
iocontrol mypanel(myPanelName, client);

// Если панель использует ключ
// iocontrol mypanel(myPanelName, key, client);

void setup()
{
	Serial.begin(115200);
	WiFi.begin(ssid, password);

	// Ждём подключения
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println();

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

		// Записываем строку в объект
		String myString = mypanel.readString(myVarName);
		// Выводим значение в монитор последовательного порта
		Serial.println(myString);
	}
}
