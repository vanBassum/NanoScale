# Simple Weight Measurement

This project demonstrates how to measure weight using an Arduino and an HX711 load cell amplifier. By connecting the HX711 to the Arduino Nano and uploading the provided code, you can measure the weight of an object and interact with the setup using a serial terminal.

## Hardware Requirements

- Arduino Nano
- HX711 load cell amplifier
- Load cell
- Connection wires

## Connections

1. Connect the `LOADCELL_DOUT_PIN` (pin 3) of the HX711 to pin 3 of the Arduino Nano.
2. Connect the `LOADCELL_SCK_PIN` (pin 2) of the HX711 to pin 2 of the Arduino Nano.
3. Connect the load cell to the HX711 as per the manufacturer's instructions.

## Software Setup

1. Clone the repository to your local machine.
2. Open the Arduino IDE.
3. Open the provided `.ino` file.
4. Connect your Arduino Nano to your computer.
5. In the Arduino IDE, select the appropriate board and port under **Tools** > **Board** and **Port**.
6. Upload the code to the Arduino Nano.

## Running the Code

After uploading the code, open the serial terminal in the Arduino IDE. Make sure the baud rate is set to `115200`.

### Commands Supported

You can send the following commands through the serial terminal to interact with the code:

- **Command**: `DRAW`
  - **Description**: Returns the raw measured value from the HX711.
  - **Arguments**: You can specify the number of measurements to average as an argument. The default is `7` if no argument is provided.
  - **Example**: Send `DRAW\n` to get the default average of `7` measurements.
    - Send `DRAW 10\n` to get the average of `10` measurements.

## Note on Error Handling

If an unsupported command is sent or if there is an overflow error during message handling, the code will respond with an error code `ERRR` followed by an error message.

## License

This project is licensed under the MIT License.

