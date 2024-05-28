#pragma once



class AsciiFrameWriter
{
    Stream& _stream;
    char _sepChar;
    char _endChar;
    //char _startChar;
    bool _first = false;
public:
    AsciiFrameWriter(Stream& stream, char sepChar = ' ', char endChar = '\n')
        : _stream(stream), _sepChar(sepChar), _endChar(endChar)
    {
    }

    bool StartFrame()
    {
        //_stream.print(_startChar);
    }

    bool Write(const int& value)
    {
        if(!_first)
            _stream.print(_sepChar);
        _first = false;
        _stream.print(value);
    }

    bool Write(const char* value)
    {
        if(!_first)
            _stream.print(_sepChar);
        _first = false;
        _stream.print(value);
    }

    bool EndFrame()
    {
        _stream.print(_endChar);
    }
};


class AsciiFrameReader
{
    Stream& _stream;
    char _sepChar;
    char _endChar;
    //char _startChar;
    bool _frameStarted = false;
    bool _frameEnded = false;

    size_t readArgumentBlocking(char* buffer, size_t bufferSize)
    {
        if(!FrameStarted())
            return 0;
        if(_frameEnded)
            return 0;
        size_t read = 0;
        while(read < bufferSize-1)
        {
            while (_stream.available() == 0);
            char c = _stream.read();
            if(c == _endChar)
                _frameEnded = true;
            if(c == _sepChar || c == _endChar)
                break;
            buffer[read] = c;
            read++;
        }
        buffer[read] = '\0';
        return read;
    }

public:
    AsciiFrameReader(Stream& stream, char sepChar = ' ', char endChar = '\n')
        : _stream(stream), _sepChar(sepChar), _endChar(endChar)
    {
    }

    bool FrameStarted()
    {
        if(_frameStarted)
            return true;
        _frameStarted = _stream.available() > 0; // _stream.read() == _startChar;
        return _frameStarted;
    }

    bool WaitForStart()
    {
        while(!FrameStarted());
        return true;
    }
    
    bool Read(int& value)
    {
        char buffer[32];
        size_t read = readArgumentBlocking(buffer, 32);
        if(read == 0)
            return false;
        value = atoi(buffer);
        return true;
    }

    bool Read(char* buffer, size_t bufferSize)
    {
        return readArgumentBlocking(buffer, bufferSize) > 0;
    }

    bool ConsumeUntilEnd()
    {
        while(!_frameEnded)
        {
            char buffer[32];
            readArgumentBlocking(buffer, 32);
        }
        return true;
    }
};

