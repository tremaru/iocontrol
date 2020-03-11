/*
 * Пример записи переменных на сайт iocontrol.ru
 * на ESP32. Необходимо заполнить название
 * панели myPanelName и название переменных myPanel*Name,
 * на название переменных созданных на сайте iocontrol.ru
 * Подробнее https://iocontrol.ru/blog/ESP32-example-6
 */

#include <iocontrol.h>
#include <WiFi.h>

const char* ssid = "ssid_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";
// Название целочисленной переменной на сайте iocontrol.ru
const char* myPanelIntName = "название_переменной_int";
const char* myPanelFloatName = "название_переменной_float";
const char* myPanelStringName = "название_переменной_string";

// Переменные, значение которых необходимо записать
int myInt = 42;
float myFloat = 3.14;
String myString = "myTestString";

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

	mypanel.write(myPanelIntName, myInt);
	mypanel.write(myPanelFloatName, myFloat);
	mypanel.write(myPanelStringName, myString);

	int status = mypanel.writeUpdate();

	// Если статус равен константе OK...
	if (status == OK) {

		Serial.println("Updated");
	}
	else {

		Serial.println(status);
	}
}
