/*
 *  Пример вывода информации о панели с сайта iocontrol.ru
 * на ESP32. Это может быть Ваша панель или любая открытая панель,
 * в постоянную myPanelName необходимо записать название панели
 */

#include <iocontrol.h>
#include <WiFi.h>

const char* ssid = "ssid_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";

// Создаём объект клиента класса WiFiClient
WiFiClient client;
// Создаём объект iocontrol, передавая в конструктор название панели и клиента
iocontrol mypanel(myPanelName, client);

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
	// Выводим в монитор последовательного порта информацию о нашей панели
	Serial.println(mypanel.info());
}

void loop()
{
	;
}
