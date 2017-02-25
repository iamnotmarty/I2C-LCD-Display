#include "stm32f10x.h"
#include "delay.h"
#include "LiquidCrystal_I2C.h"
#include <stdio.h>

#define I2Cx I2C1
//GPIO_InitTypeDef GPIO_InitStructure;

char buffer[100];

int main()
{
	
		DelayInit();
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
  
	// Init LCD (also inits I2C pins)
	LCDI2C_init(0x27,16,2);


	while(1){

		snprintf(buffer,7,"%f",1000.1);
		LCDI2C_setCursor(0,0);
	

		LCDI2C_write_String(buffer);
		DelayUs(50);
		
	}

}
