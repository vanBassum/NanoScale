#pragma once
#include <EEPROM.h>

struct DeviceCalibration
{
    uint8_t address[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    float slope = 1;
    float offset = 0;
    uint32_t crc = 0;
};

struct Settings
{
    DeviceCalibration devices[4];
    uint32_t crc = 0;
};


// https://wiki-content.arduino.cc/en/Tutorial/LibraryExamples/EEPROMCrc
uint32_t calculateCRC(uint8_t *data, uint32_t size)
{
    const uint32_t crc_table[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};

    uint32_t crc = ~0L;
    for (int index = 0; index < size; ++index)
    {
        crc = crc_table[(crc ^ data[index]) & 0x0f] ^ (crc >> 4);
        crc = crc_table[(crc ^ (data[index] >> 4)) & 0x0f] ^ (crc >> 4);
        crc = ~crc;
    }
    return crc;
}



void SaveSettings(Settings& settings)
{
    settings.crc = calculateCRC((uint8_t*)&settings, sizeof(Settings) - sizeof(settings.crc));
    EEPROM.put(0, settings);
}

bool LoadSettings(Settings& settings)
{
    EEPROM.get(0, settings);
    uint32_t calculatedCrc = calculateCRC((uint8_t*)&settings, sizeof(Settings) - sizeof(settings.crc));
    if (settings.crc != calculatedCrc) {
        Serial.println("CRC error: Settings are corrupted");
        Serial.println("Loading default settings");
        settings = Settings();  // Load default settings
        return false;
    }
    else
    {
        Serial.println("Settings loaded from EEPROM");
        return true;
    }
}




