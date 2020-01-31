#include <iocontrol.h>
#include <iarduino_Metro.h>
#include <WiFi.h>

const char* ssid = "название_точки_доступа_WiFi";
const char* password = "пароль_точки_доступа_WiFi";

WiFiClient client;
iocontrol mypanel("название_панели_iocontrol", client);

const char* sensor = "название_переменной_iocontrol";

void setup()
{
        iarduino_Metro_Start();

        Serial.begin(115200);

        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED) {
              delay(500);
              Serial.print(".");
        }

        while(int error = mypanel.begin()) {
                Serial.println(String(error));
                delay(100);
        }
}

void loop()
{
        int error;

        int light = Metro[0].read(DSL_LUX);
        mypanel.write(sensor, light);

        if ((error = mypanel.writeUpdate()) == 0) {
                Serial.print("updated: ");
                Serial.println(mypanel.readInt(sensor));
        }
        else if (error == 702)
                Serial.println("Are we there yet?");

        else if (error == 606)
                Serial.println("nothing to write");

        else
                Serial.println(String(error));
        delay(1000);
}
