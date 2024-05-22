#pragma once
#include <EEPROM.h>
#include <HX711.h>


struct Settings
{
    uint8_t uuid[7];
    uint8_t hx711_mode = HX711_MEDAVG_MODE;
    uint8_t hx711_samples = 7;
    uint32_t crc = 0;
};

void PrintSettings(Settings& settings)
{
    Serial.println("Settings:");
    Serial.print(" - UUID:          ");
    Serial.print(String(settings.uuid[0], HEX));
    Serial.print(":");
    Serial.print(String(settings.uuid[1], HEX));
    Serial.print(":");
    Serial.print(String(settings.uuid[2], HEX));
    Serial.print(":");
    Serial.print(String(settings.uuid[3], HEX));
    Serial.print(":");
    Serial.print(String(settings.uuid[4], HEX));
    Serial.print(":");
    Serial.print(String(settings.uuid[5], HEX));
    Serial.print(":");
    Serial.println(String(settings.uuid[6], HEX));
    
    Serial.print(" - HX711 Mode:    ");
    Serial.println(settings.hx711_mode);
    Serial.print(" - HX711 Samples: ");
    Serial.println(settings.hx711_samples);
    Serial.print(" - CRC:           0x");
    Serial.println(settings.crc, HEX);
}

uint32_t calculateCRC(uint8_t *data, uint32_t size);

void SaveSettings(Settings& settings)
{
    settings.crc = calculateCRC((uint8_t*)&settings, sizeof(Settings) - sizeof(settings.crc));
    EEPROM.put(0, settings);
}

void LoadDefaultSettings(Settings& settings)
{
    // Load default settings, when settings are corrupt.
    settings = Settings();  

    // Generate random UUID
    randomSeed(analogRead(0));   
    settings.uuid[0] = random(0, 255);
    settings.uuid[1] = random(0, 255);
    settings.uuid[2] = random(0, 255);
    settings.uuid[3] = random(0, 255);
    settings.uuid[4] = random(0, 255);
    settings.uuid[5] = random(0, 255);
    settings.uuid[6] = random(0, 255);
    settings.crc = calculateCRC((uint8_t*)&settings, sizeof(Settings) - sizeof(settings.crc));
}

void LoadSettings(Settings& settings)
{
    EEPROM.get(0, settings);
    uint32_t calculatedCrc = calculateCRC((uint8_t*)&settings, sizeof(Settings) - sizeof(settings.crc));
    if (settings.crc != calculatedCrc) {
        Serial.println("CRC error: Settings are corrupted");
        Serial.println("Loading default settings");
        LoadDefaultSettings(settings);
        SaveSettings(settings);
    }
    else
    {
        Serial.println("Settings loaded from EEPROM");
    }
}

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


