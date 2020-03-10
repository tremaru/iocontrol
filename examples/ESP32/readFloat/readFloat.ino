/*
 * Пример чтения вещественной переменной с сайта iocontrol.ru
 * ESP32. Необходимо заполнить название панели
 * myPanelName и название переменной myVarName,
 * созданной на сайте iocontrol.ru
 * Подробнее https://iocontrol.ru/blog/ESP32-example-3
 */

#include <iocontrol.h>
#include <WiFi.h>

const char* ssid = "ssid_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";
// Название вещественной переменной с плавающей точкой на сайте iocontrol.ru
const char* myVarName = "название_переменной";

// Создаём объект клиента класса WiFiClient
WiFiClient client;
// Создаём объект iocontrol, передавая в конструктор название панели и клиента
iocontrol mypanel(myPanelName, client);

// Создаём глобальную переменную для хранения прочитанного значения
float myFloat = 0.0;

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
