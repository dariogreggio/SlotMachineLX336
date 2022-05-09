/*********************************************************************
 *
 *                Microchip USB C18 Firmware Version 1.0
 *
 *********************************************************************
 * FileName:        io_cfg.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 2.30.01+
 * Company:         fuck Microchip Technology, Inc.
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Rawin Rojvanit       21/07/04    Original.
 * Dario Greggio        28/02/05    .
 ********************************************************************/

#ifndef IO_CFG_H
#define IO_CFG_H

/** I N C L U D E S *************************************************/

/** T R I S *********************************************************/
#define INPUT_PIN           1
#define OUTPUT_PIN          0



/** I N P U T / O U T P U T *****************************************************/
// port A
// display mux

// port B
// pulsanti

// port C



#define BeepBit			2
#define m_BeepBit		LATCCbits.LATC2

#define Puls1Bit			7
#define m_Puls1Bit	PORTCbits.RC7
#define Puls2Bit			6
#define m_Puls2Bit	PORTCbits.RC6

#define Led0Bit		5
#define m_Led0Bit		LATCbits.LATC5


#define I2CDataBit			4
#define m_I2CDataBit		LATCbits.LATC4
#define m_I2CDataBitI		PORTCbits.RC4
#define I2CClkBit				3
#define m_I2CClkBit			LATCbits.LATC3
#define m_I2CClkBitI		PORTCbits.RC3

#define I2CDataTris			TRISCbits.TRISC4
#define I2CClkTris			TRISCbits.TRISC3


// port D
//8 bit I/O display






#define Led0Val		(1 << Led0Bit)



#endif //IO_CFG_H
