#include <iarduino_I2C_SHT.h>
#include <iocontrol.h>
#include <SPI.h>
#include <Ethernet.h>

iarduino_I2C_SHT mysens;

// MAC адрес Ethernet шилда
byte mac[] = {
        0xFE, 0xED, 0xBE, 0xEF, 0xFA, 0xCC
}

EthernetClient client;
iocontrol mypanel("название_панели_iocontrol", client);

const char* temp = "название_переменной_iocontrol";

void setup()
{
        Serial.begin(9600);

        Ethernet.begin(mac);

        while(int error = mypanel.begin()) {
                Serial.println(error);
                delay(100);
        }

        mysens.begin();
}

void loop()
{
        int error;

        float sens = mysens.getTem();
        mypanel.write(temp, sens);

        if ((error = mypanel.writeUpdate()) == 0) {
                Serial.print("updated: ");
                Serial.println(mypanel.readFloat(temp));
        }
        else if (error == 702)
                Serial.println("Are we there yet?");

        else if (error == 606)
                Serial.println("nothing to write");

        else
                Serial.println(String(error));
        delay(1000);
}
