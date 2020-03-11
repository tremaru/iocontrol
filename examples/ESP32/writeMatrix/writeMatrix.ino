/*
 * Пример записи изображения монохромной матрицы 8x8 на сайт iocontrol.ru
 * на Arduino с Ethernet Shield'ом. Необходимо заполнить название
 * панели myPanelName и название переменной myVarName,
 * на название переменной созданной на сайте iocontrol.ru
 * Подробнее https://iocontrol.ru/blog/esp32-example-10
 */

#include <iocontrol.h>
#include <WiFi.h>

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";
// Название текстовой переменной на сайте iocontrol.ru
const char* myVarName = "название_переменной";
// Если панель использует ключ
// const char* key = "ключ";

const char* ssid = "ssid_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

// Переменная, значение которой необходимо записать на сайт
uint8_t image[] = {
					0b00000000,
					0b00100100,
					0b00000000,
					0b00000000,
					0b01000010,
					0b00111100,
					0b00000000,
					0b00000000
};

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

	// Вызываем функцию первого запроса к сервису
	mypanel.begin();
}

void loop()
{

	// Записываем состояние переменной
	mypanel.writeMatrix(myVarName, image);

	// Обновляем переменные на сайте
	int status = mypanel.writeUpdate();

	// Если статус равен константе OK...
	if (status == OK) {

		// Выводим текст в последовательный порт
		Serial.println("Updated");
	}
}
