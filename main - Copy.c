/********************************************************************
 FileName:     main.c
 Dependencies: See INCLUDES section
 Processor:		PIC18, PIC24, and PIC32 USB Microcontrollers
 Hardware:		This demo is natively intended to be used on Microchip USB demo
 				boards supported by the MCHPFSUSB stack.  See release notes for
 				support matrix.  This demo can be modified for use on other hardware
 				platforms.
 Complier:  	Microchip C18 (for PIC18), C30 (for PIC24), C32 (for PIC32)
 Company:		Microchip Technology, Inc.

 Software License Agreement:

********************************************************************
 File Description:

 Change History:
  Rev   Description
  ----  -----------------------------------------
  1.0   Initial release, basata su LX336 di Nuova Elettronica


Dedicato alla dolce guerra :) (il nasdaq muore, le atomiche arrivano...)

********************************************************************/

#ifndef MAIN_C
#define MAIN_C

/** INCLUDES *******************************************************/

#include <generictypedefs.h>
#include <delays.h>
#include <timers.h>
#include <reset.h>
#include <pwm.h>
#include <portb.h>
#include <stdio.h>
#include <stdlib.h>
#include "slotmachine.h"
#include "io_cfg.h"

/** CONFIGURATION **************************************************/


#pragma config WDT = ON, WDTPS = 32768, MCLRE=ON, STVREN=ON, LVP=OFF
#pragma config OSC=INTIO7, FCMEN=OFF, IESO=OFF, PWRT=ON, BOREN=SBORDIS, BORV=3
#pragma config LPT1OSC=OFF, PBADEN=OFF, CCP2MX=PORTC, XINST=ON, DEBUG=OFF


#pragma romdata
static rom const char CopyrString[]= {'C','y','b','e','r','d','y','n','e',' ','(','A','D','P','M',')',' ','-',' ','S','l','o','t','M','a','c','h','i','n','e',' ','L','X','3','3','6',' ',
	VERNUMH+'0','.',VERNUML/10+'0',(VERNUML % 10)+'0', ' ','0','7','/','0','5','/','2','2', 0 };

const rom BYTE table_7seg[]={0, // .GFEDCBA
															0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,
												 			0b01101101,0b01111101,0b00000111,0b01111111,0b01101111,
															0b01000000,			// il segno meno...
															0b01001001,			// no palle; 12
															0b01101011,			// palla alta
															0b01011101,			// palla bassa
															0b01111111,			// due palle

															};

#pragma romdata myidlocs=0x200000
const rom char data0=0x04u;
const rom char data1=0x04u;
const rom char data2=0x04u;
const rom char data3=0x07u;
const rom char data4=0x00u;
const rom char data5=0x00u;
const rom char data6=0x00u;
const rom char data7=0x00u;
#pragma romdata

#pragma udata

#define BASE_FIRST 6
BYTE first=BASE_FIRST;

BYTE gameMode=0;
BYTE durataGioco,durataGioco2;
WORD Punti,totPunti;

volatile BYTE tmr_cnt,second_1,second_100;
volatile BYTE Timer10;
BYTE dividerBeep;
BYTE dividerUI;

enum MODES {
	MODE_DEFAULT=0
	};

BYTE Displays[5][2];		// 3 display + segni vari (display microonde Daeweoo; poi display decoder ecc); RrlLx ossia RA0 è extra, RA1 è Leftmost ecc
enum MODES Mode=MODE_DEFAULT;					// ore/minuti; minuti/secondi; giorno/mese; anno

unsigned char dMode;


/** PRIVATE PROTOTYPES *********************************************/
static void InitializeSystem(void);
void ProcessIO(void);
void UserInit(void);
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();


/** VECTOR REMAPPING ***********************************************/
	
	#pragma code HIGH_INTERRUPT_VECTOR = 0x08
	void High_ISR(void)
	{
  _asm 
	  goto YourHighPriorityISRCode 
	_endasm
	}
	#pragma code LOW_INTERRUPT_VECTOR = 0x18
	void Low_ISR(void)
	{
  _asm 
		goto YourLowPriorityISRCode
	_endasm
	}
	

#pragma code

WORD Tmr1Base;
signed char clock_correction=0;
	
//These are your actual interrupt handling routines.
#pragma interrupt YourHighPriorityISRCode
void YourHighPriorityISRCode() {		//
	static BYTE dividerMux=1;
	signed char mux_corr;
		
//	if(INTCONbits.TMR0IE && INTCONbits.TMR0IF) {		//c'è solo questo
// 3.32 mSec 22/5/20 con ottimizzazioni

	
		LATA=dividerMux;
		switch(dividerMux) {
			case 1:
				LATB=~Displays[3][dMode];
//LATD=0b00000001;
				dividerMux = 2;
				break;
			case 2:
				LATB=~Displays[2][dMode];
//				LATD |= 0b10000000;		// dp fisso per ora!
//LATD=0b00000010;
				dividerMux = 4;
				break;
			case 4:
				LATB=~Displays[1][dMode];
//LATD=0b00000100;
				dividerMux = 8;
				break;
			case 8:
				LATB=~Displays[0][dMode];
//LATD=0b00001000;
				dividerMux = 16;
				break;
			case 16:
				LATB=~Displays[4][dMode];
//LATD=0b00001000;
				dividerMux = 1;
				break;
			}

#define MUX_CORR_STEP 5
		mux_corr=0;		// correggo tempo mux per dare più tempo se c'è più assorbimento!
		if(!(LATB & 1))
			mux_corr+=MUX_CORR_STEP;
		if(!(LATB & 2))
			mux_corr+=MUX_CORR_STEP;
		if(!(LATB & 4))
			mux_corr+=MUX_CORR_STEP;
		if(!(LATB & 8))
			mux_corr+=MUX_CORR_STEP;
		if(!(LATB & 16))
			mux_corr+=MUX_CORR_STEP;
		if(!(LATB & 32))
			mux_corr+=MUX_CORR_STEP;
		if(!(LATB & 64))
			mux_corr+=MUX_CORR_STEP;

//	m_Led2Bit ^= 1; //check timer	

		INTCONbits.TMR0IF = 0;

//		WriteTimer0(TMR0BASE);					// inizializzo TMR0; no funzioni per overhead
		// WRITETIMER0(0) dovrebbe essere la macro!
//		TMR0H=Tmr0Base / 256;					// reinizializzo TMR0
		TMR0L=TMR0BASE-mux_corr;					// reinizializzo TMR0
// mah, non sembra fare molto a livello visivo...


	
//		}
	
	}	//This return will be a "retfie fast", since this is in a #pragma interrupt section 


volatile BYTE divider1s;

#pragma interruptlow YourLowPriorityISRCode
void YourLowPriorityISRCode()	{
		//Check which interrupt flag caused the interrupt.
		//Service the interrupt
		//Clear the interrupt flag
		//Etc.


// 100.001 mSec 22/5/2020; ha perso 7 minuti in 11.5 ore ossia 420:41400, 1%


//	if(PIE1bits.TMR1IE && PIR1bits.TMR1IF) {

		//WriteTimer1(TMR1BASE);					// inizializzo TMR0
		//WRITETIMER1(TMR1BASE); //dovrebbe essere la macro! solo su Hitech...
		TMR1H=Tmr1Base >> 8; // è SEMPRE a 16bit - non è chiaro a che serva il RW_8BIT
		TMR1L=Tmr1Base & 0xff; //FINIRE sono 8mSec!!

//	m_Led2Bit ^= 1; //check timer	

		Timer10++;
		second_100=1;					// flag

		divider1s++;
		if(divider1s==10) {		// per RealTimeClock
			divider1s=0;
			second_1=1;					// flag
			}


		PIR1bits.TMR1IF = 0;
//		}


	
	}	//This return will be a "retfie", since this is in a #pragma interruptlow section 




/** DECLARATIONS ***************************************************/
#pragma code

/********************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
void main(void) {   

//	STKPTR=0;		// risparmia una posizione di Stack, xché main() è stata CALLed!

//		showNumbers(-42,1);

  InitializeSystem();

  while(1) {

		// Application-specific tasks.

		if(second_100) {

			second_100=0;
    	handle_events();        
			updateUI();
			}

    }	//end while
	}	//end main


/********************************************************************
 * Function:        static void InitializeSystem(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        InitializeSystem is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.                  
 *
 * Note:            None
 *******************************************************************/
static void InitializeSystem(void) {

	StatusReset();

	ClrWdt();

	// imposto oscillatore a 16MHz (per rallentare Timer1...)
	OSCCONbits.IRCF2=1;
	OSCCONbits.IRCF1=1;
	OSCCONbits.IRCF0=0;
	OSCTUNEbits.PLLEN=1;

	OSCCONbits.SCS0 = 0;
	OSCCONbits.SCS1 = 0;

	OSCCONbits.IDLEN=0;

	Delay_mS(100);		// sembra sia meglio aspettare un pizzico prima di leggere la EEPROM.. (v. forum 2006)

  ADCON1 |= 0x0F;                 // Default all pins to digital


  UserInit();
    
	}	//end InitializeSystem



/******************************************************************************
 * Function:        void UserInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine should take care of all of the demo code
 *                  initialization that is required.
 *
 * Note:            
 *
 *****************************************************************************/
void UserInit(void) {
	BYTE i;

	ClrWdt();

	srand(PORTA+PORTB+PORTC);

	EnablePullups();

	TRISA=0b00000000;		// 4(5) mux
	TRISB=0b00000000;		// 7segmenti+DP
	TRISC=0b11001011;		// I2C; 1 led; 2 switch; buzzer

	LATA=0b00000000;		// tutto spento
	LATB=0b00000000;
	LATC=0b00000000;

	Tmr1Base=TMR1BASE;

	OpenTimer0(TIMER_INT_ON & T0_8BIT & T0_SOURCE_INT & T0_PS_1_64);			// mux display
	// v. ReadTimer sotto...
#ifdef USA_32768
	OpenTimer1(TIMER_INT_ON & T1_16BIT_RW & T1_SOURCE_EXT & T1_OSC1EN_ON & T1_SYNC_EXT_OFF & T1_PS_1_1);		// RTC (32768Hz)
#else
	OpenTimer1(TIMER_INT_ON & T1_16BIT_RW & T1_SOURCE_INT & T1_OSC1EN_OFF & T1_SYNC_EXT_OFF & T1_PS_1_8);		// RTC (100mS opp 32768Hz)
//	T1CON=0b10110101;
//	PIE1bits.TMR1IE=1;
#endif

	OpenTimer2(TIMER_INT_OFF & T2_PS_1_16 & T2_POST_1_1);		// per Buzzer

	OpenPWM1(BEEP_STD_FREQ); SetDCPWM1(200 /* MSB di 200 << 6 va in CCPR1L..*/);		// SetOutputPWM1(SINGLE_OUT,PWM_MODE_1); // FINIRE! ma SetOutputPWM1 c'è solo sui moduli per motori ossia 4 PWM
	OpenPWM1ConfigIO();			// fa il TRIS...
	dividerBeep=2;		//tanto per... all'accensione!

	RCONbits.IPEN=1;				// interrupt in Modalita' avanzata (18X)
	INTCON2bits.TMR0IP = 1;			// Timer-0 high Pty
	IPR1bits.TMR1IP = 0;				// Timer-1 low Pty, ev. crystal RTC
//	IPR1bits.TMR2IP = 0;				// Timer-2 Low Pty NON USATO

	TMR0L=TMR0BASE & 255;					// 
	TMR1H=TMR1BASE / 256;					// inizializzo TMR1
	TMR1L=TMR1BASE & 255;					// 
  
	INTCONbits.GIEL = 1;			// attiva interrupt LO-pri
	INTCONbits.GIEH = 1;			// attiva interrupt HI-pri

	Displays[0][0]=Displays[1][0]=Displays[2][0]=Displays[3][0]=Displays[4][0]=0xff;
	Displays[0][1]=Displays[1][1]=Displays[2][1]=Displays[3][1]=Displays[4][1]=0xff;

	dMode=0;


	Tmr1Base=TMR1BASE+clock_correction;			// anche sotto in First...
	dividerUI=0;

	gameMode=0;
	totPunti=0;
//	totPunti=EEleggi(&totPunti);
//	totPunti |= ((WORD)EEleggi(1+&totPunti)) << 8;

	Mode=MODE_DEFAULT;

	}		//end UserInit


void handle_events(void) {
	static BYTE oldPuls1Bit=3,timePressed1;
	WORD n;

	ClrWdt();

		dividerUI++;
		
		if(first)
			return;
	
		if(!m_Puls2Bit) {
			if(oldPuls1Bit & 2) {
				SetBeep(2);		// tanto per...
	

				}

			if(!m_Puls1Bit) {
				SetBeep(5);		// 
				totPunti=0;
				gameMode=1;
//				EEscrivi_(&totPunti,LOBYTE(totPunti));
//				EEscrivi_(1+&totPunti,HIBYTE(totPunti));
				showAllNumbers(totPunti,0);
				while(!m_Puls1Bit)			// debounce per evitare che lo prenda sotto!
					ClrWdt();
				}

			oldPuls1Bit &= ~2;
			}
		else {
	
//			Displays[4] &= ~0b01111001;
			oldPuls1Bit |= 2;
			}

		if(!m_Puls1Bit) {
			if(oldPuls1Bit & 1) {
				timePressed1=0;
				}
			if(gameMode<2)
				gameMode=2;

			oldPuls1Bit &= ~1;
			}
		else {
			if(!(oldPuls1Bit & 1)) {
				if(gameMode<3) {
					gameMode=3;
					durataGioco=durataGioco2=(timePressed1*2) + (rand() & 15) + 10;
//					durataGioco=durataGioco2=20;
					}
				}
			oldPuls1Bit |= 1;
			}
		if(timePressed1<30)			// 3 secondi max
			timePressed1++;
	//	else si potrebbe innescare lancio cmq!
	

	}


void updateUI(void) {
	BYTE i;
	static BYTE dividerT;


	if(dividerUI>=5) {		//500mS

		m_Led0Bit ^= 1;
	
		dividerUI=0;
	
		if(dividerBeep) {
			dividerBeep--;
			if(!dividerBeep) {
	//			T2CONbits.TMR2ON=0;
				ClosePWM1();
				}
		
			}
	
		if(first) {			// lascia Splash per un po'..
			first--;
			
			if(first>((BASE_FIRST/2)-1)) {
				showChar(0,CopyrString[42],3,0);
				showChar(0,CopyrString[43],2,0);
				showChar(0,CopyrString[45],1,0);
				showChar(0,CopyrString[46],0,0);
				}
			else {
				showChar(0,CopyrString[48],3,0);
				showChar(0,CopyrString[49],2,0);
				showChar(0,'-',1,0);
				showChar(0,'-',0,0);
				}
	

			if(!first) {
				Displays[4][0] = 0b00000000;		//off
				dividerT=0;
				}
	
			return;
			}
		}
	
	if(first) 
		return;

	dividerT++;

	switch(gameMode) {
		case 0:		// demo/fermo
			dMode=0;
			switch(dividerT) {
				case 0:
				case 12:
					Displays[0][0] = 0b00000001;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00000000;
					break;
				case 1:
				case 13:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00000001;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00000000;
					break;
				case 2:
				case 14:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000001;
					Displays[3][0] = 0b00000000;
					break;
				case 3:
				case 15:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00000001;
					break;
				case 4:
				case 16:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00100000;
					break;
				case 5:
				case 17:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00010000;
					break;
				case 6:
				case 18:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00001000;
					break;
				case 7:
				case 19:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00001000;
					Displays[3][0] = 0b00000000;
					break;
				case 8:
				case 20:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00001000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00000000;
					break;
				case 9:
				case 21:
					Displays[0][0] = 0b00001000;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00000000;
					break;
				case 10:
				case 22:
					Displays[0][0] = 0b00000100;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00000000;
					break;
				case 11:
				case 23:
					Displays[0][0] = 0b00000010;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00000000;
					break;
				case 24:
				case 25:
				case 26:
				case 27:
				case 28:
					Displays[0][0] = 0b01101110;		// PLAy
					Displays[1][0] = 0b01110111;
					Displays[2][0] = 0b00111000;
					Displays[3][0] = 0b01110011;
					break;
				case 29:
				case 31:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00000000;
					break;
				case 30:
				case 32:
					Displays[0][0] = 0b01000000;
					Displays[1][0] = 0b01000000;
					Displays[2][0] = 0b01000000;
					Displays[3][0] = 0b01000000;
					break;
				case 33:
				case 34:
				case 35:
				case 36:
				case 37:
					Displays[0][0] = 0b01111000;		// SLOt
					Displays[1][0] = 0b00111111;
					Displays[2][0] = 0b00111000;
					Displays[3][0] = 0b01101101;
					break;
				case 38:
				case 40:
				case 42:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00000000;
					break;
				case 39:
				case 41:
				case 43:
					Displays[0][0] = 0b01000000;
					Displays[1][0] = 0b01000000;
					Displays[2][0] = 0b01000000;
					Displays[3][0] = 0b01000000;
					break;
				case 44:
					Displays[0][0] = 0b00000000;
					Displays[1][0] = 0b00000000;
					Displays[2][0] = 0b00000000;
					Displays[3][0] = 0b00000000;
					Displays[4][0] = 0b00000000;
					break;
				case 45:
					gameMode=1;
					dividerT=0;
					break;
				}
			break;

		case 1:		// demo/punti
			dMode=1;
			showAllNumbers(totPunti,0);
			if(dividerT>=12) {
				dividerT=0;
				gameMode=0;
				}
			break;

		case 2:		// prerolling
			Displays[0][0] = 0b00000000;
			Displays[1][0] = 0b01000000;
			Displays[2][0] = rand() & 1 ? 0b01000000 : 0b00000000;
			Displays[3][0] = 0b01000000;
			break;

		case 3:		// rolling
			dMode=0;
			if(durataGioco2) {
				if(dividerT >= ((durataGioco-durataGioco2)/10)) {
					showFruits(rand() & 3,rand() & 3,rand() & 3);		// escono sempre diversi... boh
					durataGioco2--;
					dividerT=0;
					}
				}
			else {
				if(dividerT > 20) {
					gameMode=4;
					dividerT=0;
					}
				}
			break;

		case 4:		// fine rolling/punteggio
			dMode=1;
			if(dividerT == 1) {
				Punti=getPuntiDaDisplay();
				showNumbers(Punti,0);
				totPunti += Punti;
				}
//				EEscrivi_(&totPunti,LOBYTE(totPunti));
//				EEscrivi_(1+&totPunti,HIBYTE(totPunti));
			if(dividerT >= 20) {
				gameMode=1;
				dMode=1;
				dividerT=0;
				}
			break;
		}
		
	}


WORD getPuntiDaDisplay(void) {
	WORD n;
	BYTE t[4];
	BYTE i;

	t[0]=t[1]=t[2]=t[3]=0;
	for(i=1; i<4; i++) {
		switch(Displays[i][0]) {
			case 0b01001001 /*12*/:			// non è il massimo ma ok..
				t[0]++;
				break;
			case 0b01101011 /*12+1*/:
				t[1]++;
				break;
			case 0b01011101 /*12+2*/:
				t[2]++;
				break;
			case 0b01111111 /*12+3*/:
				t[3]++;
				break;
			}
		}

	switch(t[3]) {
		case 3:
			return 50;
			break;
		case 2:
			if(t[1]>0 || t[2]>0) {
				return 20;
				}
			else {
				return 10;
				}
			break;
		case 1:
			if(t[1]==2 || t[2]==2) {
				return 5;
				}
			else if(t[1]==1 || t[2]==1) {
				return 3;
				}
			else {
				return 2;
				}
			break;
		case 0:
			if(t[1]==3 || t[2]==3) {
				return 3;
				}
			else if(t[1]==2 || t[2]==2) {
				return 1;
				}
			break;
		}

	return 0;
	}

void showChar(char q,char n,BYTE pos,BYTE dp) {

	if(n>='0' && n<='9')
		Displays[pos][q]=table_7seg[n-'0'+1];
	else if(n>='A' && n<='Z')
		Displays[pos][q]=table_7seg[n-'A'+10+1];
	else if(n=='-')
		Displays[pos][q]=table_7seg[11];
	else if(!n)
		Displays[pos][q]=table_7seg[0];
	else 
		Displays[pos][q]=table_7seg[n];
	if(dp)
		Displays[pos][q] |= 0b10000000;
	}

void showFruits(BYTE n1,BYTE n2,BYTE n3) {

	showChar(0,n1+12,3,0);
	showChar(0,n2+12,2,0);
	showChar(0,n3+12,1,0);
	showChar(0,0,0,0);
	}


void showOverFlow(BYTE t) {

	if(t) {
		Displays[0][1] |= 0b10000000;
		Displays[1][1] |= 0b10000000;
		Displays[2][1] |= 0b10000000;
		Displays[3][1] |= 0b10000000;
		}
	}

void show2Numbers(BYTE n1,BYTE n2,char m) {
	char myBuf[8];

	myBuf[3]= (n1 / 10) + '0';
	showChar(1,myBuf[3],3,0 /*!prec MAI */);
	myBuf[2]= (n1 % 10) + '0';
	showChar(1,myBuf[2],2,0);
	myBuf[1]= (n2 / 10) + '0';
	showChar(1,myBuf[1],1,0);
	myBuf[0]= (n2 % 10) + '0';
	showChar(1,myBuf[0],0,0);
#ifdef LCD_LTG
	if(m)
		Displays[4][1] |= 0b00110000;
	else
		Displays[4][1] &= ~0b00110000;
#else
	if(m)
		Displays[4][1] |= 0b00000110;
	else
		Displays[4][1] &= ~0b00000110;
#endif
	}

void showNumbers(int n,BYTE prec) {
	char myBuf[8];
	char sign;		// 1 se negativo

	if(n<0) {
		sign=1;
		n=-n;
		}
	else
		sign=0;
	myBuf[0]= (n % 10) + '0';
	showChar(1,myBuf[0],0,0 /*!prec MAI */);
	n /= 10;
	prec--;
	myBuf[1]= (n % 10) + '0';
	showChar(1,myBuf[1],1,!prec);
	n /= 10;
	prec--;
	myBuf[2]= (n % 10) + '0';
	showChar(1,myBuf[2],2,!prec);
	n /= 10;
	prec--;
	if(sign) {
//		myBuf[0]='-';
		showChar(1,'-',3,!prec);
		if(n>=1)
			showOverFlow(1);
		}
	else {
		myBuf[3]= (n % 10) + '0';
		showChar(1,myBuf[3] != '0' ? myBuf[3] : 0,3,!prec);
		if(n>=10)
			showOverFlow(1);
		}
	}

void showAllNumbers(int n,BYTE prec) {
	char myBuf[8];
	char sign;		// 1 se negativo

	if(n<0) {
		sign=1;
		n=-n;
		}
	else
		sign=0;
	myBuf[0]= (n % 10) + '0';
	showChar(1,myBuf[0],0,0 /*!prec MAI */);
	n /= 10;
	prec--;
	myBuf[1]= (n % 10) + '0';
	showChar(1,myBuf[1],1,!prec);
	n /= 10;
	prec--;
	myBuf[2]= (n % 10) + '0';
	showChar(1,myBuf[2],2,!prec);
	n /= 10;
	prec--;
	if(sign) {
//		myBuf[0]='-';
		showChar(1,'-',3,!prec);
		if(n>=1)
			showOverFlow(1);
		}
	else {
		myBuf[3]= (n % 10) + '0';
		showChar(1,myBuf[3],3,!prec);
		if(n>=10)
			showOverFlow(1);
		}
	}

void StdBeep(void) {

	CCPR1L=BEEP_STD_FREQ/2;
	CCPR1H=BEEP_STD_FREQ/2;
	PR2=BEEP_STD_FREQ;

	T2CONbits.TMR2ON=1;
	Delay_S();
	T2CONbits.TMR2ON=0;
	}

void SetBeep(BYTE n) {

//	CCPR1L=BEEP_STD_FREQ/2;
//	CCPR1H=BEEP_STD_FREQ/2;
//	PR2=BEEP_STD_FREQ;

//	T2CONbits.TMR2ON=1;
	OpenPWM1(BEEP_STD_FREQ); SetDCPWM1(BEEP_STD_FREQ*2 /* MSB di 200 << 6 va in CCPR1L..*/);		// SetOutputPWM1(SINGLE_OUT,PWM_MODE_1); // FINIRE! ma SetOutputPWM1 c'è solo sui moduli per motori ossia 4 PWM
	OpenPWM1ConfigIO();			// fa il TRIS...

	dividerBeep=1 /*n*/;		// FINIRE SISTEMARE tempi sopra!

	}

void ErrorBeep(void) {

	CCPR1L=BEEP_STD_FREQ/2+30;
	CCPR1H=BEEP_STD_FREQ/2+30;
	PR2=BEEP_STD_FREQ+60;

	T2CONbits.TMR2ON=1;
	Delay_S();
	T2CONbits.TMR2ON=0;
	}



// -------------------------------------------------------------------------------------
void EEscrivi_(SHORTPTR addr,BYTE n) {		// usare void * ?

	EEADR = (BYTE)addr;
	EEDATA=n;

	EECON1bits.EEPGD=0;		// Point to Data Memory
	EECON1bits.CFGS=0;		// Access EEPROM
	EECON1bits.WREN=1;

	INTCONbits.GIE = 0;			// disattiva interrupt globali... e USB?
	EECON2=0x55;		 // Write 55h
	EECON2=0xAA;		 // Write AAh
	EECON1bits.WR=1;									// abilita write.
	INTCONbits.GIE = 1;			// attiva interrupt globali
	do {
		ClrWdt();
		} while(EECON1bits.WR);							// occupato ? 


	EECON1bits.WREN=0;								// disabilita write.
  }

BYTE EEleggi(SHORTPTR addr) {			// usare void * ?

	EEADR=(BYTE)addr;			// Address to read
	EECON1bits.EEPGD=0;		// Point to Data Memory
	EECON1bits.CFGS=0;		// Access EEPROM
	EECON1bits.RD=1;		// EE Read
	return EEDATA;				// W = EEDATA
	}

void EEscriviWord(SHORTPTR addr,WORD n) {			// usare void * ?

	EEscrivi_(addr++,n & 0xff);
	n >>= 8;
	EEscrivi_(addr,n & 0xff);
	}

WORD EEleggiWord(SHORTPTR addr) {			// usare void * ?
	WORD n;

	n=EEleggi(addr++);
	n <<= 8;
	n|=EEleggi(addr);

	return n;
	}


// -------------------------------------------------------------------------------------
//Delays W microseconds (includes movlw, call, and return) @ 48MHz
void Delay_uS(BYTE uSec) {

	// 3/4 di prologo...
	do {
		Delay1TCY();			// 1
		Delay1TCY();			// 1
		Delay1TCY();			// 1
		Delay1TCY();			// 1
		Nop();						// 1
		Nop();						// 1
		Nop();						// 1
		ClrWdt();						// 1; Clear the WDT
		} while(--uSec);		// 3
// dovrebbero essere 12...
  //return             ; 4

/*Delay		macro	Time				// fare una cosa simile...

  if (Time==1)
		goto	$ + 1		;2
		goto	$ + 1
		nop						;1
  		exitm
  endif

  if (Time==2)
		goto	$ + 1
		goto	$ + 1
		goto	$ + 1
		goto	$ + 1
		goto	$ + 1
  		exitm
  endif

	movlw	Time
	call	Delay_uS

	endm*/

	}




void Delay_S_(BYTE n) {				// circa n*100mSec

	do {
	  Delay_mS(100);
		} while(--n);
	}


void Delay_mS(BYTE n) {				// circa n ms

	do {
		Delay_uS(250);
		Delay_uS(250);
		Delay_uS(250);
		Delay_uS(250);
		} while(--n);
	}


/** EOF main.c *************************************************/


