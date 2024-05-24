
#include <stdlib.h>
#include "HX711.h"
#include "settings.h"
#include "oneWireDevice.h"

#define LOADCELL_DOUT_PIN 4
#define LOADCELL_SCK_PIN 2
#define ONEWIRE_PIN 3

auto hub = OneWireHub(ONEWIRE_PIN);
MyDevice *oneWireDevice = nullptr;
HX711 loadcell;
Settings settings;

int32_t rawValue = 0;

void CMD_ResetDevice(OneWireHub *hub)
{
    Serial.println("Resetting device");
    void(* resetFunc) (void) = 0; //declare reset function @ address 0
    resetFunc();  //call reset
}

void CMD_SaveSettings(OneWireHub *hub)
{
    Serial.println("Saving settings to EEPROM");
    SaveSettings(settings);
}

void CMD_LoadDefaultSettings(OneWireHub *hub)
{
    Serial.println("Loading default settings");
    LoadDefaultSettings(settings);
}

void CMD_SetUUID(OneWireHub *hub)
{
    Serial.println("Setting UUID");
    if(!hub->recv(settings.uuid, 7))
        Serial.println("Failed to set UUID");
}

void CMD_StartWeightRaw(OneWireHub *hub)
{
    Serial.print("Start measurement ");
    uint64_t startTime = millis();
    switch (settings.hx711_mode)
    {
    case HX711_AVERAGE_MODE:
        rawValue = loadcell.read_average(settings.hx711_samples);
        break;
    case HX711_MEDIAN_MODE:
        rawValue = loadcell.read_median(settings.hx711_samples);
        break;
    case HX711_MEDAVG_MODE:
        rawValue = loadcell.read_medavg(settings.hx711_samples);
        break;
    case HX711_RUNAVG_MODE:
        rawValue = loadcell.read_runavg(settings.hx711_samples);
        break;
    default:
        Serial.println("Invalid mode");
        return;
    }
    uint64_t endTime = millis();

    Serial.print(" Done (");
    Serial.print((uint32_t)(endTime - startTime));
    Serial.println(" ms)");
}

void CMD_ReadWeightRaw(OneWireHub *hub)
{
    Serial.println("Sending weight raw = ");
    hub->send((uint8_t*)&rawValue, 4);
}


constexpr static const Command commands[] = {
    // Device commands
    {0x01, CMD_ResetDevice},
    {0x02, CMD_SaveSettings},
    {0x03, CMD_LoadDefaultSettings},
    {0x04, CMD_SetUUID},

    // Sensor commands
    {0x10, CMD_StartWeightRaw},
    {0x11, CMD_ReadWeightRaw},
};

// Setup function
void setup()
{
    Serial.begin(115200); // Initialize Serial communication
    Serial.println("Starting onewire HX711");

    Serial.println("Loading settings");
    LoadSettings(settings);
    PrintSettings(settings);

    Serial.println("Setup loadcell");
    loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    loadcell.set_raw_mode();

    Serial.println("Setup one wire device");
    oneWireDevice = new MyDevice(settings.uuid, commands, sizeof(commands) / sizeof(Command));
    hub.attach(*oneWireDevice);

    Serial.println("Setup done");
}

// Main loop function
void loop()
{
    hub.poll();
    if (hub.hasError())
        hub.printError();
}

