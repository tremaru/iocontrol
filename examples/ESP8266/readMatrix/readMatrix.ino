/*
 * Пример чтения изображения монохромной матрицы 8x8 с сайта iocontrol.ru
 * на Arduino с Ethernet Shield'ом. Необходимо заполнить название
 * панели myPanelName и название переменной myVarName,
 * на название переменной созданной на сайте iocontrol.ru
 * Подробнее https://iocontrol.ru/blog/ESP32-example-11
 */

#include <iocontrol.h>
#include <ESP8266WiFi.h>
#include <iarduino_I2C_Matrix_8x8.h>

// Переменная для хранения изображения матрицы
uint8_t image[8]{0};

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";
// Название переменной с типом int на сайте iocontrol.ru
const char* myVarName = "название_переменной";
// Если панель использует ключ
// const char* key = "ключ";

const char* ssid = "ssid_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

// Создаём объект клиента класса WiFiClient
WiFiClient client;
// Создаём объект iocontrol, передавая в конструктор название панели и клиента
iocontrol mypanel(myPanelName, client);
// Если панель использует ключ
// iocontrol mypanel(myPanelName, key, client);

// Объект I2C матрицы
iarduino_I2C_Matrix_8x8 disp;

void setup()
{
	Serial.begin(115200);
	WiFi.begin(ssid, password);

	// Ждём подключения
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

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
