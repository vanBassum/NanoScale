#pragma once
#include "Device.h"
#include "OneWire.h"

int32_t abs(int32_t value)
{
    if(value < 0)
        return -value;
    return value;
}

class Calibrator
{
    OneWire& ds;

    int32_t applyWeightAndStabilize(Device& device, int32_t weight, int32_t prevValue, int32_t changeThreshold)
    {
        Serial.print("Apply ");
        Serial.print(weight);
        Serial.println(" gram");
        while(abs(device.MeasureRaw(ds) - prevValue) < changeThreshold);   // Wait for weight to change
        Serial.println("Waiting for stable");
        device.WaitForStable(ds);
        return device.MeasureRaw(ds);
    }

public:
    Calibrator(OneWire& ds) : ds(ds) {}

    void Calibrate(Device& device)
    {
        Serial.print("Calibrating ");
        device.address.Print();
        Serial.println();

        Serial.println("Waiting for stable");
        device.WaitForStable(ds);
        int32_t value_init = device.MeasureRaw(ds);

        int32_t y1 = 4000;
        int32_t x1 = applyWeightAndStabilize(device, y1, value_init, 10000);
        int32_t y2 = 0;
        int32_t x2 = applyWeightAndStabilize(device, y2, x1, 10000);

        float dx = x1 - x2;
        float dy = y1 - y2;
        float slope = dy / dx;
        float offset = y1 - slope * x1;

        device.slope = slope;
        device.offset = offset;

        Serial.print("x1 = ");
        Serial.println(x1);
        Serial.print("x2 = ");
        Serial.println(x2);
        Serial.print("y1 = ");
        Serial.println(y1);
        Serial.print("y2 = ");
        Serial.println(y2);
        Serial.print("dx = ");
        Serial.println(dx);
        Serial.print("dy = ");
        Serial.println(dy);
        Serial.print("Slope calculated = ");
        Serial.println(slope);
        Serial.print("Offset calculated = ");
        Serial.println(offset);
    }
};
