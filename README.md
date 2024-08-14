# Project Overview
Bilibili:https://www.bilibili.com/video/BV1m7bMevEQW/?vd_source=c7c9f8b655a74cd44e121d2e52ed5ba5

# Getting started

Preparations
---
To build it you need 2 ESP32 dev models, 2 ILI9341 240*320 screens, 2 buttons,and multiple wires. (Breadboards are recommended)

Caution:The version of ESP32 SDK for Arduino `must be 2.0.x`,such as 2.0.17, 3.x will `cause error` when compiling.

Configuration of TFT_eSPI
---
Edit 'User_setup.h', the default directory of the file is C:\\Users\\'Your username'\\Documents\\Arduino\\libraries\\TFT_eSPI\\User_setup.h

Assembly
---
Connect the parts as the following:
![assembly](https://github.com/user-attachments/assets/04051e12-6a90-44eb-b16c-9f83270e921d)

Or you can follow the code

```xml
/*Connections
  T_IRQ->16
  T_DO->D19
  T_DIN->D23
  T_CS->D5
  T_CLK->D18
  MISO->No connection
  LED->3V3
  SCK->D18
  MOSI->D23
  DC->D2
  RESET->D4
  CS->D15
*/
```

Coding
---
1. Download the code and open it using arduino IDE.

2. Replace the following part with THE OTHER BOARD's mac address
```xml
    if your mac address is 1C:69:20:1C:18:98
```
Then the code should be something like this:
```xml
    const uint8_t PROGMEM broadcastAddress[] = {0x1C,0x69,0x20,0x1C,0x18,0x98};
```
Note:If you do not know the mac address of your boards, you can obtain it by scanning the QR code on your ESP32 board.

3. Write the program into both boards.

How to use
---
1. Boot up, calibrate the screen following the instructions: try to touch the edges of the display untill the min coordinations of X/Y doesn't change anymore,then press the button once. Do the same thing untill the max coordinations of X/Y doesn't change anymore. When done, press the button once again.

2. Now you are free to draw. In order to clear the screen, press the button once.

# Troubleshooting
