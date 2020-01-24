//#include <iarduino_I2C_SHT.h>
#include <iocontrol.h>
//#include <SPI.h>
//#include <Ethernet.h>
//#include <ArduinoJson.h>

//iarduino_I2C_SHT mysens(0x09);
//#define mymac {0xFE, 0xED, 0xDE, 0xAD, 0xDE, 0xAD}

iocontrol mypanel("test2");

//byte mac[] = mac;
void setup()
{
        Serial.begin(9600);
        mypanel.begin();
        mypanel.write("myInt2", 43);
        //mysens.begin();
        //memoryGet();
        //Serial.println(sizeof(float));
}

void loop()
{
        if (mypanel.readAll() == 0) {
        int i = mypanel.readInt("myInt2");
        float f = mypanel.readFloat("myFloat3");
        //String s = mypanel.readString("myString");
        //char* c = mypanel.readCstring("myString");
        Serial.println(i);
        Serial.println(f, 3);
        //Serial.println(s);
        //Serial.println(c);
        Serial.println();
        }
}
