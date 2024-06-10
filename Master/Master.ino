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
OneWire oneWire(4);
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
    Calibrator calibrator(oneWire);
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


class CircleBuffer
{   
    constexpr static const int Size = 16;
    float buffer[Size] = {0};
    int wr = 0;

public:

    void Append(float value)
    {
        buffer[wr] = value;
        wr = (wr + 1) % Size; // Update the write index and handle wrap-around
    }

    float Average()
    {
        float sum = 0;
        for (int i = 0; i < Size; i++) {
            sum += buffer[i];
        }
        return sum / Size; // Calculate and return the average
    }

    bool CheckRange(float tolerance)
    {
        float avg = Average();
        for (int i = 0; i < Size; i++) {
            if (abs(buffer[i] - avg) > tolerance) {
                return false; // Return false if any value is outside the tolerance range
            }
        }
        return true; // Return true if all values are within the tolerance range
    }

    void PrintAllMeasurements()
    {
        float avg = Average();
        for (int i = 0; i < Size; i++) {
            Serial.print(round(buffer[i] / 1000));
            if (i < Size - 1) {
                Serial.print(" kg, ");
            }
        }
        Serial.print("Average: ");
        Serial.println(avg);
    }
};


class StateContext;
class State
{
protected:
    StateContext& context;
public:
    State(StateContext& context) : context(context){}
    virtual void Entry() {}
    virtual void Update() {}
};

class StateMeasure;
class StateInit;
class StateSend;
class StateProcess;

class StateContext
{
    State* nextState = nullptr;
    State* state = nullptr;

public:
    DeviceManager& deviceManager;
    OneWire& oneWire;
    CircleBuffer measurements;
    float tareValue = 0;

    StateContext(DeviceManager& deviceManager, OneWire& oneWire) : deviceManager(deviceManager), oneWire(oneWire)
    {
    }

    ~StateContext()
    {
        if(nextState)
        {
            delete nextState;
        }
        if(state)
        {
            delete state;
        }
    }

    template<typename T>
    void Transition()
    {
        static_assert(std::is_base_of<State, T>::value, "T must be a subclass of State");
        nextState = new T(*this);
    }

    void Update()
    {
        
        if(nextState)
        {
            if(state)
            {
                delete state;
            }
            state = nextState;
            nextState = nullptr;
            if(state)
            {
                state->Entry();
            }
            
        }

        if(state)
        {
            state->Entry();
        }
    }
};

class StateSend : public State
{
public:
    constexpr static const char* TAG = "Send";
    StateSend(StateContext& context) : State(context)
    {
    }

    void Entry() override {
        Serial.println("Entered send state");
    }

    void Update() override
    {
        float average = context.measurements.Average();
        float tare = context.tareValue;
        float calculated = average - tare;

        Serial.print("Sending data - ");
        Serial.print("Average: ");
        Serial.print(average);
        Serial.print(", Tare: ");
        Serial.print(tare);
        Serial.print(", Calculated: ");
        Serial.println(calculated);

        Point sensor("weight");
        sensor.addTag("device", "esp");
        //sensor.addField("value", totalValue);
        sensor.addField("average", average);
        sensor.addField("tare", tare);
        sensor.addField("calculated", calculated);
        client.writePoint(sensor);
        context.Transition<StateMeasure>();
    }
};


class StateProcess : public State
{
public:
    constexpr static const char* TAG = "Process";
    StateProcess(StateContext& context) : State(context)
    {

    }

    void Entry() override {
        Serial.println("Entered process state");
    }

    void Update() override {
        // If any value is outside 250 gram, take more measurements
        if(!context.measurements.CheckRange(250))
        {
            Serial.println("Value outside 250 gram range. Taking more measurements.");
            context.measurements.PrintAllMeasurements();
            context.Transition<StateMeasure>();
            return;
        }

        float average = context.measurements.Average();
        if(average < 100000)
        {
            Serial.println("Average below 100000. Setting tare value.");
            context.tareValue = average;
        }

        context.Transition<StateSend>();
    }
};


class StateMeasure : public State
{
    const uint32_t delayMs = 1000;
    uint32_t started = 0;

    void StartMeasurement()
    {
        context.deviceManager.VisitDevices([&](Device& device){
            device.StartMeasurement(context.oneWire);
        });
    }

    float ReadMeasurement()
    {
        float total = 0;
        context.deviceManager.VisitDevices([&](Device& device){
            int32_t raw = device.ReadRawMeasurement(context.oneWire);
            float weight = device.ApplyScaling(raw);
            total += weight;
        });
        return total;
    }

public:
    constexpr static const char* TAG = "Measure";
    StateMeasure(StateContext& context) : State(context)
    {

    }

    void Entry() override
    {
        Serial.println("Entered measure state");
        StartMeasurement();
        started = millis();
    }

    void Update() override
    {
        // Wait for conversion to finish
        uint32_t current = millis();
        if (current - started < delayMs)
            return;

        // Append measurement to list
        float total = ReadMeasurement();
        Serial.print("Measurement completed. Total: ");
        Serial.println(total);
        context.measurements.Append(total);
        context.Transition<StateProcess>();
    }
};

class StateInit : public State
{
public:
    constexpr static const char* TAG = "Init";
    StateInit(StateContext& context) : State(context)
    {

    }

    void Entry() override {
        Serial.println("Entered init state");
    }

    void Update() override
    {
        Serial.println("Initializing device manager.");
        context.deviceManager.Discover(context.oneWire);
        if(context.deviceManager.CountDevices() == 4)
        {
            Serial.println("Device count is 4. Transitioning to measure state.");
            context.Transition<StateMeasure>();
        }
    }
};


StateContext context(deviceManager, oneWire);

void setup()
{
    Serial.begin(115200); // Initialize Serial communication
    delay(1000);    //Wait so the terminal captures the first things as well
    Serial.println("Starting...");

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

    
    LoadSettings(settings);

    for(int i = 0; i < DEVICE_SETTINGS_SIZE; i++)
    {
        Address address(settings.devices[i].address);
        Serial.print("Settings for device found ");
        address.Print();
        Serial.println();
    }


    deviceManager.onConnect = [](Device& device) {
        deviceConnected(device);
    };

    deviceManager.onDisconnect = [](Device& device) {
        deviceDisconnected(device);
    };


    context.Transition<StateInit>();

}

void loop()
{

    context.Update();

}
