xmega-software-twi
==================

![TWI](https://cloud.githubusercontent.com/assets/4998806/19326633/507ac0da-9099-11e6-9e6f-ea7c7d47e81d.png)

A software two-wire interface (TWI) driver for the Atmel XMega micocontrollers. 

Usage
-----

- **STWI_Start**: Performs initial setup of the SDA/SCL pins and generates a `START` condition
- **STWI_Restart**: Generates a `Sr` (repeated start) condition
- **STWI_Stop**: Generates a `STOP` condition
- **STWI_WriteByte**: Writes the passed byte by manipulating the configured SDA and SCL pins. Returns 0 to indicate a slave `ACK`, and 1 to indicate there was no `ACK`.
- **STWI_WriteBytes**: Writes `length` bytes if the passed buffer is not `NULL`. Returns the number of bytes written. If the return value is less than `length` that means the slave did not `ACK`.
- **STWI_ReadByte**: Reads a byte and returns it to the caller. If `nack` is non-zero, a `NACK` will be generated, otherwise SDA will be pulled low to `ACK` the slave.
- **STWI_ReadBytes**: Reads `length` bytes into the supplied buffer (if the buffer is not `NULL`) and generates a `NACK` to the slave on the last byte.

Precautions
-----------

- It is recommended that you disable interrupts for all TWI transactions with `cli()`/`sei()` to avoid potential issues.
- This library can be alongside XMega TWI hardware modules (eg if you need to bit bang for an MCP4728). Simply clear the `TWI_MASTER_ENABLE` bit in the `TWIx.MASTER.CTRLA` register prior to the transaction, then set it after calling `STWI_Stop()`.
- This library **must** be used with pull-up resistors on the SDA/SCL pins. Luckily the XMega GPIOs are capable of being configured with internal pull-ups. Simply set the "output/pull configuration" bits in `PORTx.PINyCTRL` register to `WIREDANDPULL` to enable the internal pull-up. This will ensure that the SDA/SCL pins are pulled high while they are configured as inputs.
- Bus arbitration is not supported. Please use caution when using this library with multiple masters on a bus.
- Clock stretching is not supported. If a slave attempts to stretch the clock by holding SCL low, the master will completely ignore it and continue on its way.
