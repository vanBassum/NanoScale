#pragma once
#include "Address.h"
#include "OneWire.h"

class Device
{


public:
    float slope = 1;
    float offset = 0;
    Address address;

    Device() {

    }

    Device(const Address& address) : address(address) {

    }

    Device& operator=(const Device& device) = default;

    void StartMeasurement(OneWire& ds) const
    {
        ds.reset();
        ds.select(address.data());
        ds.write(0x10); // Start conversion
        delay(10);
    }

    int32_t ReadRawMeasurement(OneWire& ds) const
    {
        ds.reset();
        ds.select(address.data());
        ds.write(0x11); // Read value

        int32_t value = 0;
        value |= ds.read();
        value |= ds.read() << 8;
        value |= ds.read() << 16;
        value |= ds.read() << 24;
        return value;
    }

    float ApplyScaling(int32_t raw) const
    {
        return raw * slope + offset;
    }

    int32_t MeasureRaw(OneWire& ds) const
    {
        StartMeasurement(ds);
        delay(1000);
        return ReadRawMeasurement(ds);
    }



    void WaitForStable(OneWire& ds, int32_t margin = 300) const
    {
        int32_t prev = MeasureRaw(ds);
        int32_t actual = MeasureRaw(ds);
        int32_t change = abs(actual - prev);

        while (change > 300)
        {
            prev = actual;
            actual = MeasureRaw(ds);
            change = abs(actual - prev);
        } 
    }

};
