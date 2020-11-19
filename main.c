#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <ctype.h>
#include <string.h>
#include "Font.c"

#define BUTTON (PINC & (1<<PC0))	//define the input Button
#define BUSY !(PIND & (1<<PD3))	//define BUSY
#define PROGMEM   __ATTR_PROGMEM__	//define PROGMEM attribute


const uint8_t DispHeight = 152;	//height of display
const uint8_t DispWidth = 152;	//width of display

volatile uint16_t ISR_zaehler;

ISR (TIMER0_OVF_vect)	//timer used a a delay
{
	TCNT0 = 0;
	ISR_zaehler++;	//Increase ISR_zaehler by 1
}

//LUT Data
//======================================================================
const unsigned char lut_vcom0[] =
{
    0x0E, 0x14, 0x01, 0x0A, 0x06, 0x04, 0x0A, 0x0A,
    0x0F, 0x03, 0x03, 0x0C, 0x06, 0x0A, 0x00
};
 
const unsigned char lut_w[] =
{
    0x0E, 0x14, 0x01, 0x0A, 0x46, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x86, 0x0A, 0x04
};
 
const unsigned char lut_b[] = 
{
    0x0E, 0x14, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x4A, 0x04
};
 
const unsigned char lut_g1[] = 
{
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
};
 
const unsigned char lut_g2[] = 
{
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
};
 
const unsigned char lut_vcom1[] = 
{
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
 
const unsigned char lut_yellow0[] = 
{
    0x83, 0x5D, 0x01, 0x81, 0x48, 0x23, 0x77, 0x77,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
 
const unsigned char lut_yellow1[] = 
{
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};  
//======================================================================


//Function Prototypes
//======================================================================
void Init();	//Display initilisation
void SPI_Com (uint8_t data);	//Send Command to display
void SPI_Data (uint16_t data);	//Send Data to display
void SPI_DataArray (uint8_t data[], uint16_t ArraySize, uint8_t inverted);	//used to send an array of data
void Wait(void);	//Small Delay
void Wait4Idle(void);	//Wait for display to be idle
void FillBW (uint8_t Pat, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t Refresh);		//Set Black and White to specified Pattern
void FillYellow (uint8_t Pat, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t Refresh);		//Set Yellow to specified Pattern
void drawString (char str[], uint8_t yellow, uint8_t inverted, uint8_t NewLineX, uint8_t x, uint8_t y, uint8_t Refresh);	//Draw a String
void drawImage (uint8_t imageNum, uint8_t mainCol, uint8_t backCol, uint8_t x, uint8_t y, uint8_t repeat, uint8_t Refresh);	//Draw a Image	(Col values: 0 Black, 1 Transparent when other not 0 else white, 2 yellow)
void refresh();		//send Refresh command
void refreshPartial(uint8_t x, uint8_t y,  uint8_t width, uint8_t height);	//Refresh designated space on display
void powerOFF();	//Turn off power
void defineWindow(uint8_t x, uint8_t y,  uint8_t width, uint8_t height);	//define a window for partial mode
//======================================================================

volatile uint8_t initialised = 0;	//used check if display has been initalised since last shutdown

void SPI_Com (uint8_t data)
{/* Start transmission */
	
	//Set Pins to Command position
	//==================================================================
	PORTD &= ~(1<<PD0);
	PORTD &= ~(1<<PD1);
	//==================================================================
	SPDR = data;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}

void SPI_Data (uint16_t data)
{/* Start transmission */
	//Set Pins to Data position
	//==================================================================
	PORTD &= ~(1<<PD0);
	PORTD |= (1<<PD1);
	//==================================================================
	SPDR = data;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}

void SPI_DataArray (uint8_t data[], uint16_t ArraySize, uint8_t inverted)
{/* Start transmission */
	//Set Pins to Data position
	//==================================================================
	PORTD &= ~(1<<PD0);
	PORTD |= (1<<PD1);
	//==================================================================
	uint16_t counter;
	for (counter = 0; counter < ArraySize; counter++)
	{
		if(inverted != 0)
		{
			SPI_Data(~data[counter]);	//Send data inverted
		}
		else
		{
			SPI_Data(data[counter]);	//Send data normaly
		}
	}
	/* Wait for transmission complete */
}

void Wait(void)
{
	ISR_zaehler = 0;
	while(ISR_zaehler != 200);	//Wait until ISR_zaehler == 200 then continue with programm
}

void Wait4Idle(void)
{
	while(BUSY);	//Wait until not BUSY
}

void SetLutBw(void) 
{
    unsigned int count;     
    SPI_Com(0x20);
    for(count = 0; count < 15; count++) 
    {
        SPI_Data(lut_vcom0[count]);
    } 
    SPI_Com(0x21);       
    for(count = 0; count < 15; count++) 
    {
        SPI_Data(lut_w[count]);
    } 
    SPI_Com(0x22);         
    for(count = 0; count < 15; count++) 
    {
        SPI_Data(lut_b[count]);
    } 
    SPI_Com(0x23);         
    for(count = 0; count < 15; count++) 
    {
        SPI_Data(lut_g1[count]);
    } 
    SPI_Com(0x24);        
    for(count = 0; count < 15; count++) 
    {
        SPI_Data(lut_g2[count]);
    } 
}

void SetLutY(void) 
{
    unsigned int count;     
    SPI_Com(0x25);
    for(count = 0; count < 15; count++) 
    {
        SPI_Data(lut_vcom1[count]);
    } 
    SPI_Com(0x26);
    for(count = 0; count < 15; count++) 
    {
        SPI_Data(lut_yellow0[count]);
    } 
    SPI_Com(0x27);
    for(count = 0; count < 15; count++) 
    {
        SPI_Data(lut_yellow1[count]);
    } 
}

void Init()
{	
	//Reset Display
	//==================================================================
	PORTD &= ~(1<<PD2);
	Wait();
	PORTD |= (1<<PD2);
	Wait();
	//==================================================================
	
	//Booster Soft Start
	//==================================================================
	SPI_Com(0x06);
	SPI_Data(0x17);
	SPI_Data(0x17);
	SPI_Data(0x17);
	//==================================================================
	
	//Power settings
	//==================================================================
	SPI_Com(0x01);
	SPI_Data(0x03);
	SPI_Data(0x00);
	SPI_Data(0x2b);
	SPI_Data(0x2b);
	SPI_Data(0x09);
	//==================================================================
	
	SPI_Com(0x04); //Power ON
	while(BUSY)
	{
		SPI_Com(0x71);	//Read data from 0x71
	}
	Wait4Idle();
	
	//Panel Setting
	//==================================================================
	SPI_Com(0x00);
	SPI_Data(0xCF);
	//==================================================================
	
	//Vcom and data interval setting
	//==================================================================
	SPI_Com(0x50);
	SPI_Data(0x10);
	//==================================================================
	
	//PLL control
	//==================================================================
	SPI_Com(0x30);
	SPI_Data(0x3a);
	//==================================================================	
	
	//Resolution settings
	//==================================================================
	SPI_Com(0x61);
	SPI_Data(DispWidth);
	SPI_Data(0x00);
	SPI_Data(DispHeight);
	//==================================================================
	
	//VCM_DC setting
	//==================================================================
	SPI_Com(0x82);
	SPI_Data(0x0E);
	//==================================================================
	
	initialised = 1;
	
	SetLutBw();
	SetLutY();
}

int main(void)
{
	// Set MOSI, SCK as Output
    DDRB |= (1<<PB3) | (1<<PB5) | (1<<PB2) | (1<<PB1);
	DDRB &= ~(1<<PB4);	//Input (Miso)
	PORTB |= (1<<PB1);
	PORTB &= ~(1<<PB2);
	//PORTD |= (1<<PD0);//epaper-BUSY; activate pullup
    // Enable SPI, Set as Master
    // Prescaler: Fosc/16, Enable Interrupts
    //The MOSI, SCK pins are as per ATMega8
    // Enable SPI as master, set clock rate fck/2
    //Init SPI		CLK/2
	//==================================================================
	SPCR = (1<<SPE) | (1<<MSTR);
	SPSR |= (1<<SPI2X);
	//==================================================================
 
	//Konfiguration Timer Overflow
	//==================================================================
	TCCR0	= 0x00;
	TCCR0	= 0x04;
	TIMSK	|= (1 << TOIE0);
	TIFR |= (1 << TOV0);
	//==================================================================
    // Enable Global Interrupts
    sei();
	
	DDRD |= (1<<PD0) | (1<<PD1) | (1<<PD2);	//Pin definition CS DS Reset
	PORTD &= ~(1<<PD1);
	PORTD &= ~(1<<PD0);	//set CS to low
	PORTD |= (1<<PD2);	//set Reset to high
	
	DDRC &= ~(1<<PC0);	//Pin definition Input Button
	PORTC |= (1<<PC0);	//activate Pull up
	
	DDRD &= ~(1<<PD3);	//Pin definition Busy Input
	PORTD |= (1<<PD3);	//activate Pull up
	
	Init();
	
	//~ FillBW(0x00);
	//~ FillYellow(0xFF);
	
	Wait4Idle();
	FillBW(0xff, 0, 0, DispWidth/4, DispHeight, 0);
	FillYellow(0x00, 0, 0, DispWidth/4, DispHeight, 0);
	
	drawImage(0, 0, 1, 2, 0, 0, 0);
	drawImage(1, 0, 2, 0, 0, 0, 0);
	drawImage(2, 2, 1, 0, 16, 0, 0);
	drawImage(3, 0, 1, 6, 0, 0, 0);
	SPI_Com(0x92);
	refresh();
	powerOFF();
	while(BUTTON);
	Wait4Idle();
	
	powerOFF();
	//~ while(1)
	//~ {
		//~ Wait4Idle();
		//~ SPI_Com(0x12);
	//~ }
	
	//~ FillYellow(0xFF);
	//~ refresh();
	//~ FillYellow(0x00);
	//~ refresh();
	
	//~ FillYellow(0xFF);
	//~ FillBW(0x00);
	//~ refresh();
	//~ FillYellow(0x00);
	//~ FillBW(0xFF);
	//~ refresh();
	while(1);
}//end of main

void FillBW (uint8_t Pat, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t Refresh)
{
	if (initialised == 0)	//Check if Display has been initialised since last Deep Sleep
	{
		Init();
	}
	uint16_t counter;	//Counter variable
	Wait4Idle();
	defineWindow(x, y, width, height);
	SPI_Com(0x10);	//set to BW data transmission mode
	for (counter = 0; counter < (((width * 8) * height)/8); counter++)
	{
		Wait4Idle();
		SPI_Data(Pat);	//Send Patern
	}
	if (Refresh != 0)	//if refresh is set to 1 refresh window
	{
		refresh();
	}
	SPI_Com(0x92);	//close window
}

void FillYellow (uint8_t Pat, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t Refresh)
{
	if (initialised == 0)	//Check if Display has been initialised since last Deep Sleep
	{
		Init();
	}
	uint16_t counter;	//Counter variable
	Wait4Idle();
	defineWindow(x, y, width, height);
	SPI_Com(0x13);	//set to BW data transmission mode
	for (counter = 0; counter < (((width * 8) * height)/8); counter++)
	{
		Wait4Idle();
		SPI_Data(Pat);	//Send data
	}
	if (Refresh != 0)	//if refresh is set to 1 refresh window
	{
		refresh();
	}
	SPI_Com(0x92);	//close window
}


void drawString (char str[], uint8_t yellow, uint8_t inverted, uint8_t NewLineX, uint8_t x, uint8_t y, uint8_t Refresh)
{
	if (initialised == 0)	//Check if Display has been initialised since last Deep Sleep
	{
		Init();
	}
	uint8_t LetterBuff[15];	//Buffer to store Data for drawing letter
	uint8_t counter = 0;	//counter variable
	uint8_t Xcount = 0;		//counter variable for x axis
	uint8_t Ycount = 0;		//counter variable for y axis
	uint8_t Xmax = 0;
	
	if (inverted >= 1)	//check if inverted is true
	{
		inverted = 0xff;	//set inverted to 0xFF so that it can be easily inverted with the not operator
	}
	
	//Sending letter data
	//==================================================================
	for (Ycount = y; Ycount < 10; Ycount++)
	{
		for (Xcount = x; Xcount < 19; Xcount++)
		{	
			if(counter < strlen(str))	//Check if the entire String has been proccessed
			{
				if(str[counter] == '\n')	//Check for a Newline
				{
					Xcount = NewLineX;	//Set X position to the designated Newline startposition
					Ycount++;	//go to next line
					counter++;	//go to next char in String
				}
				
				if(Xmax < Xcount)	//check if the current Xposition is greater than the maximum X position that has been reached so far
				{
					Xmax = Xcount;	//set X max to current x position
				}

				SPI_Com(0x92);	//close current window
				defineWindow(Xcount, Ycount * 15, 1, 15);	//Define new Window for Letter
				if (yellow == 0)	//if yellow is set to zero go to BW data transmission mode
				{
					SPI_Com(0x10);
				}
				else 	//if yellow is not set to zero go to Yellow data transmission mode
				{
					SPI_Com(0x13);
				}
				Wait4Idle();
				memcpy_P(LetterBuff, &Font[str[counter]-32], 15);	//Read letter data from Font into buffer
				if(yellow == 0)	//check if data has to be sent inverted or not
				{
					SPI_DataArray(LetterBuff, 15, inverted);	//send data from buffer
				}
				else
				{
					SPI_DataArray(LetterBuff, 15, ~inverted);	//send data from buffer
				}
				counter++;
			}
			else
			{
				break;
			}
		}
		if(counter >= strlen(str))	//Go out of for Loop when entire string has been proccessed
		{
			break;
		}
	}
	//==================================================================
	if (Refresh != 0)	//Check if rdisplay needs to be refreshed
	{
		if (Ycount != y)	//check if more than one line has been written
		{
			refreshPartial(NewLineX, y, (Xmax * 2) - NewLineX, (y + ((Ycount+1) * 15)));	//refresh in partial window
		}
		else
		{
			refreshPartial(x, y, Xcount - x, (y + ((Ycount+1) * 15)));	//refresh in partial window
		}
	}
	SPI_Com(0x92);
}

void drawImage (uint8_t imageNum, uint8_t mainCol, uint8_t backCol, uint8_t x, uint8_t y, uint8_t repeat, uint8_t Refresh)
{
	if (initialised == 0)	//Check if Display has been initialised since last Deep Sleep
	{
		Init();
	}
	uint8_t mode = 0x10;	//Value used to designate if it is currently drawing in BW mode or Yellow mode
	uint16_t XRepeatcount = 0;	//counter used to count how often the image has been repeatet in the X axis
	uint16_t YRepeatcount = 0;	//counter used to count how often the image has been repeatet in the X axis
	
	uint16_t counter = 0;	//counter used for going throught the values in the picture list
	
	uint16_t EepromPos = 0;	//Varible used to Store the current position in the Eeprom
	uint16_t EepromStart = 0;	//Variable which stores the starting plosition of the image inside of the Eeprom
	uint16_t height = 0;	//Variable used to store the height of the image
	uint16_t width = 0;	//Variable used to store the width of the image
	
	for(counter = 0; counter <= imageNum; counter++)
	{
		width = eeprom_read_byte((uint8_t*)(EepromStart));	//get width of image from Eeprom
		EepromStart++;
		height = eeprom_read_byte((uint8_t*)(EepromStart));	//get height of image from Eeprom
		EepromStart++;

		if(counter != imageNum)	//if current image is not the requested image Set Eeprom start to the start of the next image
		{
			EepromStart += ((width/8) * height);
		}
	}
	
	
	uint16_t roundY = (((DispHeight - y)*100) / (height*100)) + ((((DispHeight - y)*100) / (height*100)) % 100);	//Calculate a rounded up value for the repetition count of the picture on the Y axis
	uint16_t roundX = (((DispWidth - (x * 8))*100) / (width*100)) + ((((DispWidth - (x * 8))*100) / (width*100)) % 100);	//Calculate a rounded up value for the repetition count of the picture on the X axis
	
	for (mode = 0x10; mode <= 0x13; mode += 0x03)	//After BW mode change to Yellow mode
	{
		for (YRepeatcount = 0; YRepeatcount < roundY; YRepeatcount++)
		{
			for (XRepeatcount = 0; XRepeatcount < roundX; XRepeatcount++)
			{
				SPI_Com(0x92);	//close last window
				defineWindow(x + (XRepeatcount * (width/8)), y + (YRepeatcount * height), (width/4) - 1, height-1);	//define a new window
				SPI_Com(mode);	//Set the color mode
				Wait4Idle();
				
				EepromPos = EepromStart;	//Set current Position in Eeprom to the starting Position of the image
				
				for (counter = 0; counter < ((width / 8) * height); counter++)
				{
					if(mainCol == backCol)	//check if both colors are equal
					{
						if (mainCol == 0)	//if mainCol is 0 send full Black
						{
							SPI_Data(0x00);
						}
						else if (mainCol == 1 && mode == 0x10)	//if mainCol is 1 and current mode is BW send full white
						{
							SPI_Data(0xFF);
						}
						else if (mainCol == 2 && mode == 0x13)	//if mainCol is 2 and current mode is Yellow send full Yellow
						{
							SPI_Data(0xFF);
						}
					}
					else if ((mainCol == 0 && mode == 0x10) || (backCol == 2 && mode == 0x13))	//Check if data can be sent normaly
					{
						SPI_Data(eeprom_read_byte((uint8_t*)EepromPos));	//send data normaly
					}
					else if ((backCol == 0 && mode == 0x10) || (mainCol == 2 && mode == 0x13))	//Check if data needs to be sent nverted
					{
						SPI_Data(~eeprom_read_byte((uint8_t*)EepromPos));	//send data inverted
					}
					EepromPos += 1;
				}
				if(repeat == 0)	//check if repeat is false
				{
					YRepeatcount = roundY;	//set YRepeatcount to its maximum to stop repetition
					XRepeatcount = roundX;	//set XRepeatcount to its maximum to stop repetition
				}
			}
		}
	}
	if (Refresh != 0)	//check if display needs to be refreshed
	{
		if(repeat != 0)	//Check if repeat is not active
		{
			refreshPartial(x, y, (DispWidth / 4) - x, DispHeight - y);	//Refresh area of picture
		}
		else
		{
			refresh();	//refresh entire display
		}
	}
	SPI_Com(0x92);	//close window
}

void refresh()
{
	Wait4Idle();
	SPI_Com(0x12);	//refresh display
}

void refreshPartial(uint8_t x, uint8_t y,  uint8_t width, uint8_t height)
{
	Wait4Idle();
	defineWindow(x, y, width, height);	//define window
	SPI_Com(0x12);	//refresh
}

void powerOFF()
{
	Wait4Idle();
	SPI_Com(0x02);	//turn power off
	initialised = 0;	//set initialised to 0
}

void defineWindow(uint8_t x, uint8_t y,  uint8_t width, uint8_t height)
{
	Wait4Idle();
	SPI_Com(0x91);	//go to partial mode
	SPI_Com(0x90);	//window define mode
	SPI_Data(x<<3);		//define x start
	SPI_Data(((x+(width/2))<<3) + 5);	//define x end
	//define y start
	//==================================================================
	SPI_Data(y>>8);		
	SPI_Data(y);
	//==================================================================
	
	//define y end
	//==================================================================
	SPI_Data((y+height)>>8);
	SPI_Data((y+height));
	//==================================================================
	
	SPI_Data(0x01);	//set gate scan mode
}




