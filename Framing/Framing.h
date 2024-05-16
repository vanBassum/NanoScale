#pragma once;



/// @brief Interface for writing frames
class IArgumentWriter
{
public:
    virtual ~IArgumentWriter() {}
    virtual IArgumentWriter& WriteFloat(float value) = 0;
    virtual IArgumentWriter& WriteInt(int value) = 0;
    virtual IArgumentWriter& WriteString(const char* str) = 0;
    virtual void EndFrame() = 0;
};

/// @brief Interface for reading frames
class IArgumentReader
{
public:
    virtual ~IArgumentReader() {}
    virtual IArgumentReader& ReadFloat(float& value) = 0;
    virtual IArgumentReader& ReadInt(int& value) = 0;
    virtual IArgumentReader& ReadString(char* buffer, size_t size) = 0;
    virtual bool IsFrameEnded() = 0;
};

class IFraming
{
public:
    virtual ~IFraming() {}
    virtual IArgumentWriter& WriteFrame() = 0;
    virtual IArgumentReader& ReadFrame() = 0;
};
