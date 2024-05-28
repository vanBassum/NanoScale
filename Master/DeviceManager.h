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

    int findInList(const Address& address, const Address* addressList, size_t size)
    {
        for(int i = 0; i < size; i++)
        {
            if(addressList[i] == address)
                return i;
        }
        return -1;
    }

    int findEmtpy()
    {
        for(int i = 0; i < maxDevices; i++)
        {
            if(devices[i] == Address::Empty())
                return i;
        }
        return -1;
    }

    void HandleFound(const Address& address)
    {
        int empty = findEmtpy();
        if(empty == -1)
            return;
        
        devices[empty] = Device(address);
        if(onConnect)
            onConnect(devices[empty]);
    }

    void HandleLost(const Address& address)
    {
        int index = findInList(address, devices, maxDevices);
        if(index == -1)
            return;

        devices[index] = Device(Address::Empty());
        
        if(onDisconnect)
            onDisconnect(devices[index]);
    }

public:

    DeviceManager()
    {
        for(int i = 0; i < maxDevices; i++)
        {
            devices[i] = Device(Address::Empty());
        }
    }

    void Discover(OneWire& ds)
    {
        Address discovered[maxDevices];
        size_t found = 0;
        ds.reset_search();
        uint8_t addr[8];
        while (ds.search(addr))
        {
            discovered[found] = Address(addr);
            found++;
        } 

        // Check for new devices
        for(int i = 0; i < found; i++)
        {
            int index = findInList(discovered[i], devices, maxDevices);
            if(index == -1)
                HandleFound(discovered[i]);
        }

        // Check for lost devices
        for(int i = 0; i < maxDevices; i++)
        {
            if(devices[i] == Address::Empty())
                continue;

            int index = findInList(devices[i], discovered, found);
            if(index == -1)
                HandleLost(devices[i]);
        }
    }

    void VisitDevices(std::function<void(Device&)> visitor)
    {
        for(int i = 0; i < maxDevices; i++)
        {
            if(devices[i] == Address::Empty())
                continue;
            visitor(devices[i]);
        }
    }
};
