# Getting started

Preparations
---
To build it you need 2 ESP32 dev models, 2 ILI9341 240*320 screens, 2 buttons,and multiple wires. (Breadboards are recommended)

Assembly
---
Connect them as the following:

Coding
---
1. Download the code and open it using arduino IDE.

2. Replace the following part with THE OTHER BOARD's mac address
```xml
    const uint8_t PROGMEM broadcastAddress[] = {0xyour,0xmac,0xaddress,0x,0x,0x};
```
It should be something like this:
```xml
    const uint8_t PROGMEM broadcastAddress[] = {0x1C,0x69,0x20,0x1C,0x18,0x98};
```
Note:If you do not know the mac address of your boards, you can obtain it by scanning the QR code on your ESP32 board.

3. Write the program into both boards.

How to use
---
1. Boot up, calibrate the screen following the instructions: try to touch the edges of the display untill the min coordinations of X/Y doesn't change anymore,then press the button once. Do the same thing untill the max coordinations of X/Y doesn't change anymore. When done, press the button once again.
2.Now you are free to draw.In order to clear the screen, press the button once.

# Troubleshooting
