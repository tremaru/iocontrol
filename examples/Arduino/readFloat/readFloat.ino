#include <iocontrol.h>
#include <SPI.h>
#include <Ethernet.h>

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";
// Название вещественной переменной с плавающей точкой на сайте iocontrol.ru
const char* myVarName = "название_переменной";

// Создаём объект клиента класса EthernetClient
EthernetClient client;
// Создаём объект iocontrol, передавая в конструктор название панели и клиента
iocontrol mypanel(myPanelName, client);

// MAC адреса Ethernet шилда. Должен быть уникальным в сети
byte mac[] = {
	0xFE, 0xED, 0xDE, 0xAD, 0xFA, 0xCC
};

// Создаём глобальную переменную для хранения прочитанного значения
float myFloat = 0.0;

void setup()
{
	Serial.begin(9600);
	Ethernet.begin(mac);

	// Вызываем функцию первого запроса к сервису
	mypanel.begin();
}

void loop()
{
	// Обновляем переменные в памяти и записываем
	// результат функции в переменную
	int status = mypanel.readUpdate();

	// Если статус равен константе OK...
	if (status == OK) {
		// Выводим значение в монитор последовательного порта
                myFloat = mypanel.readFloat(myVarName);
		Serial.println(myFloat);
	}
}
