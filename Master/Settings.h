#pragma once
#include <Preferences.h>
Preferences prefs;

#define DEVICE_SETTINGS_SIZE 4

struct DeviceCalibration
{
    uint8_t address[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    float slope = 1;
    float offset = 0;
};

struct Settings
{
    DeviceCalibration devices[DEVICE_SETTINGS_SIZE];
};


void SaveSettings(Settings& settings)
{
    prefs.begin("my-app"); // use "my-app" namespace

    prefs.putBytes("addr_0", settings.devices[0].address, 8);
    prefs.putFloat("slope_0", settings.devices[0].slope);
    prefs.putFloat("offset_0", settings.devices[0].offset);

    prefs.putBytes("addr_1", settings.devices[1].address, 8);
    prefs.putFloat("slope_1", settings.devices[1].slope);
    prefs.putFloat("offset_1", settings.devices[1].offset);

    prefs.putBytes("addr_2", settings.devices[2].address, 8);
    prefs.putFloat("slope_2", settings.devices[2].slope);
    prefs.putFloat("offset_2", settings.devices[2].offset);

    prefs.putBytes("addr_3", settings.devices[3].address, 8);
    prefs.putFloat("slope_3", settings.devices[3].slope);
    prefs.putFloat("offset_3", settings.devices[3].offset);

    prefs.end();
}

bool LoadSettings(Settings& settings)
{
    prefs.begin("my-app"); // use "my-app" namespace

    prefs.getBytes("addr_0", settings.devices[0].address, 8);
    settings.devices[0].slope = prefs.getFloat("slope_0", 1);
    settings.devices[0].offset = prefs.getFloat("offset_0", 0);

    prefs.getBytes("addr_1", settings.devices[1].address, 8);
    settings.devices[1].slope = prefs.getFloat("slope_1", 1);
    settings.devices[1].offset = prefs.getFloat("offset_1", 0);

    prefs.getBytes("addr_2", settings.devices[2].address, 8);
    settings.devices[2].slope = prefs.getFloat("slope_2", 1);
    settings.devices[2].offset = prefs.getFloat("offset_2", 0);

    prefs.getBytes("addr_3", settings.devices[3].address, 8);
    settings.devices[3].slope = prefs.getFloat("slope_3", 1);
    settings.devices[3].offset = prefs.getFloat("offset_3", 0);

    prefs.end();
    return true;
}




