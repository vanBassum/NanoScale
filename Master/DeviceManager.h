#pragma once
#include "Device.h"
#include <functional>


class DeviceManager
{
public:
    std::function<void(Device&)> onConnect;
    std::function<void(Device&)> onDisconnect;

private:
    constexpr static const size_t maxDevices = 10;
    Device devices[maxDevices];

    size_t FindAll(OneWire& ds, Address* addresses, size_t size)
    {
        size_t found = 0;
        ds.reset_search();
        uint8_t addr[8];
        while (ds.search(addr))
        {
            addresses[found] = Address(addr);
            found++;
        } 
        return found;
    }

    int findInList(const Address& address, const Address* addressList, size_t size)
    {
        for(int i = 0; i < size; i++)
        {
            if(addressList[i] == address)
                return i;
        }
        return -1;
    }

    int findInList(const Address& address, const Device* deviceList, size_t size)
    {
        for(int i = 0; i < size; i++)
        {
            if(deviceList[i].address == address)
                return i;
        }
        return -1;
    }

    void CheckForNewDevices(Address* discovered, size_t size)
    {
        for(int i = 0; i < size; i++)
        {
            int index = findInList(discovered[i], devices, maxDevices);
            if(index == -1)
            {
                size_t emptyIndex = findInList(Address::empty, devices, maxDevices);
                if(emptyIndex == -1)
                {
                    Serial.println("No space for device");
                    return;
                }
                devices[emptyIndex] = Device(discovered[i]);
                if(onConnect)
                    onConnect(devices[emptyIndex]);

            }
        }
    }

    void CheckForLostDevices(Address* discovered, size_t size)
    {
        for(int i = 0; i < maxDevices; i++)
        {
            if(devices[i].address == Address::empty)
                continue;

            int index = findInList(devices[i].address, discovered, size);
            if(index != -1)
                continue;

            if(onDisconnect)
                onDisconnect(devices[i]);

            devices[i] = Device(Address::empty);
        }
    }

public:

    DeviceManager()
    {
        for(int i = 0; i < maxDevices; i++)
        {
            devices[i] = Device(Address::empty);
        }
    }

    void Discover(OneWire& ds)
    {
        Address discovered[maxDevices];
        size_t found = FindAll(ds, discovered, maxDevices);
        CheckForNewDevices(discovered, found);
        CheckForLostDevices(discovered, found);
    }

    void VisitDevices(std::function<void(Device&)> visitor)
    {
        for(int i = 0; i < maxDevices; i++)
        {
            if(devices[i].address == Address::empty)
                continue;
            visitor(devices[i]);
        }
    }
};
