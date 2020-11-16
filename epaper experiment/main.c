/*
 *Code by Simon Chatziparaskewas 
 * finished this version 9.10.2020(european date)
 * written for Atmega8
 * used display: GDEW0213C38 (from GOOD DISPLAY)
 * used adapter for comunication with display: DESPI-CO2
 * 
 * i dont own the pictures
 * made bitmap for writing myself
 * 
*/


#define F_CPU 8000000UL
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <avr/eeprom.h>

#define CS1 PORTC |= (1<<PC0)//ignores you
#define CS0 PORTC &= ~(1<<PC0)//is spoken to
#define DC1 PORTC |= (1<<PC1)//data interpreted as data
#define DC0 PORTC &= ~(1<<PC1)//data interpreted as command
#define RES1 PORTC |= (1<<PC2)//reset off
#define RES0 PORTC &= ~(1<<PC2)//reset on
#define BUSY (PINC & (1<<PC3))//BUSY wen low do not talk
#define TASTE !(PIND & (1<<PD0))//taste



void Transd (char data);//transfer byte as data
void Transc (char data);//transfer byte as command
void EPD_init (void);//initialize display
void EPD_wait (void);//wait fore BUSY high if BUSY low
void EPD_display (uint8_t black, uint8_t yellow, uint16_t bytes);// draws number of bytes on display with defined colors
void EPD_displayarray (uint8_t ARRAY1, uint8_t ARRAY2 , uint16_t byte);//draws number of bytes on display with array as source ARRAY1 = Black ARRAY2 = Yellow if 0 is given as array it outputs white (array chosen with position of array in array: arraynr)
void EPD_refresh (void);//refreshes the colors use after data to display is transphered
void EPD_sleep (void);//shuts down display use at end of programm after use display needs to be initialised aganin
void EPD_window (uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint8_t* schwarz, uint8_t* gelb);// fills window with given color
void EPD_windowarray (uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint8_t ARRAY1, uint8_t ARRAY2);//same as EPD_displayarray in window (if y of window >1pixel it somhow adds 1 to the x (banks) no error in code has to be my display so you need to make wantet x2 -1 as input the other corrections are made in code)
void EPD_text (uint8_t x, uint8_t y, uint8_t color);// x defines bank (byte) and y pixel in top right of text color: 1=yellow 0=black  text needs to be defined before with strcpy(eingabe, "TEXT HERE") usable characters are in array: letters 
void EPD_letter (uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, char letter, uint8_t color);//displays letter used fore EPD_text cordinates same as EPD_window color same as EPD_text
void EPD_pletter (uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint16_t i, uint8_t color);//displays letter from given position in array:letters  rest same as EPD_letter
void EPD_displayeeprom (uint8_t x, uint8_t y, uint8_t bild, uint8_t color);



const uint8_t letters[]PROGMEM={//numb.6 // bitmap eingebbare zeichen
	0xff, 0xc0, 0xb7, 0xb7, 0xb7, 0xc0, 0xff,//A 0-6
	0xff, 0xc9, 0xb6, 0xb6, 0xb6, 0x80, 0xff,//B 7-13
	0xff, 0xdd, 0xbe, 0xbe, 0xbe, 0xc1, 0xff,//C 14-20
	0xff, 0xc1, 0xbe, 0xbe, 0xbe, 0x80, 0xff,//D 21-27
	0xff, 0xbe, 0xb6, 0xb6, 0xb6, 0x80, 0xff,//E 28-34
	0xff, 0xbf, 0xb7, 0xb7, 0xb7, 0x80, 0xff,//F 35-41
	0xff, 0xd9, 0xba, 0xbe, 0xbe, 0xc1, 0xff,//G 42-48
	0xff, 0x80, 0xf7, 0xf7, 0xf7, 0x80, 0xff,//H 49-55
	0xff, 0xff, 0xbe, 0x80, 0xbe, 0xff, 0xff,//I 56-62
	0xff, 0xbf, 0x81, 0xbe, 0xfe, 0xfd, 0xff,//J 63-69
	0xff, 0xbe, 0xdd, 0xeb, 0xf7, 0x80, 0xff,//K 70-76
	0xff, 0xfe, 0xfe, 0xfe, 0xfe, 0x80, 0xff,//L 77-83
	0xff, 0x80, 0xdf, 0xe7, 0xdf, 0x80, 0xff,//M 84-90
	0xff, 0x80, 0xfb, 0xf7, 0xef, 0x80, 0xff,//N 91-97
	0xff, 0xc1, 0xbe, 0xbe, 0xbe, 0xc1, 0xff,//O 98-104
	0xff, 0xcf, 0xb7, 0xb7, 0xb7, 0x80, 0xff,//P 105-111
	0xff, 0xc2, 0xbd, 0xba, 0xbe, 0xc1, 0xff,//Q 112-118
	0xff, 0xce, 0xb5, 0xb3, 0xb7, 0x80, 0xff,//R 119-125
	0xff, 0xb9, 0xb6, 0xb6, 0xb6, 0xce, 0xff,//S 126-132
	0xff, 0xbf, 0xbf, 0x80, 0xbf, 0xbf, 0xff,//T 133-139
	0xff, 0x81, 0xfe, 0xfe, 0xfe, 0x81, 0xff,//U 140-146
	0xff, 0x87, 0xf9, 0xfe, 0xf9, 0x87, 0xff,//V 147-153
	0xff, 0x81, 0xfe, 0xf1, 0xfe, 0x81, 0xff,//W 154-160
	0xff, 0x9c, 0xeb, 0xf7, 0xeb, 0x9c, 0xff,//X 161-167
	0xff, 0x9f, 0xef, 0xf0, 0xef, 0x9f, 0xff,//Y 168-174
	0xff, 0x9e, 0xae, 0xb6, 0xba, 0xbc, 0xff,//Z 175-181
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,//space 182-188
	0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff,//.  189-195
	0xff, 0xff, 0xff, 0x86, 0xff, 0xff, 0xff,// ! 196-202
	0xff, 0xff, 0xff, 0xff, 0xfd, 0xfe, 0xff,//,  203-209
	0xff, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xff,//-  210-216
	0xff, 0x87, 0xb7, 0xb2, 0xbf, 0x9f, 0xff,//?  217-223
	0xff, 0xc1, 0xba, 0xb6, 0xae, 0xc1, 0xff,//0 224-230
	0xff, 0xfe, 0xfe, 0x80, 0xde, 0xde, 0xff,//1 231-237
	0xff, 0xde, 0xae, 0xb6, 0xba, 0xdc, 0xff,//2 238-244
	0xff, 0xc9, 0xb6, 0xb6, 0xbe, 0xdd, 0xff,//3 245-251
	0xff, 0xfb, 0xfb, 0xf0, 0xfb, 0x83, 0xff,//4 252-258
	0xff, 0xb9, 0xb6, 0xb6, 0xb6, 0x86, 0xff,//5 259-265
	0xff, 0xd9, 0xb6, 0xb6, 0xb6, 0xc1, 0xff,//6 266-272
	0xff, 0x9f, 0xaf, 0xb0, 0xbf, 0xbf, 0xff,//7 273-279
	0xff, 0xc9, 0xb6, 0xb6, 0xb6, 0xc9, 0xff,//8 280-286
	0xff, 0xc1, 0xb6, 0xb6, 0xb6, 0xcd, 0xff,//9 287-293
	0xff, 0xf7, 0xf7, 0xc1, 0xf7, 0xf7, 0xff,//+ 294-300
	0xff, 0xff, 0xeb, 0xf7, 0xeb, 0xff, 0xff,//* 301-307
	0xff, 0xf7, 0xf7, 0xd5, 0xf7, 0xf7, 0xff,// / 308-314
	0xff, 0xd7, 0xd7, 0xd7, 0xd7, 0xd7, 0xff,//= 315-321
	};
char eingabe[] = {0}; // numb. 5// variable eingabestring  



const  unsigned char* const arraynr[] = {0/*0 needs to stay at first place*/,};


int main (void)
{

	uint16_t pixel = 2756;
	uint8_t taste = 1;

	
	DDRB |= (1<<2) | (1<<3) | (1<<5); // SCK, MOSI und SS outputs
	DDRC |= (1<<PC0) | (1<<PC1) | (1<<PC2); //init zusätzliche ports
	DDRB &= ~(1<<4); //MISO input
	DDRC &= ~(1<<PC3);//BUSY wen interrupted no data transfer
	DDRD &= ~(1<<PD0);//taster 
	PORTD |= (1<<PD0); //pullup taster
	
	
	SPCR |= (1<<MSTR); //set as master
	SPCR |= (1<<SPR0) | (1<<SPI2X); // divide clock by 
	SPCR |= (1<<SPE); // enable spi

		EPD_init();
		EPD_display(0xff,0xff,pixel);
		EPD_displayeeprom(1,2,2,1);
		EPD_refresh();
		EPD_wait();
		EPD_sleep();


		
		
return 0 ;

}//end of main

void Transd (char data)//transfer data
{


	DC1;//transpher read as data
	CS0; //display is spoken to
	SPDR = data; // send data
	while(!(SPSR & (1<<SPIF))); // wait till transmissioncempetition
	CS1;//"unspeak" display
}


void Transc (char data)//transfer command
{


	DC0;//transpher read as data
	CS0; //display is spoken to
	SPDR = data; // send data
	while(!(SPSR & (1<<SPIF))); // wait till transmissioncempetition
	CS1;//"unspeak" display
	DC1;//transpher read as data (for safety)
}




void EPD_init (void){//epaper initialisation
	//reset display processor
	RES0;
	_delay_ms(250);
	RES1;
	_delay_ms(250);
	

	Transc(0x06);//bost soft start
	Transd(0x17);//A
	Transd(0x17);//B
	Transd(0x17);//C
	
	Transc(0x04);//Power on
	EPD_wait();
	
	
	Transc(0x00);//Panel settings
	Transd(0b00000011);    //LUT from OTP£¬128x296
	Transd(0x0d);     //VCOM to 0V fast
	
	Transc(0x61);     //resolution setting
	Transd(0x68); //104
	Transd(0x00); //212
	Transd(0xd4);
	
	Transc(0X50);     //VCOM AND DATA INTERVAL SETTING
	Transd(0x77);    //WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
}

void EPD_displayarray (uint8_t ARRAY1, uint8_t ARRAY2 , uint16_t byte){
    uint16_t i;//byte counter for transpher & position of byte in array
    EPD_wait();
    Transc(0x10);        //Transfer black data
    for(i=0;i<byte;i++){
		EPD_wait();
		if(ARRAY1 == 0){//if no array is given
			Transd (0xff);
		}
		else{
			Transd(pgm_read_byte(&(arraynr[ARRAY1])[i]));
		} 
	}
	EPD_wait();
	
    Transc(0x13);        //Transfer yellow data
    for(i=0;i<byte;i++){   
		EPD_wait();
		if(ARRAY2 == 0){
			Transd (0xff);
		}
		else{
			Transd(pgm_read_byte(&(arraynr[ARRAY2])[i]));
		}
	}
	EPD_wait();
	Transc(0x11);//end of data transmition do not forget ore it wont start
	Transd(0xff);//data transmition complete
}

void EPD_display (uint8_t black, uint8_t yellow, uint16_t bytes)
{
    uint16_t i;
    EPD_wait();
    Transc(0x10);        //Transfer black data
    for(i=0;i<bytes;i++){
		EPD_wait();
		Transd(black);
	}
  
	EPD_wait();

    Transc(0x13);        //Transfer yellow data
    for(i=0;i<bytes;i++){
		EPD_wait();
		Transd(yellow);
  }
  EPD_wait();
  
  Transc(0x11);//end of data transmition do not forget ore it wont start
  Transd(0xff);
}

void EPD_window (uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint8_t* schwarz, uint8_t* gelb){
	uint16_t pixel = ((x2 - x1) * (y2 - y1));

	Transc(0x91);//enter window mode
		
	Transc(0x90);
	Transd(x1<<3);
	Transd((x2<<3) + 5);
	Transd(y1>>8);          //DEFINE WINDOW
	Transd(y1);
	Transd(y2>>8);
	Transd(y2);
	Transd(0x01);
	

	
	EPD_display(schwarz, gelb, pixel);//warning: passing argument 1 of 'EPD_display' makes integer from pointer without a cast
	
	EPD_wait();
	Transc(0x92);//leave window mode
}

void EPD_windowarray (uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint8_t ARRAY1, uint8_t ARRAY2){
	uint16_t pixel = (((x2+1) - x1) * (y2 - y1) + 1);

	Transc(0x91);
		
	Transc(0x90);//enter window mode
	Transd(x1<<3);
	Transd((x2<<3)+5);
	Transd(y1>>8);          //DEFINE WINDOW
	Transd(y1);
	Transd(y2>>8);
	Transd(y2);
	Transd(0x01);
	

	
	EPD_displayarray(ARRAY1, ARRAY2, pixel);
	
	EPD_wait();
	Transc(0x92);//leave window mode
}

void EPD_text (uint8_t x, uint8_t y, uint8_t color){
	uint8_t i = 0;
	uint8_t ni = 0;
	uint8_t x1;
	uint8_t x2;
	uint8_t y1;
	uint8_t y2;

	
	x2 = x + 1;
	x1 = x;
	y1 = y;
	y2 = y + 1;
	
	while(eingabe[i] != '\0'){//count lenght of string
		i++;
		ni++;
	}
	while(ni != 0){//output string backwards
		ni--;
		EPD_letter( x1, x2, y1, y2,eingabe[ni], color);
		EPD_wait();
		y1 = y1 + 7;
		y2 = y2 + 7;
	}

	
}

void EPD_letter (uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, char letter, uint8_t color){
	switch(letter){
		case 65 ... 90: EPD_pletter(x1,x2,y1,y2,((letter - 65) * 7),color); break;
		case 48 ... 57: EPD_pletter(x1,x2,y1,y2,((letter - 16) * 7),color); break;
		case ' ': EPD_pletter(x1,x2,y1,y2,182,color); break;
		case '!': EPD_pletter(x1,x2,y1,y2,196,color); break;
		case '.': EPD_pletter(x1,x2,y1,y2,189,color); break;
		case ',': EPD_pletter(x1,x2,y1,y2,203,color); break;
		case '-': EPD_pletter(x1,x2,y1,y2,210,color); break;
		case '?': EPD_pletter(x1,x2,y1,y2,217,color); break;
		case '+': EPD_pletter(x1,x2,y1,y2,294,color); break;
		case '*': EPD_pletter(x1,x2,y1,y2,301,color); break;
		case '/': EPD_pletter(x1,x2,y1,y2,308,color); break;
		case '=': EPD_pletter(x1,x2,y1,y2,315,color); break;
		
		
	}
	
	
}

void EPD_pletter (uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint16_t i, uint8_t color){
	uint8_t Y1 = y1;
	uint8_t Y2 = y2;
	uint16_t I = i;
	if(color == 0) {
		while((i+7) != I){//output all 7 banks (byte) alone
			EPD_window(x1,x2,Y1,Y2,(pgm_read_byte(&letters[I])), 0xff);//warning: passing argument 5/6 of 'EPD_window' makes pointer from integer without a cast
			Y2++;
			Y1++;
			I++;
		}
	}
	else if(color == 1){
		while((i+7) != I){//output all 7 banks (byte) alone
			EPD_window(x1,x2,Y1,Y2, 0xff,(pgm_read_byte(&letters[I])));//warning: passing argument 5/6 of 'EPD_window' makes pointer from integer without a cast
			Y2++;
			Y1++;
			I++;
		}
	}
}

void EPD_refresh (void){EPD_wait();
	Transc(0x12);     //DISPLAY REFRESH   
	_delay_ms(500);
	EPD_wait();
}


void EPD_sleep (void){
	Transc(0X50);  //VCOM AND DATA INTERVAL SETTING
	Transd(0xf7); //WBmode:VBDF 17|D7 VBDW 97 VBDB 57    WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
	Transc(0X02);   //power off
	EPD_wait();
}

void EPD_wait (void){
	while(BUSY == 0){//when BUSY low wait till no longer low
		_delay_ms(100);
	}
}

void EPD_displayeeprom (uint8_t x, uint8_t y, uint8_t bild, uint8_t color){
	uint16_t position = 0;
	uint8_t bildx = 0;
	uint8_t bildy= 0;
	uint8_t cntr = 0;
	
	
	while(bild != cntr){
		cntr++;
		bildx = eeprom_read_byte(position);
		position++;
		bildy = eeprom_read_byte (position);
		position++;
		if(bild != cntr){
			position += (bildx * bildy) / 8;
		}
	}
	cntr = 0;
	
	uint8_t x1 = x;
	uint8_t x2 = x + (bildx / 8);
	uint8_t y1 = y;
	uint8_t y2 = y + bildy;
	Transc(0x91);//enter window mode
		
	Transc(0x90);
	Transd(x1<<3);
	Transd((x2<<3) + 5);
	Transd(y1>>8);          //DEFINE WINDOW
	Transd(y1);
	Transd(y2>>8);
	Transd(y2);
	Transd(0x01);
	if(color == 0){
		while(cntr != (bildx * bildy) / 8){
			EPD_wait();
			Transc(0x10);
			Transd(eeprom_read_byte(position));
			cntr++;
			position++;
		}
	}
	if(color == 1){
		while(cntr != (bildx * bildy) / 8){
		EPD_wait();
		Transc(0x13);
		Transd(eeprom_read_byte(position));
		cntr++;
		position++;
		}
	}
	EPD_wait();
	Transc(0x11);//end of data transmition do not forget ore it wont start
	Transd(0xff);//data transmition complete
	EPD_wait();
	Transc(0x92);//leave window mode
	
	
	
	
	
}



