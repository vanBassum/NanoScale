#include <OneWire.h>
#include "Framing.h"
#include "Settings.h"
#include "DeviceManager.h"
#include "Calibrator.h"
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "Config.h"




Settings settings;
OneWire ds(4);
DeviceManager deviceManager;
WiFiMulti wifiMulti;
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

  


int findDeviceSettings(const Address& address)
{
    for(int i = 0; i < DEVICE_SETTINGS_SIZE; i++)
    {
        if(Address(settings.devices[i].address) == address)
        {
            return i;
        }
    }
    return -1;
}


void storeDeviceSettings(Device& device)
{
    int index = findDeviceSettings(device.address);
    if(index != -1)
    {
        settings.devices[index].offset = device.offset;
        settings.devices[index].slope = device.slope;
    }
    else
    {
        index = findDeviceSettings(Address::empty);
        if(index < DEVICE_SETTINGS_SIZE)
        {
            Serial.print("Storing device settings at index ");
            Serial.println(index);
            memcpy(settings.devices[index].address, device.address.data(), Address::addressSize);
            settings.devices[index].offset = device.offset;
            settings.devices[index].slope = device.slope;
        }
        else
        {
            Serial.println("No empty slot found");
        }
    }
    SaveSettings(settings);
}

bool tryLoadDeviceSettings(Device& device)
{
    int index = findDeviceSettings(device.address);
    if(index != -1)
    {
        device.offset = settings.devices[index].offset;
        device.slope = settings.devices[index].slope;
        return true;
    }
    return false;
}

void calibrateDevice(Device& device)
{
    Calibrator calibrator(ds);
    calibrator.Calibrate(device);
}

void deviceConnected(Device& device)
{
    Serial.print("Callback Device connected:    ");
    device.address.Print();
    Serial.println();

    if(!tryLoadDeviceSettings(device))
    {
        calibrateDevice(device);
        storeDeviceSettings(device);
    }
}

void deviceDisconnected(Device& device)
{
    Serial.print("Callback Device disconnected: ");
    device.address.Print();
    Serial.println();
}


void setup()
{
    Serial.begin(115200); // Initialize Serial communication
    delay(1000);    //Wait so the terminal captures the first things as well
    Serial.println("Starting...");
    LoadSettings(settings);
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to wifi");
    while (wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
    }
    Serial.println();

    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

    // Check server connection
    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }


    deviceManager.onConnect = [](Device& device) {
        deviceConnected(device);
    };

    deviceManager.onDisconnect = [](Device& device) {
        deviceDisconnected(device);
    };
}


void loop()
{
    static uint32_t i = 0;
    static float tareWeight = 0;
    deviceManager.Discover(ds);

    if(i % 100 == 0)
    {
        
        int32_t totalValue = 0;
        float totalWeight = 0;
        deviceManager.VisitDevices([&totalValue, &totalWeight](Device& device) {
            int32_t value = device.MeasureRaw(ds);
            float weight = device.MeasureWeight(ds);
            Serial.print("Device = ");
            device.address.Print();
            Serial.print(" Value = ");
            Serial.print(value);
            Serial.print(" Weight = ");
            Serial.println(weight);
            totalWeight += weight;
            totalValue += value;

            Point sensor("weight");
            sensor.addTag("device", "nano");
            sensor.addTag("address", device.address.ToString());
            sensor.addField("value", value / 1000.0);
            sensor.addField("weight", weight / 1000.0);
            client.writePoint(sensor);
        });

        if(totalWeight < 60000)
        {
            // Assume bed is empty, so tare the scale
            tareWeight = totalWeight;
        }
        else
        {
            Point sensor("weight");
            sensor.addTag("device", "esp");
            sensor.addField("value", totalValue / 1000.0);
            sensor.addField("weight", totalWeight / 1000.0);
            sensor.addField("tare", tareWeight / 1000.0);
            client.writePoint(sensor);
        } 
    }
    i++;
    delay(100);
}
