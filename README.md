# solarSystem
WiFi Enabled portable AC powered by solar and lithium in deficit. Wemos Esp8266, Maple Mini, BeagleBone Black, i2c, Python, Dash, Sqlalchemy, Sqlite.

Project Layout
* The Maple Mini and Wemos communicate over a 2 wire i2c interface.
* The Wemos receives orders and requests from the webserver on the BeagleBone Black.
* The BBB hosts the interface for monitoring and controlling the wemos/maple.
* The BBB also hosts the database of history.

## Maple Mini - STM32 - Muscles
https://www.st.com/resource/en/datasheet/stm32f103tb.pdf
* 32-Bit MCU, 72Mhz
* 128kb flash, 20Kb SRAM
* 12-Bit ADC / 3.3V


## Wemos - ESP8266 - Brains & Communication
https://www.espressif.com/sites/default/files/documentation/0a-esp8266ex_datasheet_en.pdf
* 32-Bit MCU, 160Mhz
* 16MBflash, 50kb SRAM
* 2.4Ghz Wifi


### Power Supply - Simple Switcher - Buck regulator
* http://www.ti.com/lit/ds/symlink/lm2596.pdf
* This powers the Wemos and Maple directly from the 24Volts Lithium


## BeagleBone Black
* https://cdn-shop.adafruit.com/datasheets/BBB_SRM.pdf
* 32-Bit MCU, 1Ghz, ARM Cortex A8, RISC, 
* 2GB EMMC Storage, 512MB RAM
* CPU:http://www.ti.com/product/AM3358
