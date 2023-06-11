# DoAn1
## Project: Smart locker
### Overview
- Use `Esp32` connect to modules include: `LCD` through I2C, `RC522 (RFID)` through SPI, `Fingerprint module AS608` through UART, `Keypad`.
- Make the options to open cabinet.
- Save data such as: `password`, `code RFID card` in the `flash memory`.
### Detail
`MCU` : ESP32 DevKit V1 (ESP-WROOM-32)

`Peripheral`: 
- [LCD 16x02](https://www.futurlec.com/LED/LCD16X2.shtml)
- [RC522: RFID Reader](https://www.electronicwings.com/esp32/rfid-rc522-interfacing-with-esp32)
- [AS608: Fingerprint module](https://tapit.vn/giao-tiep-cam-bien-nhan-dang-van-tay-as608-phan-1/)
- [Keypad 4x4](https://esp32io.com/tutorials/esp32-keypad)

`IDE` : Arduino IDE

`Features`:
- Unlock with password.
- Unlock with RFID card.
- Unlock with Fingerprint.
- Management function: Master card allows to modify password, add/delete rfid card, fingerprint.

### Codes
`Menu`

To display functions and easy to operate, we create some menus in program, includes:
- Main menu
- Master menu
- Change finger menu
- Change id card menu

<picture>
   <img alt="menus" height="70%" width="70%" src="https://i.imgur.com/2NF0uaO.jpg">
</picture>

### Schematic

<picture>
   <img alt="schematic" height="70%" width="70%" src="https://i.imgur.com/xxChdGJ.png">
</picture>

`3D View`

<picture>
   <img alt="schematic" height="40%" width="40%" src="https://i.imgur.com/R4za0Jv.png">
</picture>

### Result
