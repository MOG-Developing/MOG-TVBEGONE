# MOG-TVBEGONE

---

### MOG-TVBEGONE is a project for ESP32WROOM32U microcontrollers.

*WARNING: USING IT FOR MALICIOUS PURPOSES MAY RESULT YOU GETTING IN TROUBLE*

---

## STUFF YOU NEED FOR THIS PROJECT: 
- ESP32WROOM32U DevKitC_V4 38-pins
- PCB0100 Transmitter (or similar)
- Female to Female jumper wires
- MicroUSB cable (for flashing and etc.)

## PINOUT:

IR Transmitter     | ESP32-WROOM-32U
-------------------|-----------------
VCC                | 3.3V
GND                | GND
DAT (signal)       | GPIO4

## SETTING IT UP:
- Download ![CP210x driver](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers)
- Download ArduinoIDE ![ARDUINO DOWNLOAD PAGE](https://www.arduino.cc/en/software/)
- Run ArduinoIDE in administrator.
- Go to File->>Preferences.
- In the "Additional Boards Manager URLs" paste this. <pre> https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json </pre> and click OK.
- Then go to Tools->>Board->>Boards Manager and install "esp32" by "esp32 by Espressif Systems"
- Download all of these ![AsyncTCP](https://github.com/me-no-dev/AsyncTCP/archive/refs/heads/master.zip) ![ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer/archive/refs/heads/master.zip) ![ArduinoJson](https://github.com/bblanchon/ArduinoJson/archive/refs/heads/master.zip)
- After you download them do this: Sketch->>Include Library->> Add .ZIP Library->> And add all of those .ZIP files.
- Then you open the .ino file: File->>Open and select the file
- Then you plug your ESP32WROOM32U to your computer. Select "ESP32 Dev Module" for the port.
- Then you press the "Upload" button and hold boot button on the ESP32WROOM32U.

---
## PHOTOS:
