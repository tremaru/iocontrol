/*
 * Пример записи переменных на сайт iocontrol.ru
 * на Arduino с Ethernet Shield'ом. Необходимо заполнить название
 * панели myPanelName и название переменных myPanel*Name,
 * на название переменных созданных на сайте iocontrol.ru
 * Подробнее https://iocontrol.ru/blog/arduino-example-6
 */

#include <iocontrol.h>
#include <SPI.h>
#include <Ethernet.h>

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

// Создаём объект клиента класса EthernetClient
EthernetClient client;
// Создаём объект iocontrol, передавая в конструктор название панели и клиента
iocontrol mypanel(myPanelName, client);

// Если панель использует ключ
// iocontrol mypanel(myPanelName, key, client);

// MAC адреса Ethernet шилда. Должен быть уникальным в сети
byte mac[] = {
	0xFE, 0xED, 0xDE, 0xAD, 0xFA, 0xCC
};

//  Задаём статический IP-адрес на тот случай,
// если динамическое присвоение адреса даст сбой
// IPAddress ip(192, 168, 1, 31);
// IPAddress myDns(192, 168, 1, 1);

void setup()
{
	Serial.begin(9600);

	// Инициируем Ethernet Shield со статическим адресом
	// Ethernet.begin(mac, ip, myDns);

	// Инициируем Ethernet Shield с использованием DHCP
	Ethernet.begin(mac);

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
