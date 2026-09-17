# AGENTS.md

Solar power monitor/controller: custom firmware on two microcontrollers plus a
BeagleBone Black webserver. Hardware-in-the-loop project; no CI or build system
in this repo.

## Data flow

EPsolar charge controller --RS485/Modbus--> Wemos (ESP8266) --WiFi, HTTP POST
JSON--> BBB Flask server (sqlite history). The Wemos also polls the Maple Mini
over I2C (slave address 17) for battery-bank voltages; the Maple drives the
load-control relays (inverter, air conditioner).

## Layout

- `i2c/workingWemosMaster/` — ESP8266 (Wemos) firmware. I2C bus master; talks to
  the BBB webserver over WiFi. `loginCred.h` holds WiFi credentials and is
  gitignored — never commit it.
- `i2c/WorkingMapleSlave/` — STM32F103 (Maple Mini) firmware. I2C slave; reads
  battery/panel voltages over 12-bit ADC.
- `EPsolar_Data_comms/` — Modbus-over-serial code for the EPsolar charge
  controller.
- `MapleMiniVoltagesPercents/` — standalone Maple Mini ADC test sketch.
- `pythonDash/` — BBB webserver. Everything is gitignored except `weather.py`;
  the rest lives on the device.

## Existing functionality

### `i2c/workingWemosMaster/workingWemosMaster.ino` — Wemos hub

- I2C master, slave address 17: `sendRequest()` reads a `SLAVE_DATA` struct,
  `updateConfig()` writes a `SLAVE_CONFIG` (char val[3]).
- Modbus master via `ModbusMaster` (slave ID 1, 115200 baud) to the charge
  controller over RS485. Pulls in `EPsolar_Data_comms/modbus_over_serial.ino`
  for the controller routines (see below).
- `loop()`: `readController()` (Modbus poll, result CSV to Serial), then I2C
  request; 10 s delay between cycles. If `slave_data.value1 == 5` it writes
  config `"v1"` back to the slave.

### `EPsolar_Data_comms/modbus_over_serial.ino` — charge-controller comms

- No longer a standalone sketch: setup()/loop() are commented out; the file is
  `#include`d by the Wemos sketch.
- `updateTime()`: fetches the date/time from the HTTP `Date:` header
  (google.com, fallback time.is) and writes it to the controller RTC via
  holding registers 0x9013.
- `readController()`: reads input registers 0x3100 (PV/battery/load V, A, W;
  controller temp) and 0x311A (battery remaining %, battery temp). Raw
  registers are scaled /100; 32-bit values are `high<<16|low`. Returns an
  8-field CSV: battV, battRemain%, loadA, loadW, pvV, pvA, pvW, contTemp.

### `i2c/WorkingMapleSlave/WorkingMapleSlave.ino` — Maple Mini control

- I2C slave address 17: `requestEvent()` answers with `SLAVE_DATA`,
  `receiveEvent()` accepts a `SLAVE_CONFIG`.
- Requires the STM32F1 Wire library patched: `BUFFER_LENGTH = 128` in
  `WireBase.h` (matches the Wemos 128-byte limit).
- Pulls in `i2c/WorkingMapleSlave/MapleMiniVoltagesPercents/*.ino` for
  `displayVoltages(bank, 'a'|'v'|'p')` over two ADC battery banks.
- Relay pins: `invPreCharge` 19, `invSolenoid` 20, `invPowerPin` 22 (inverter
  remote sleep), `acPin` 21. Relays are ACTIVE LOW (`ON` = LOW).
- Control helpers exist but are NOT called from `loop()` (demo commented out):
  `activateInverter()` runs precharge ~10 s then closes the solenoid;
  `enableAC()`/`activateAC()` cascade (start the inverter first if needed);
  `sleepInverter(bool)` toggles the remote pin, deactivating AC first;
  `deactivateInverter()` shuts AC then solenoid.
- As committed, `loop()` only prints bank voltages/percents every 30 s.

### `pythonDash/` — BBB server

- `basicserver.py` (on device, not tracked): Flask + sqlite `time_db.sqlite`.
  JSON POST ingest: `/postjson` (DS18B20 temps, sensor ID → friendly name via
  `lookupNames`), `/solarPostJSON` (controller fields), `/voltagePOSTJSON`
  (bank1/bank2 volts + percent), `/housePostJSON` (house intake/output temps).
  GET `/BatteryMonitor` returns a recent-readings HTML summary.
- `weather.py` (tracked): Dash dashboard on port 7859; OpenWeatherMap
  current + forecast (city id 4163599) persisted to sqlite
  `sqlalchemy_example.db` via `alchemy_sample.py` (on device, not tracked).

## Known quirks / hazards

- Both i2c sketches `#include` sibling sketches by absolute Windows path
  (`C:/Users/Optiplex 9010/...`); they will not build elsewhere until those
  paths are fixed locally.
- Protocol mismatch: the Wemos `SLAVE_DATA` declares 9 floats but the Maple's
  holds 2×uint16_t, and the Wemos loop reads `slave_data.value1`, which its
  own struct does not define. Re-sync the two structs before trusting the I2C
  path.
- `loginCred.c` is a tracked placeholder with dummy WiFi credentials; real
  credentials go in the gitignored `loginCred.h`.
- Unplug the RS485 adapter from the ESP while flashing — it shares the serial
  port.

## Constraints

- Maple Mini (slave) and Wemos (master) share a 2-wire I2C link; the shared
  buffer is 128 bytes. Changing the protocol or buffer size requires updating
  both firmwares.
- Arduino sketches (.ino) are built/uploaded with the Arduino IDE or
  arduino-cli; no Makefile here.
- `.entire/` and `.pi/` are agent tooling config — keep them tracked, and keep
  `.entire/` honoring its own `.gitignore` (local logs/settings stay out).
