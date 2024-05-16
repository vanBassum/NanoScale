#include <stdlib.h>
#include <EEPROM.h>
#include "HX711.h"
#include "Framing/NewLineFraming.h"

#define LOADCELL_DOUT_PIN   3
#define LOADCELL_SCK_PIN    2


class ICommand
{
public:
    virtual bool Execute(IArgumentReader& reader, IArgumentWriter& writer) = 0;

};



struct CommandRecord {
    const char command[5]; // Commands are always 4 characters long
    ICommand* handler;
};


class CommandHandler
{
    IFraming& framing;
    const CommandRecord* commandRegister = nullptr;
    size_t commandCount = 0;

public:
    CommandHandler(IFraming& framing, const CommandRecord* commandRegister, size_t commandCount) : framing(framing), commandRegister(commandRegister), commandCount(commandCount) {}



    void HandleCommand()
    {
        char commandString[32];
        IArgumentReader& reader = framing.ReadFrame().ReadString(commandString, sizeof(commandString));
        ICommand* command = nullptr;
        // Find command in commandRegister
        for (size_t i = 0; i < commandCount; i++) {
            if (strncmp(commandRegister[i].command, commandString, 4) == 0) {
                command = commandRegister[i].handler;
                break;
            }
        }

        if(command == nullptr)
        {
            framing.WriteFrame().WriteString("ERR").WriteString("Command not found").EndFrame();
            return;
        }

        command->Execute(reader, framing.WriteFrame());
    }
};




class TestCommand : public ICommand
{
    bool Execute(IArgumentReader& reader, IArgumentWriter& writer) override
    {
        writer.WriteString("OK").EndFrame();
        return true;
    }
};

class EchoCommand : public ICommand
{
    bool Execute(IArgumentReader& reader, IArgumentWriter& writer) override
    {
        char buffer[64];
        reader.ReadString(buffer, sizeof(buffer));
        writer.WriteString("OK").WriteString(buffer).EndFrame();
        return true;
    }
};


const CommandRecord commandRegister[] = {
    {"TEST", new TestCommand()},
    {"ECHO", new EchoCommand()}
};




HX711 loadcell;
NewLineFraming framing(Serial);
CommandHandler commandHandler(framing, commandRegister, sizeof(commandRegister) / sizeof(CommandRecord));


// Setup function
void setup() {
    Serial.begin(115200); // Initialize Serial communication
    loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    loadcell.set_raw_mode();
}

// Main loop function
void loop() {
    commandHandler.HandleCommand();
}


