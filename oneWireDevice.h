#pragma once
#include "OneWireHub.h"
#include "OneWireItem.h"

struct Command
{
    uint8_t cmd;
    void (*func)(OneWireHub *hub);
};

class MyDevice : public OneWireItem
{
    const Command *commands;
    size_t commandCount;

public:
    MyDevice(const uint8_t ID[7], const Command *cmdList, size_t cmdCount)
        : OneWireItem(ID[0], ID[1], ID[2], ID[3], ID[4], ID[5], ID[6]), commands(cmdList), commandCount(cmdCount) {}

    void duty(OneWireHub *hub) final
    {
        uint8_t cmd;
        if (hub->recv(&cmd, 1))
            return;

        Serial.print("cmd: 0x");
        Serial.println(cmd, HEX);

        for (size_t i = 0; i < commandCount; ++i)
        {
            if (commands[i].cmd == cmd)
            {
                commands[i].func(hub);
                return;
            }
        }

        Serial.println("Command not recognized.");
    }
};


