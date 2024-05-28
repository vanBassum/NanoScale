#pragma once
#include <Arduino.h>


class Address
{
public:
    static const Address empty;
    constexpr static const size_t addressSize = 8;
    uint8_t address[addressSize];
    

    Address() {
        memset(address, 0, addressSize);
    }

    Address(const uint8_t address[8]) {
        memcpy(this->address, address, addressSize);
    }

    bool operator==(const Address& other) const {
        return memcmp(address, other.address, addressSize) == 0;
    }

    bool operator!=(const Address& other) const {
        return !(*this == other);
    }


    const uint8_t* data() const
    {
        return address;
    }


    String ToString() const
    {
        String result = "";
        for(int i = 0; i < addressSize; i++)
        {
            if(i > 0)
                result += ":";
            if(address[i] < 16)
                result += "0";
            result += String(address[i], HEX);
        }
        return result;
    }

    
    void Print() const
    {
        Serial.print(ToString());
    }

};

const Address Address::empty = Address();
