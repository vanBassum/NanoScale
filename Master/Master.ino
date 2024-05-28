#include <OneWire.h>
#include "Framing.h"
#include "Settings.h"
#include "DeviceManager.h"
#include "Calibrator.h"


Settings settings;
OneWire ds(4);
DeviceManager deviceManager;

void setup()
{
    Serial.begin(115200); // Initialize Serial communication
    delay(1000);    //Wait so the terminal captures the first things as well
    Serial.println("Starting...");
    LoadSettings(settings);
    Serial.println("Settings loaded");
    
    deviceManager.onConnect = [](Device& device) {
        Serial.print("Device connected: ");
        device.Print();
        Serial.println();
        Calibrator calibrator(ds);
        calibrator.Calibrate(device);
    };

    deviceManager.onDisconnect = [](Device& device) {
        Serial.print("Device disconnected: ");
        device.Print();
        Serial.println();
    };
}

void loop()
{
    static int i = 0;
    deviceManager.Discover(ds);

    if(i % 100 == 0)
    {
        deviceManager.VisitDevices([](Device& device) {
            int32_t value = device.MeasureRaw(ds);
            float weight = device.MeasureWeight(ds);
            Serial.print("Device = ");
            device.Print();
            Serial.print(" Value = ");
            Serial.print(value);
            Serial.print(" Weight = ");
            Serial.println(weight);
        });
    }
    i++;

    delay(100);
}
