#include <iocontrol.h>
#include <SPI.h>
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

	mypanel.write(myPanelIntName, myInt);
	mypanel.write(myPanelFloatName, myFloat);
	mypanel.write(myPanelStringName, myString);

	int status = mypanel.writeUpdate();

	// Если статус равен константе OK...
	if (status = OK) {

		Serial.println("Updated");
	}
	else {

		Serial.println(status);
	}
}
