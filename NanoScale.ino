#include <stdlib.h>
#include <EEPROM.h>
#include "Protocol.h"
#include "HX711.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 1

#define LOADCELL_DOUT_PIN   3
#define LOADCELL_SCK_PIN    2

#define STRINGIFY(x) #x
#define CONCATENATE(a, b) a ## b
#define VERSION_STRING STRINGIFY(VERSION_MAJOR) "." STRINGIFY(VERSION_MINOR) "." STRINGIFY(VERSION_PATCH)


MyProtocolHandler protocol(Serial);
HX711 loadcell;

struct Settings
{
    uint8_t major = VERSION_MAJOR;
    uint8_t minor = VERSION_MINOR;
    uint8_t patch = VERSION_PATCH;
    int myValue = 5;

    void reset() {
        *this = Settings();
    }

    void load() {
        EEPROM.get(0, *this);
    }

    void save() {
        EEPROM.put(0, *this);
    }

    // TODO: The future could do upgrades between settings.
    bool checkVersion(){
        if(major != VERSION_MAJOR) return false;
        if(minor != VERSION_MINOR) return false;
        if(patch != VERSION_PATCH) return false;
        return true;
    }

};

Settings settings;

void HandleSettings_SetMyValue(MyProtocolHandler& protocol, char* data, size_t dataLength) {
    settings.myValue = atoi(data);
    protocol.Send("RMYV", "Ok");
}

void HandleSettings_GetMyValue(MyProtocolHandler& protocol, char* data, size_t dataLength) {

    protocol.Send("RMYV", settings.myValue);
}

void HandleSettings_SetDefault(MyProtocolHandler& protocol, char* data, size_t dataLength) {
    settings.reset();
    protocol.Send("RREC", "Ok");
}

void HandleSettings_Save(MyProtocolHandler& protocol, char* data, size_t dataLength) {
    settings.save();
    protocol.Send("RSAV", "Ok");
}

void HandleGetVersion(MyProtocolHandler& protocol, char* data, size_t dataLength) {
    protocol.Send("RVER", VERSION_STRING);
}

void HandleGetRawWeight(MyProtocolHandler& protocol, char* data, size_t dataLength) {
    int averages = 7;
    if(dataLength > 0)
        averages = atoi(data);

    float raw = loadcell.read_medavg(averages);
    protocol.Send("RRAW", raw);
}

// Command list (use constexpr for immutability)
constexpr Command CommandList[] = {
    {"DVER", HandleGetVersion},
    {"DRAW", HandleGetRawWeight},
    {"CREC", HandleSettings_SetDefault},
    {"CSAV", HandleSettings_Save},
    {"DMYV", HandleSettings_GetMyValue},
    {"CMYV", HandleSettings_SetMyValue},
};


// Setup function
void setup() {
    settings.load();

    // If version doenst match, return to default settings.
    if(!settings.checkVersion()){
        settings.reset();
    }

    Serial.begin(115200); // Initialize Serial communication
    protocol.Init(CommandList, sizeof(CommandList) / sizeof(CommandList[0]));
    loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    loadcell.set_raw_mode();
}

// Main loop function
void loop() {
    protocol.Handle();
}

