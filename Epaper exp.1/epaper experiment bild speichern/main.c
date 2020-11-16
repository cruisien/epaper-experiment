
#define F_CPU 8000000UL
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <avr/eeprom.h>

const uint8_t bild0[]={16/*bild hoehe*/,4,0b11111111, 0b11111111, 0b11111111, 0b11111111,0b11111111, 0b11111111, 0b11111111, 0b11111111};


const uint8_t bild2[]={16,4,0b11000011, 0b11000011, 0b11000011, 0b11000011,0b11000011, 0b11000011, 0b11000011, 0b11000011}; //bilder 


const uint8_t bild1[]={16,4,0b10011001, 0b10011001, 0b10011001, 0b10011001,0b10011001, 0b10011001, 0b10011001, 0b10011001};

const uint8_t bilcnt = 3;                                      //menge bilder
const  unsigned char* const bilder[] = {bild0, bild1, bild2};//bilder reihenfolge


int main (void)
{
	uint16_t position = 0;
	uint16_t poscntr = 0;
	uint8_t bilcntr = 0;
	
	while(bilcntr != bilcnt){
		position += sizeof(bilder[bilcntr]);
		while(position != poscntr){
			eeprom_write_byte((uint8_t*)(poscntr + position), ((bilder[bilcntr])[poscntr]));
			poscntr ++;
		}
		bilcntr ++;
	}

	
	
	
return 0;
}
