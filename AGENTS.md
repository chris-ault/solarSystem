# AGENTS.md

Solar power monitor/controller: custom firmware on two microcontrollers plus a
BeagleBone Black webserver. Hardware-in-the-loop project; no CI or build system
in this repo.

## Layout

- `i2c/workingWemosMaster/` — ESP8266 (Wemos) firmware. I2C bus master; talks to
  the BBB webserver over WiFi. `loginCred.h` holds WiFi credentials and is
  gitignored — never commit it.
- `i2c/WorkingMapleSlave/` — STM32F103 (Maple Mini) firmware. I2C slave; reads
  battery/panel voltages over 12-bit ADC.
- `EPsolar_Data_comms/` — Modbus-over-serial sketch for the EPsolar charge
  controller.
- `MapleMiniVoltagesPercents/` — standalone Maple Mini ADC test sketch.
- `pythonDash/` — BBB webserver. Everything is gitignored except `weather.py`;
  the rest lives on the device.

## Constraints

- Maple Mini (slave) and Wemos (master) share a 2-wire I2C link; the shared
  buffer is 128 bytes. Changing the protocol or buffer size requires updating
  both firmwares.
- Arduino sketches (.ino) are built/uploaded with the Arduino IDE or
  arduino-cli; no Makefile here.
- `.entire/` and `.pi/` are agent tooling config — keep them tracked, and keep
  `.entire/` honoring its own `.gitignore` (local logs/settings stay out).
