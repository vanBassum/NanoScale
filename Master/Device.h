#pragma once
#include "Address.h"
#include "OneWire.h"

class Device : public Address
{
    float slope = 0;
    float offset = 0;
public:

    Device() : Address(Address::Empty()) {

    }

    Device(const Address& address) : Address(address) {

    }

    Device& operator=(const Device& other) {
        if (this != &other) {
            memcpy(address, other.address, addressSize);
            slope = other.slope;
            offset = other.offset;
        }
        return *this;
    }

    void StartMeasurement(OneWire& ds)
    {
        ds.reset();
        ds.select(address);
        ds.write(0x10); // Start conversion
        delay(10);
    }

    void SetSlope(float slope)
    {
        this->slope = slope;
    }

    void SetOffset(float offset)
    {
        this->offset = offset;
    }

    int32_t ReadRawMeasurement(OneWire& ds)
    {
        ds.reset();
        ds.select(address);
        ds.write(0x11); // Read value

        int32_t value = 0;
        value |= ds.read();
        value |= ds.read() << 8;
        value |= ds.read() << 16;
        value |= ds.read() << 24;
        return value;
    }

    int32_t MeasureRaw(OneWire& ds)
    {
        StartMeasurement(ds);
        delay(1000);
        return ReadRawMeasurement(ds);
    }

    float MeasureWeight(OneWire& ds)
    {
        return MeasureRaw(ds) * slope + offset;
    }
    
    void WaitForStable(OneWire& ds, int32_t margin = 300)
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
