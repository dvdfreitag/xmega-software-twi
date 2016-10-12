#include <avr/io.h>
#include <avr/interrupt.h>

#include "SoftwareTWI.h"

// This bit-bang works because SDA/SCL are pulled high. At startup, OUT
//   and DIR default to 0x00. This means all GPIOs are inputs and are
//   hi-Z. Thus the logic state at startup is SDA = 1, SCL = 1. By setting
//   the DIR bits, SDA/SCL pins are set to outputs with OUT = 0
//   meaning they are actively pulled low as you'd expect. This means that the
//   state of the bus can be manipulated simply by changing the direction bits.

PORT_t *TWI;
uint8_t SDA;
uint8_t SCL;

#define NOP asm volatile("nop");
// Quarter-bit delay
void qdelay(void) { NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP }
// Half-bit delay
void hdelay(void) { qdelay(); qdelay(); }

//     ________
// SDA         |_____
//     ___________
// SCL            |__
void STWI_Start(PORT_t *twi, uint8_t sda, uint8_t scl)
{	// Prepare for a transaction
	TWI = twi;
	SDA = sda;
	SCL = scl;
	// Force SDA and SCL to inputs and set them low
	TWI->DIRCLR = SDA | SCL;
	TWI->OUTCLR = SDA | SCL;
	// Initial delay
	hdelay();
	// Generate START condition: SDA goes low followed by SCL a 
	//   half-bit later
	TWI->DIRSET = SDA;	// Set SDA low
	hdelay();
	TWI->DIRSET = SCL;	// Set SCL low
}

//        _____
// SDA __/     |____
//            ___
// SCL ______|   |__
void STWI_Restart(void)
{	// Generate START/RESTART condition: While SDA is high send SCL
	//   high. After a quarter-bit delay send SDA low, then SCL
	//   after another quarter-bit delay.
	TWI->DIRCLR = SDA; // Ensure SDA is set high
	hdelay();
	TWI->DIRCLR = SCL; // Begin SR by sending SCL high
	qdelay();
	TWI->DIRSET = SDA; // Send SDA low
	qdelay();
	TWI->DIRSET = SCL; // Send SCL low
	hdelay();
}

//     __       __
// SDA   \_____|
//            ____
// SCL ______|   
void STWI_Stop(void)
{	// Generate a STOP condition: While SDA is low, send SCL high
	//   followed by SDA a quarter-bit later.
	TWI->DIRSET = SDA; // Ensure SDA is set low
	hdelay();
	TWI->DIRCLR = SCL; // Set SCL high
	qdelay();
	TWI->DIRCLR = SDA; // Set SDA high
	hdelay();
}

uint8_t STWI_WriteByte(uint8_t data)
{
	for (int8_t i = 7; i >= 0; i--)
	{// Write the data bit
		if (data & (1 << i))
		{	// If the bit is a 1,
			TWI->DIRCLR = SDA; // Set SDA high
		}
		else
		{	// Otherwise
			TWI->DIRSET = SDA;	// Set SDA low
		}
		// One clock transition
		qdelay();
		TWI->DIRCLR = SCL;	// Set SCL high
		hdelay();
		TWI->DIRSET = SCL;	// Set SCL low
	}
	// Ensure SDA is "high" -> SDA is an input.
	// This is where the slave will indicate an ACK by actively pulling SDA down,
	//   or a NACK is indicated by no response.
	TWI->DIRCLR = SDA;	// Set SDA high
	hdelay();
	TWI->DIRCLR = SCL;	// Set SCL high
	data = TWI->IN & SDA; // Get ACK bit
	hdelay();
	TWI->DIRSET = SCL;	// Set SCL low
	hdelay();

	return (data != 0); // Return (N)ACK status
}

uint8_t STWI_WriteBytes(uint8_t *data, uint8_t length)
{	// If the passed buffer is null, don't do anything
	if (!data) return 0;
	// Keep track of the number of bytes written
	uint8_t count = 0;
	// Transmit data
	for (; count < length; count++)
	{	// Let the caller know we got NACK'ed by returning count 
		//   that is less-than length.
		if (STWI_WriteByte(data[count]) != STWI_ACK) return count;
	}
	// Return the number of bytes written
	return count;
}

uint8_t STWI_Read(uint8_t nack)
{
	uint8_t c = 0;
	// Ensure SDA is "high" -> SDA is an input.
	TWI->DIRCLR = SDA;	// Set SDA high

	for (int8_t i = 7; i >= 0; i--)
	{
		hdelay();
		TWI->DIRCLR = SCL;		// Set SCL high
		c <<= 1;				// Shift bit
		c |= TWI->IN & SDA;	// Read bit
		hdelay();
		TWI->DIRSET = SCL;		// Set SCL low
	}
	// Set SDA low for ACK, otherwise leave SDA
	if (!nack) TWI->DIRSET = SDA;
	//   high to NACK
	hdelay();
	TWI->DIRCLR = SCL;	// Set SCL high
	hdelay();
	TWI->DIRSET = SCL;	// Set SCL low
	// Ensure SDA is high
	TWI->DIRCLR = SDA;	// Set SDA high

	return c;
}

int8_t STWI_ReadBytes(uint8_t *data, uint8_t length)
{	// If buffer is null don't do anything
	if (!data) return -1;
	// Offset length
	length--;
	// Write bytes
	for (uint8_t i = 0; i < length; i++)
	{
		data[i] = STWI_Read(STWI_ACK);
	}
	// Read last byte with NACK
	data[length] = STWI_Read(STWI_NACK);
	// Return success
	return 0;
}
