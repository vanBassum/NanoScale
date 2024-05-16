#pragma once
#include "Framing.h"

class NewLineFraming : public IFraming
{
    constexpr const static char EOF_CHARACTER = '\n';
    constexpr const static char ARG_SEPARATOR = ' ';
    Stream& dataStream;
public:
    NewLineFraming(Stream& dataStream);

    IArgumentWriter& WriteFrame() override;
    IArgumentReader& ReadFrame() override;

    class FrameWriter : public IArgumentWriter
    {
        bool endOfFrameSent = false;
        NewLineFraming& framing;
    public:
        FrameWriter(NewLineFraming& framing);
        ~FrameWriter();

        IArgumentWriter& WriteFloat(float value) override;
        IArgumentWriter& WriteInt(int value) override;
        IArgumentWriter& WriteString(const char* str) override;
        void EndFrame() override;
    };

    class FrameReader : public IArgumentReader
    {
        NewLineFraming& framing;
        bool frameEnded = false;

    public:
        FrameReader(NewLineFraming& framing);

        IArgumentReader& ReadFloat(float& value) override;
        IArgumentReader& ReadInt(int& value) override;
        IArgumentReader& ReadString(char* buffer, size_t size) override;
        bool IsFrameEnded() override;
    };
};



NewLineFraming::NewLineFraming(Stream& dataStream) : dataStream(dataStream) {}

NewLineFraming::FrameWriter::FrameWriter(NewLineFraming& framing) : framing(framing) {}

NewLineFraming::FrameWriter::~FrameWriter()
{
    if (!endOfFrameSent)
    {
        framing.dataStream.print('\n');
    }
}

IArgumentWriter& NewLineFraming::FrameWriter::WriteFloat(float value)
{
    framing.dataStream.print(value, 2);
    framing.dataStream.print(' ');
    return *this;
}

IArgumentWriter& NewLineFraming::FrameWriter::WriteInt(int value)
{
    framing.dataStream.print(value);
    framing.dataStream.print(' ');
    return *this;
}

inline IArgumentWriter & NewLineFraming::FrameWriter::WriteString(const char * str)
{
    framing.dataStream.print(str);
    framing.dataStream.print(' ');
    return *this;
}

void NewLineFraming::FrameWriter::EndFrame()
{
    framing.dataStream.print('\n');
    endOfFrameSent = true;
}

NewLineFraming::FrameReader::FrameReader(NewLineFraming& framing) : framing(framing) {}

IArgumentReader& NewLineFraming::FrameReader::ReadFloat(float& value)
{
    if(frameEnded)
        return *this;

    char buffer[32];
    size_t i = 0;
    for(; i < sizeof(buffer); i++)
    {
        while (!framing.dataStream.available());
        char c = framing.dataStream.read();
        frameEnded = c == '\n';
        if(c == ' ' || c == '\n')
            break;
        buffer[i] = c;
    }
    buffer[i] = '\0';
    value = atof(buffer);
    return *this; 
}

IArgumentReader& NewLineFraming::FrameReader::ReadInt(int& value)
{
    if(frameEnded)
        return *this;

    char buffer[32];
    size_t i = 0;
    for(; i < sizeof(buffer); i++)
    {
        while (!framing.dataStream.available());
        char c = framing.dataStream.read();
        frameEnded = c == '\n';
        if(c == ' ' || c == '\n')
            break;
        buffer[i] = c;
    }
    buffer[i] = '\0';
    value = atoi(buffer);
    return *this;
}

IArgumentReader& NewLineFraming::FrameReader::ReadString(char* buffer, size_t size)
{
    if (size == 0)
        return *this;

    if(frameEnded)
    {
        buffer[0] = '\0';
        return *this;
    }

    size_t i = 0;
    for(; i < size - 1; i++)
    {
        while (!framing.dataStream.available());
        char c = framing.dataStream.read();
        frameEnded = c == '\n';
        if(c == ' ' || c == '\n')
            break;
        buffer[i] = c;
    }
    buffer[i] = '\0';
    return *this;
}

bool NewLineFraming::FrameReader::IsFrameEnded()
{
    return frameEnded;
}

IArgumentWriter& NewLineFraming::WriteFrame()
{
    return *new FrameWriter(*this);
}

IArgumentReader& NewLineFraming::ReadFrame()
{
    while (!dataStream.available());
    return *new FrameReader(*this);
}
