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

	// Вызываем функцию первого запроса к сервису
	mypanel.begin();
	// Выводим в монитор последовательного порта информацию о нашей панели
	Serial.println(mypanel.info());
}

void loop()
{
	;
}
