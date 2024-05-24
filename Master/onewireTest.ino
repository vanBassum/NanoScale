#include <OneWire.h>
#include <vector>
#include <memory>

OneWire ds(4);

void setup()
{
    Serial.begin(115200); // Initialize Serial communication
}

void loop()
{
    int devCount = 0;
    uint8_t adresses[10][8];

    ds.reset_search();
    while (ds.search(adresses[devCount]) && devCount < 10)
    {
        devCount++;
    }

    Serial.print("Found ");
    Serial.print(devCount);
    Serial.println(" devices");

    for(int i = 0; i < devCount; i++)
    {
        Serial.print("Device ");
        Serial.print(i);
        Serial.print(" Address =  ");
        for(int j = 0; j < 8; j++)
        {
            Serial.print(adresses[i][j], HEX);
            Serial.print(":");
        }
        Serial.println();

        ds.reset();
        ds.select(adresses[i]);
        ds.write(0x10); // Start conversion
        delay(10);
    }

    delay(1000);

    for(int i = 0; i < devCount; i++)
    {
        ds.reset();
        ds.select(adresses[i]);
        ds.write(0x11); // Read value

        int32_t value = 0;
        value |= ds.read();
        value |= ds.read() << 8;
        value |= ds.read() << 16;
        value |= ds.read() << 24;

        Serial.print("Device ");
        Serial.print(i);
        Serial.print(" = ");
        Serial.println(value);
        delay(10);
    }

    delay(1000);
}
