#include <iocontrol.h>
#include <SPI.h>
#include <Ethernet.h>

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";

// Создаём объект клиента класса EthernetClient
EthernetClient client;
// Создаём объект iocontrol, передавая в конструктор название панели и клиента
iocontrol mypanel(myPanelName, client);

// MAC адреса Ethernet шилда. Должен быть уникальным в сети
byte mac[] = {
	0xFE, 0xED, 0xDE, 0xAD, 0xFA, 0xCC
};


void setup()
{
	Serial.begin(9600);
	Ethernet.begin(mac);

	// Вызываем функцию первого запроса к сервису
	mypanel.begin();
	// Выводим в монитор последовательного порта информацию о нашей панели
	Serial.println(mypanel.info());
}

void loop()
{
	;
}
