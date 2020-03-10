/*
 * Пример чтения вещественной переменной с сайта iocontrol.ru
 * на Arduino с Ethernet Shield'ом. Необходимо заполнить название
 * панели myPanelName и название переменной myVarName,
 * созданной на сайте iocontrol.ru
 * Подробнее https://iocontrol.ru/blog/arduino-example-3
 */

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
		// Сохраняем значение в переменную
		myFloat = mypanel.readFloat(myVarName);

		// Получаем кол-во знаков после точки
		uint8_t prec = mypanel.getFloatPrec(myVarName);

		// Выводим значение в монитор последовательного порта,
		// указывая кол-во знаков после точки
		Serial.println(myFloat, prec);
	}
}
