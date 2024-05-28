#pragma once


class Address
{
protected:
    constexpr static const size_t addressSize = 8;
    uint8_t address[addressSize];
public:
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

    void Print()
    {
        for(int i = 0; i < addressSize; i++)
        {
            if(i > 0)
                Serial.print(":");
            Serial.print(address[i], HEX);
        }
    }

    static Address Empty()
    {
        return Address();
    }
};
