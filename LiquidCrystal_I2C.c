#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#include "delay.h"
#include "LiquidCrystal_I2C.h"
#include "I2C.h"


#define I2Cx I2C1

//YWROBOT
//last updated on 21/12/2011
//Tim Starling Fix the reset bug (Thanks Tim)
//wiki doc http://www.dfrobot.com/wiki/index.php?title=I2C/TWI_LCD1602_Module_(SKU:_DFR0063)
//Support Forum: http://www.dfrobot.com/forum/
//Compatible with the Arduino IDE 1.0
//Library version:1.1


LiquidCrystal_I2C_Def lcdi2c;


void LCDI2C_write(uint8_t value){
	LCDI2C_send(value, Rs);
}


void LCDI2C_init(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows)
{
	//GPIO_SetBits(GPIOA,GPIO_Pin_9);
  lcdi2c.Addr = lcd_Addr;
  lcdi2c.cols = lcd_cols;
  lcdi2c.rows = lcd_rows;
  lcdi2c.backlightval = LCD_BACKLIGHT;
	lcdi2c.displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  lcdi2c.displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	lcdi2c.displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	init_I2C(); 

  LCDI2C_begin(lcd_cols, lcd_rows);
}



void LCDI2C_begin(uint8_t cols, uint8_t lines) {
	if (lines > 1) {
		lcdi2c.displayfunction |= LCD_2LINE;
	}
	lcdi2c.numlines = lines;

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	DelayMs(51);

	// Now we pull both RS and R/W low to begin commands
	LCDI2C_expanderWrite(lcdi2c.backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	GPIO_SetBits(GPIOA,GPIO_Pin_9);
	DelayMs(5);

  	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	  // we start in 8bit mode, try to set 4 bit mode
   LCDI2C_write4bits(0x03 << 4);
   DelayMs(5); // wait min 4.1ms

   // second try
   LCDI2C_write4bits(0x03 << 4);
   DelayMs(5); // wait min 4.1ms

   // third go!
   LCDI2C_write4bits(0x03 << 4);
   DelayMs(5);

   // finally, set to 4-bit interface
   LCDI2C_write4bits(0x02 << 4);


	// set # lines, font size, etc.
	LCDI2C_command(LCD_FUNCTIONSET | lcdi2c.displayfunction);

	// turn the display on with no cursor or blinking default
	lcdi2c.displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	LCDI2C_display();

	// clear it off
	LCDI2C_clear();

	// Initialize to default text direction (for roman languages)
	lcdi2c.displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);

	LCDI2C_home();

}

/********** high level commands, for the user! */
void LCDI2C_clear(void){
	LCDI2C_command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	DelayUs(1800);  // 1.57ms
}

void LCDI2C_home(void){
	LCDI2C_command(LCD_RETURNHOME);  // set cursor position to zero
	DelayUs(1800);  // 1.57ms
}

void LCDI2C_setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > lcdi2c.numlines ) {
		row = lcdi2c.numlines-1;    // we count rows starting w/0
	}
	LCDI2C_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LCDI2C_noDisplay(void) {
	lcdi2c.displaycontrol &= ~LCD_DISPLAYON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

void LCDI2C_display(void) {
	lcdi2c.displaycontrol |= LCD_DISPLAYON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// Turns the underline cursor on/off
void LCDI2C_noCursor(void) {
	lcdi2c.displaycontrol &= ~LCD_CURSORON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}
void LCDI2C_cursor(void) {
	lcdi2c.displaycontrol |= LCD_CURSORON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// Turn on and off the blinking cursor
void LCDI2C_noBlink(void) {
	lcdi2c.displaycontrol &= ~LCD_BLINKON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

void LCDI2C_blink(void) {
	lcdi2c.displaycontrol |= LCD_BLINKON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCDI2C_scrollDisplayLeft(void) {
	LCDI2C_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCDI2C_scrollDisplayRight(void) {
	LCDI2C_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCDI2C_leftToRight(void) {
	lcdi2c.displaymode |= LCD_ENTRYLEFT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This is for text that flows Right to Left
void LCDI2C_rightToLeft(void) {
	lcdi2c.displaymode &= ~LCD_ENTRYLEFT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This will 'right justify' text from the cursor
void LCDI2C_autoscroll(void) {
	lcdi2c.displaymode |= LCD_ENTRYSHIFTINCREMENT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This will 'left justify' text from the cursor
void LCDI2C_noAutoscroll(void) {
	lcdi2c.displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCDI2C_createChar(uint8_t location, uint8_t charmap[]) {
		int i;
	location &= 0x7; // we only have 8 locations 0-7
	LCDI2C_command(LCD_SETCGRAMADDR | (location << 3));

	for (i=0; i<8; i++) {
		LCDI2C_write(charmap[i]);
	}
}

// Turn the (optional) backlight off/on
void LCDI2C_noBacklight(void) {
	lcdi2c.backlightval=LCD_NOBACKLIGHT;
	LCDI2C_expanderWrite(0);
}

void LCDI2C_backlight(void) {
	lcdi2c.backlightval=LCD_BACKLIGHT;
	LCDI2C_expanderWrite(0);
}



/*********** mid level commands, for sending data/cmds */

void LCDI2C_command(uint8_t value) {
	LCDI2C_send(value, 0);
}


/************ low level data pushing commands **********/

// write either command or data
void LCDI2C_send(uint8_t value, uint8_t mode) {
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
  LCDI2C_write4bits((highnib)|mode);
	LCDI2C_write4bits((lownib)|mode);
}

void LCDI2C_write4bits(uint8_t value) {
	LCDI2C_expanderWrite(value);
	LCDI2C_pulseEnable(value);
}

void LCDI2C_expanderWrite(uint8_t _data){
	I2C_StartTransmission (I2C1, I2C_Direction_Transmitter, lcdi2c.Addr); 
	I2C_WriteData(I2C1, (int)(_data) | lcdi2c.backlightval); 

	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF));
}

void LCDI2C_pulseEnable(uint8_t _data){
	LCDI2C_expanderWrite(_data | En);	// En high
	DelayUs(1);		// enable pulse must be >450ns

	LCDI2C_expanderWrite(_data & ~En);	// En low
	DelayUs(40);		// commands need > 37us to settle
}



void LCDI2C_write_String(char* str) {
  uint8_t i=0;
  while(str[i])
  {
    LCDI2C_write(str[i]);
    i++;
  }
}


