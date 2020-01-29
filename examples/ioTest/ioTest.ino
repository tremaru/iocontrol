#include <iarduino_I2C_SHT.h>
#include <iocontrol.h>
//#include <SPI.h>
//#include <Ethernet.h>
//#include <ArduinoJson.h>



iarduino_I2C_SHT mysens;
#define mymac {0xFE, 0xED, 0xDE, 0xAD, 0xDE, 0xAD}

byte mac[] = mymac;

iocontrol mypanel("test1", mac);
//iocontrol mypanel("test1");

String temp = "Temp";

void setup()
{

        Serial.begin(9600);

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
        mypanel.write(temp, sens, 1);

        if ((error = mypanel.writeUpdate()) == 0) {

                Serial.print("updated: ");
                Serial.println(mypanel.readFloat(temp), 1);

        }

        else if (error == 702)
                Serial.println("Are we there yet?");
                

        else if (error == 606)
                Serial.println("nothing to write");


        else
                Serial.println("different error");

        delay(1000);

}

