#include <iarduino_I2C_SHT.h>
#include <iocontrol.h>
#include <ESP8266WiFi.h>

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

        while(int error = mypanel.begin()) {
                Serial.println(String(error));
                delay(100);
        }

        mysens.begin();
}

void loop()
{
        int error;

        float sens = mysens.getTem();
        mypanel.write(temp, sens, FLOAT_PRECISION);

        if ((error = mypanel.writeUpdate()) == 0) {
                Serial.print("updated: ");
                Serial.println(mypanel.readFloat(temp), FLOAT_PRECISION);
        }
        else if (error == 702)
                Serial.println("Are we there yet?");

        else if (error == 606)
                Serial.println("nothing to write");

        else
                Serial.println(String(error));

        delay(1000);
}
