typedef unsigned char * SHORTPTR;


#define SERNUM      1000
#define VERNUMH     1
#define VERNUML     0

//#define USA_32768 1
#define LCD_LTG 1   //2022 per display decoder tv (mancherebbero le iconcine, che prima erano le frecce sotto i digit)
// 	A=cuore B=wifi C=power red D= power blue E=punto alto F= punto basso
//#define USA_SVEGLIA 1
//#define H12_24 1
#define HOUR_TICK 1

enum {
	BEEP_STD_FREQ=60,		// 4KHz @16MHz
	BEEP_ERR_FREQ=100
	};



// il Timer0 conta ogni 250nSec*prescaler... (@16MHz CPUCLK => 8MHz) su PIC18
//#define TMR0BASE ((65536-62500)-0)			// 2Hz per orologio
#define TMR0BASE ((256-206)-1)			// 300Hz per timer/orologio/mux (a
#ifdef USA_32768
#define TMR1BASE 65536-3277					// 10Hz per orologio
#else
// il Timer1 conta ogni 250nSec*prescaler... (@16MHz CPUCLK => 8MHz) su PIC18
//#define TMR1BASE ((65536-26655)-0)		// 300Hz per timer/orologio/mux (a
//#define TMR1BASE ((65536-50000)+3)		// 10Hz per orologio
//#define TMR1BASE ((65536-(50000-500))+3)		// 10Hz per orologio 1% correzione
#define TMR1BASE ((65536-(50000-525))+3)		// 10Hz per orologio 5x10000 correzione
#endif




void EEscrivi_(SHORTPTR addr,BYTE n);
BYTE EEleggi(SHORTPTR addr);
void EEscriviDword(SHORTPTR addr,DWORD n);
DWORD EEleggiDword(SHORTPTR addr);
void EEscriviWord(SHORTPTR addr,WORD n);
WORD EEleggiWord(SHORTPTR addr);
#define EEcopiaARAM(p) { *p=EEleggi(p); }
#define EEcopiaAEEPROM(p) EEscrivi_(p,*p)



	
void Delay_uS(BYTE );
void Delay_mS(BYTE );
void Delay_S_(BYTE );
#define Delay_S() Delay_S_(10)
#define SPI_DELAY() Delay_SPI()



void handle_events(void);
void updateUI(void);
WORD getPuntiDaDisplay(void);
void showNumbers(int,BYTE);
void show2Numbers(BYTE,BYTE,char);
void showAllNumbers(int,BYTE);
void showChar(char,char,BYTE,BYTE);
void showOverFlow(BYTE);
void showFruits(BYTE n1,BYTE n2,BYTE n3);


void SetBeep(BYTE);
void StdBeep(void);
void ErrorBeep(void);



