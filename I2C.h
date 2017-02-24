#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "delay.h"
#include "stm32f10x_i2c.h"

void init_I2C1(void);
void I2C_StartTransmission(I2C_TypeDef*, uint8_t,  uint8_t);
void I2C_WriteData(I2C_TypeDef*, uint8_t);
uint8_t I2C_ReadData(I2C_TypeDef*);
