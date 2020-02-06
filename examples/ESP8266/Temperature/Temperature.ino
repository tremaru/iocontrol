#include <iarduino_I2C_SHT.h>
#include <iocontrol.h>
#include <ESP8266WiFi.h>

// Знаков после запятой для функций write и println
#define FLOAT_PRECISION 1

const char* ssid = "название_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

iarduino_I2C_SHT mysens;

WiFiClient client;
iocontrol mypanel("название_панели_iocontrol", client);

const char* temp = "название_переменной_iocontrol";



void setup()
{
	Serial.begin(115200);

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	while(int status = mypanel.begin()) {
		Serial.println(String(status));
		delay(100);
	}

	mysens.begin();
}

void loop()
{
	int status;

	float sens = mysens.getTem();
	mypanel.write(temp, sens, FLOAT_PRECISION);

	if ((status = mypanel.writeUpdate()) == OK) {
		Serial.print("updated: ");
		Serial.println(mypanel.readFloat(temp), FLOAT_PRECISION);
	}
	else if (status == WAITING)
		Serial.println("Are we there yet?");

	else if (status == NO_CHANGE)
		Serial.println("nothing to write");

	else
		Serial.println(String(status));

	delay(1000);
}
