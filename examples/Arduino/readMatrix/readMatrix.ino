/*
 * Пример чтения изображения монохромной матрицы 8x8 с сайта iocontrol.ru
 * на Arduino с Ethernet Shield'ом. Необходимо заполнить название
 * панели myPanelName и название переменной myVarName,
 * на название переменной созданной на сайте iocontrol.ru
 * Подробнее https://iocontrol.ru/blog/arduino-example-11
 */

#include <iocontrol.h>
#include <SPI.h>
#include <Ethernet.h>
#include <iarduino_I2C_Matrix_8x8.h>

// Переменная для хранения изображения матрицы
uint8_t image[8]{0};

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";
// Название переменной с типом int на сайте iocontrol.ru
const char* myVarName = "название_переменной";

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

// Объект I2C матрицы
iarduino_I2C_Matrix_8x8 disp;

void setup()
{
	Serial.begin(9600);
	Ethernet.begin(mac);

	// Инициируем матрицу 8x8
	disp.begin();

	// Выключаем все светодиоды матрицы
	disp.clrScr();

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
		mypanel.readMatrix(myVarName, image);

		// Выводим изображение на матрицу 8x8
		disp.drawImage(image);
	}
}
