# DoAn1
## Project: Smart locker
### Overview
- Use `Esp32` connect to modules include: `LCD` through I2C, `RC522 (RFID)` through SPI, `Fingerprint module AS608` through UART, `Keypad`.
- Make the options to open cabinet.
- Save data such as: `password`, `code RFID card` in the `flash memory`.
### Detail
`MCU` : ESP32 DevKit V1 (ESP-WROOM-32)

`Peripheral`: 
- LCD 16x02
- RC522: RFID Reader
- AS608: Fingerprint module
- Keypad 4x4

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

