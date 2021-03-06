#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <ctype.h>
#include <string.h>

#define PROGMEM   __ATTR_PROGMEM__	//define PROGMEM attribute

//Pictures
//======================================================================
const uint8_t picture1[] PROGMEM = //Pixels
{
	0b11111111, 0b11111111,
	0b11101111, 0b11111111,
	0b11111101, 0b11111011,
	0b11110111, 0b01111011,
	0b10101111, 0b01011110,
	0b11011110, 0b10111100,
	0b10110110, 0b10110110,
	0b10101101, 0b11000110,
	0b10110010, 0b01011011,
	0b10100010, 0b01110110,
	0b10100010, 0b11011000,
	0b10101000, 0b11010000,
	0b01010010, 0b01010000,
	0b01000010, 0b10000010,
	0b00101000, 0b10000000,
	0b01000000, 0b01000000,
	0b01000000, 0b00000000,
	0b00000000, 0b00000000,
};

const uint8_t picture2[128] PROGMEM = //Scarab SRV
{
	0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xaf, 0xff, 
    0xff, 0xf3, 0x8f, 0xff, 
    0xe1, 0xc1, 0x2f, 0xff, 
    0xde, 0xfa, 0xaf, 0xff, 
    0xbd, 0x40, 0x23, 0xff, 
    0xbd, 0x5f, 0x8b, 0xff, 
    0xbd, 0x5f, 0xfb, 0xff, 
    0xb1, 0x55, 0x7b, 0xff, 
    0xd8, 0x00, 0x03, 0xff, 
    0xe3, 0x8b, 0xbf, 0xff, 
    0xfc, 0x71, 0xdf, 0xff, 
    0xf3, 0xee, 0xe3, 0xff, 
    0x8f, 0xea, 0xdd, 0xfc, 
    0x77, 0xea, 0xd5, 0x57, 
    0x57, 0xf1, 0xd4, 0xfe, 
    0x56, 0x87, 0xe3, 0xdb, 
    0x89, 0x6b, 0xc0, 0xff, 
    0x00, 0x00, 0x3e, 0x7f, 
    0xff, 0xff, 0xbb, 0x07, 
    0xd2, 0xad, 0xe3, 0xf0, 
    0xbf, 0x9b, 0xf7, 0xff, 
    0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xfb, 
    0xff, 0xf0, 0xfd, 0x3b, 
    0xf3, 0xff, 0x7f, 0x3b, 
    0xff, 0xdf, 0xff, 0x9b, 
    0x8f, 0xff, 0xfb, 0xe7, 
    0xfb, 0x9f, 0x7f, 0xff, 
    0xff, 0xe9, 0xff, 0xff
};

const uint8_t picture3[32] PROGMEM= //Station
{
	0x80, 0x01, 
    0x7b, 0xc0, 
    0x77, 0xe0, 
    0x6f, 0xf0, 
    0x5f, 0xf8, 
    0x3f, 0xfc, 
    0x78, 0x1e, 
    0x77, 0xee, 
    0x77, 0xee, 
    0x78, 0x1e, 
    0x3f, 0xfc, 
    0x5f, 0xf8, 
    0x6f, 0xf0, 
    0x77, 0xe0, 
    0x7b, 0xc0, 
    0x80, 0x01
};

const uint8_t picture4[72] PROGMEM= //Microcontroller
{
	0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 
    0xfe, 0xaa, 0xbf, 
    0xfe, 0xaa, 0xbf, 
    0xfe, 0xaa, 0xbf, 
    0xfc, 0x00, 0x1f, 
    0xe0, 0x9d, 0x43, 
    0xfd, 0x49, 0xdf, 
    0xe1, 0xc9, 0x43, 
    0xfd, 0x49, 0x5f, 
    0xe0, 0x00, 0x03, 
    0xfc, 0x88, 0x1f, 
    0xe1, 0x54, 0x03, 
    0xfc, 0x9c, 0x1f, 
    0xe1, 0x54, 0x03, 
    0xfc, 0x94, 0x1f, 
    0xe0, 0x00, 0x03, 
    0xfc, 0x00, 0x1f, 
    0xfe, 0xaa, 0xbf, 
    0xfe, 0xaa, 0xbf, 
    0xfe, 0xaa, 0xbf, 
    0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff
};
//======================================================================

volatile uint16_t EepromSize = 512;	//Eeprom size in Bytes

volatile uint16_t EepromPos = 0;	//Variable used to store the current position in Eeprom

void SavePicture(uint8_t *BitMap, uint8_t width, uint8_t height);	//function used to store images in the Eeprom. Height and width of the image are in pixels. Width has to be dividable by 8.

int main(void)
{
	DDRB |= (1<<PB5);	//set Pin B5 as output
    
    //Saving Pictures
    //==================================================================
    SavePicture(&picture2, 32, 32);
    SavePicture(&picture3, 16, 16);
    SavePicture(&picture1, 16, 16);
    SavePicture(&picture4, 24, 24);
    //==================================================================
    
    return 0;
}//end of main

void SavePicture(uint8_t *BitMap, uint8_t width, uint8_t height)
{
	uint16_t MapPos = 0;	//Variable used to store the current position in the bitmap
	if(EepromPos + (((width/8) * height) + 2) <= EepromSize)	//Check if image fits into the leftover space in Eeprom
	{
		PORTB |= (1<<PB5); //Led ON to display Active Process
		eeprom_write_byte((uint8_t*)EepromPos, width);	//Write width of image into Eeprom
		EepromPos++;
		eeprom_write_byte((uint8_t*)EepromPos, height);	//Write height of image into Eeprom
		EepromPos++;
		
		MapPos = 0;
		while(MapPos < ((width / 8) * height))	//Write Bitmap into Eeprom
		{
			eeprom_write_byte((uint8_t*)EepromPos, pgm_read_byte(&BitMap[MapPos]));
			MapPos++;
			EepromPos++;
		}
	}
	else
	{
		uint16_t Counter = 0;	//Used for the delay between on and off of the LED
		uint8_t Active = 0;	//Used to store the current state of the LED
		while(1)	//Led Blinking to display not enought storage space
		{
			Counter++;
			if(Counter == 0)
			{
				Active = !Active;	//Change Active to oposite state when counter overflows
			}
			
			if(Active)	//Set LED to current state of Active
			{
				PORTB |= (1<<PB5);
			}
			else
			{
				PORTB &= ~(1<<PB5);
			}
		}
	}
	PORTB &= ~(1<<PB5);	//Led OFF to display no Active Process
}
