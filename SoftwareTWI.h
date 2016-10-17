#ifndef SOFTWARETWI_H_
#define SOFTWARETWI_H_

#define STWI_ACK	0x00U
#define STWI_NACK	0x01U

void STWI_Start(PORT_t *twi, uint8_t sda, uint8_t scl);
void STWI_Restart(void);
void STWI_Stop(void);
uint8_t STWI_WriteByte(uint8_t data);
uint8_t STWI_WriteBytes(uint8_t *data, uint8_t length);
uint8_t STWI_ReadByte(uint8_t nack);
uint8_t STWI_ReadBytes(uint8_t *data, uint8_t length);

#endif
