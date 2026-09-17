// Shared I2C protocol between the Wemos master (i2c/workingWemosMaster) and
// the Maple Mini slave (i2c/WorkingMapleSlave), slave address 17.
//
// Single source of truth for the wire structs: edit here, then reflash BOTH
// firmwares. Keep sizeof(SLAVE_DATA) well under the 128-byte transfer limit
// (BUFFER_LENGTH patched to 128 in the STM32F1 Wire library's WireBase.h to
// match the Wemos).
//
// Use fixed-width types only: i2cSimpleTransfer copies raw bytes between the
// 32-bit Maple and the ESP8266, so layout must be identical on both sides.
//
// SLAVE_DATA is filled by the slave's requestEvent() and read by the master's
// sendRequest(). value1 is currently a demo counter the slave increments per
// request (the master writes config "v1" back when it sees 5). When real
// sensor fields (bank voltages/percents from displayVoltages()) are wired in,
// extend this struct instead of declaring local copies.

#ifndef SLAVE_PROTOCOL_H
#define SLAVE_PROTOCOL_H

#include <stdint.h>

struct SLAVE_DATA {
    uint16_t value1;
    uint16_t value2;
};

// Master -> slave command, handled by the slave's receiveEvent().
struct SLAVE_CONFIG {
    char val[3];
};

#endif // SLAVE_PROTOCOL_H
