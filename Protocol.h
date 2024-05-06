#include <Stream.h>

// Forward declaration
class MyProtocolHandler;

// Define a function pointer type for message handlers
typedef void (*MessageHandler)(MyProtocolHandler&, char*, size_t);
typedef void (*Callback)(MyProtocolHandler&);

// Command registration structure
struct Command {
    const char command[4]; // Commands are always 4 characters long
    MessageHandler handler;
};

class MyProtocolHandler {
private:
    Stream& dataStream; // Reference to the stream (e.g., Serial)
    const Command* commandRegister; // Pointer to an array of Command structures
    size_t commandCount; // Number of commands in the commandRegister
    char buffer[64]; // Buffer to store incoming data
    size_t bufferIndex; // Current index in the buffer

    static void HandleOnOverflow(MyProtocolHandler& protocol)
    {
        protocol.Send("ERRR", "Overflow");
    }

    static void HandleOnCommandNotFound(MyProtocolHandler& protocol)
    {
        protocol.Send("ERRR", "Command not found");
    }

public:
    Callback OnOverflow = HandleOnOverflow;
    Callback OnCommandNotFound = HandleOnCommandNotFound;

    // Constructor
    MyProtocolHandler(Stream& stream) : dataStream(stream), commandRegister(nullptr), commandCount(0), bufferIndex(0) {

    }

    // Initialization function
    void Init(const Command* commandList, size_t count) {
        commandRegister = commandList;
        commandCount = count;
    }

    // Send a command and data over the stream
    void Send(const char* command, const char* data) {
        dataStream.print(command);
        dataStream.println(data);
    }

    // Handle incoming data
    void Handle() 
    {
        // Check if data is available in the stream
        if (!dataStream.available())
            return;

        // Read the next character from the stream
        char newCharacter = dataStream.read();

        // Handle end-of-frame character (`\n`)
        if (newCharacter == '\n') {
            if (bufferIndex < sizeof(buffer) - 1) {
                // Process the complete frame (message)
                ProcessFrame();
            } else {
                if(HandleOnOverflow)
                    HandleOnOverflow(*this);
            }
            // Reset the buffer index for the next frame
            bufferIndex = 0;
            return;
        }

        // Ignore carriage return characters (`\r`)
        if (newCharacter == '\r') {
            return;
        }

        // Check if there is space in buffer.
        if (bufferIndex >= sizeof(buffer) - 1)
            return; 

        buffer[bufferIndex++] = newCharacter;
        return;
    }

private:
    // Process a received frame (message)
    void ProcessFrame() {
        buffer[bufferIndex] = '\0'; // Null-terminate the buffer

        // Command should be 4 characters long. 
        if(bufferIndex < 4)
        {
            OnCommandNotFound(*this);
            return;
        }
        
        // Iterate through all commands to find a match
        for (size_t i = 0; i < commandCount; i++) {
            const Command& cmd = commandRegister[i];
            // Compare the received command (first 4 characters of buffer) with registered commands
            if (strncmp(buffer, cmd.command, 4) == 0) {
                // Call the handler function for the matched command
                cmd.handler(*this, buffer + 4, bufferIndex - 4);
                return; // Command found and executed
            }
        }

        if(OnCommandNotFound)
            OnCommandNotFound(*this);
    }
};



