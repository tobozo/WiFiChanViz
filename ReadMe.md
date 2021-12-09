# WiFiChanViz


![](img/screenshot.jpg)


## Motivation

This tool was initially coded to help find the idlest 2.4GHz channel in order to connect a ZigBee device to HomeAssistant.

Some areas can be very busy, I live in a city center and an average of 80 devices on every scan.

Pairing a ZigBee devices in such conditions can prove difficult, especially when the chosen channel is overlapped by too many WiFi devices.



# Installation


Choose one of these installation methods:

- Download and flash the binary from the release assets.

- Get it from the M5Stack AppStore application (A.K.A. M5Stack-SD-Menu downloader)

- Load this project in Arduino IDE, compile and flash

Library dependencies:
  - [LovyanGFX](https://github.com/Lovyan03/LovyanGFX)
  - [ESP32-Chimera-Core](https://github.com/tobozo/ESP32-Chimera-Core)
  - [M5Stack-SD-Updater](https://github.com/tobozo/M5Stack-SD-Updater)



Credits:

  - [@Lovyan03](https://github.com/Lovyan03) for providing the great LGFX library
  - [virtualabs](https://github.com/virtualabs) for inspiring this project (see the [hackwatch](https://github.com/virtualabs/hackwatch))
