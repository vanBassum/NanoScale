#include <stdlib.h>
#include "Protocol.h"
#include "HX711.h"


#define LOADCELL_DOUT_PIN   3
#define LOADCELL_SCK_PIN    2

MyProtocolHandler protocol(Serial);
HX711 loadcell;


void HandleError(MyProtocolHandler& protocol, char* data, size_t dataLength) {

}

void HandleGetRawWeight(MyProtocolHandler& protocol, char* data, size_t dataLength) {
    int averages = 7;

    if(dataLength > 0){
        averages = atoi(data);
    }

    float raw = loadcell.read_medavg(averages);

    char buff[12];
    dtostrf(raw, 4, 6, buff);
    protocol.Send("RRAW", buff);
}

// Command list (use constexpr for immutability)
constexpr Command CommandList[] = {
    {"ERRR", HandleError},
    {"DRAW", HandleGetRawWeight},
};

// Setup function
void setup() {
    Serial.begin(115200); // Initialize Serial communication
    protocol.Init(CommandList, sizeof(CommandList) / sizeof(CommandList[0]));
    loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    loadcell.set_raw_mode();
}

// Main loop function
void loop() {
    protocol.Handle();
}

