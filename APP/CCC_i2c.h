#ifndef __CCC_I2C_H
#define __CCC_I2C_H

void CCC_I2C_Init(void);
void CCC_I2C_WriteReg(uint8_t WriteAdd, uint8_t WriteData);
uint8_t CCC_I2C_ReadReg(uint8_t ReadAdd);

#endif
