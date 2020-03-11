/*
 *  Пример вывода информации о панели с сайта iocontrol.ru
 * на Arduino с Ethernet Shield'ом.
 * Это может быть Ваша панель или любая открытая панель,
 * в постоянную myPanelName необходимо записать название панели
 */

#include <iocontrol.h>
#include <SPI.h>
#include <Ethernet.h>

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";

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
	// Выводим в монитор последовательного порта информацию о нашей панели
	Serial.println(mypanel.info());
}

void loop()
{
	;
}
