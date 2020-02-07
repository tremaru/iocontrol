#include <iocontrol.h>
#include <WiFi.h>

const char* ssid = "ssid_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

// Устанавливаем кол-во знаков после точки
// (для правильной работы на сайте в настройках
// переменной должно стоять такое же количество)
const uint8_t prec = 5;
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
                myFloat = mypanel.readFloat(myVarName, prec);
		Serial.println(myFloat, prec);
	}
}
