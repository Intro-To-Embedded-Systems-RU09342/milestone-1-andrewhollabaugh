# Milestone 1: Communicating with Will Byers
This program implements a slave device for a individually addressable LED system. The MSP430G2553 processor is supported. An RGB LED (or just 3 normal LEDs) is connected to the board, and is controlled by UART commands. Of the UART command given, the first part is used to configure the three LED brightnesses of this device, then the rest of the command is given to the next node.

## LEDs
The LEDs are currently on GPIO pins P1.6, P2.1, and P2.5. These cannot really be changed, because this program uses hardware PWM. The following table gives more details.

GPIO Pin | Color | Timer | CCR Register
--- | --- | --- | ---
P1.6 | Red | A0 | TA0CCR1
P2.1 | Green | A1 | TA1CCR1
P2.5 | Blue | A1 | TA1CCR2

The LED(s) are connected on a breadboard with resistors and jumpers to the MSP430G2ET dev board.

## UART
UART is set up on the G2553 to use a baud rate of 9600, no parity, 8 data bits and 1 stop bit. Data is expected in the following format in a transmission of n bytes:

- 0th byte: the total number of bytes in the transmission, including this byte and the carriage return
- 1st byte: duty cycle for red LED
- 2nd byte: duty cycle for green LED
- 3rd byte: duty cycle for blue LED
- 4th to (n-1)th bute: duty cycle data for nodes further down the chain; will be transmitted exactly as received
- nth byte: carriage return (0x0D)

The device will transmit data in the same format that it received data. In the new transmission that it will transmit, there will be three fewer bytes, meaning the first byte is the old first byte minus three.

The program relies only on the length data in the 0th byte to parse the received data. If the 0th byte is inaccurate, the program will not work as expected. Carriage returns (0x0D) could be used as duty cycle data, so the program does not detect carriage returns to know when the transmission ends.

