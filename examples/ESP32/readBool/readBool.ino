#include <iocontrol.h>
#include <WiFi.h>

const char* ssid = "ssid_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

// Название панели на сайте iocontrol.ru
const char* myPanelName = "название_панели";
// Название переменной кнопки на сайте iocontrol.ru
const char* myVarName = "название_переменной";

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
}

void loop()
{
	// Обновляем переменные в памяти и записываем
	// статус запроса в переменную
	int status = mypanel.readUpdate();

	// Если статус равен константе OK...
	if (status == OK) {

		// Записываем значение в переменную
		bool myBool = mypanel.readBool(myVarName);

		// Выводим значение в монитор последовательного порта
		Serial.println(myBool ? "ON" : "OFF");
	}
}
