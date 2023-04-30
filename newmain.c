/*
 * File:   newmain.c
 * Author: nemet
 *
 * Created on April 21, 2023, 6:51 PM
 */
// PIC16F18446 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FEXTOSC = HS     // External Oscillator mode selection bits (HS (crystal oscillator) above 4MHz; PFM set to high power)
#pragma config RSTOSC = EXT1X   // Power-up default value for COSC bits (EXTOSC operating per FEXTOSC bits)
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (FSCM timer enabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTS = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 2.45V)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = OFF    // Peripheral Pin Select one-way control (The PPSLOCK bit can be set and cleared repeatedly by software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config BBSIZE = BB512   // Boot Block Size Selection bits (512 words boot block size)
#pragma config BBEN = OFF       // Boot Block Enable bit (Boot Block disabled)
#pragma config SAFEN = OFF      // SAF Enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block Write Protection bit (Application Block not write protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration Register not write protected)
#pragma config WRTD = OFF       // Data EEPROM write protection bit (Data EEPROM NOT write protected)
#pragma config WRTSAF = OFF     // Storage Area Flash Write Protection bit (SAF not write protected)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (UserNVM code protection disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define _XTAL_FREQ 8000000
#include <xc.h>
#include <stdio.h>
#include<stdint.h>
#include <math.h>

void setNcoFreq(uint32_t freq) {
    uint32_t a, b, c;
    a = 0;
    b = 0;
    c = 0;
    uint32_t temp =  ( ( ( ( freq * 2LL ) *1048576LL ) ) / 125000000LL );
    a = (temp & 0xff) + 1;
    b = ((temp >> 8)&0xff);
    c = ((temp >> 16)&0xff);
    NCO1INCL = a;
    NCO1INCH = b;
    NCO1INCU = c;
}

void smt_init(void) {
    SMT1CLKbits.CSEL = 0b001;
    SMT1CON0bits.EN = 1;
    SMT1CON1bits.MODE = 0b0010; //capture mode
    SMT1CON1bits.REPEAT = 1;
    SMT1CON0bits.SMT1PS = 0;
    SMT1PR = 0xffffff;
    SMT1CON1bits.SMT1GO = 1;
    SMT1SIGPPS = 0b00010110; //RC6

}

void uart_init(void) {
    TX1STAbits.TXEN = 1;
    TX1STAbits.BRGH = 1;
    RC1STAbits.SPEN = 1;
    BAUD1CONbits.BRG16 = 1;
    SPBRGH = 0;
    SPBRGL = 207; //9600bps
    RC7PPS = 0b1111; //Uart to RC7


}
volatile char dataAvailable;
volatile long smt2Value;
void main(void) {
    RB7PPS = 0x18;
    TRISBbits.TRISB7 = 0;
    TRISAbits.TRISA4 = 1; //smt1 signal in(default)
    ANSELAbits.ANSA4 = 0; //analog enabled by default.....
    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC4 = 0;
    
    
    TRISCbits.TRISC6 = 1;
    ANSELCbits.ANSC6 = 0;
    
    
    CLKRCONbits.CLKREN = 1;
    CLKRCONbits.CLKRDIV = 0b110; //16

    NCO1CONbits.EN = 1;
    NCO1CLKbits.CKS = 0b0110;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIE8bits.SMT1PRAIE = 1;
    setNcoFreq(5000);
    smt_init();
    uart_init();
    while (1) {
        if(dataAvailable)
        {
           unsigned long temp = lround(1000/((0.000000125)*(SMT1CPR&0xffffff)));
            setNcoFreq(temp*50);  
            printf("%ld\n",temp);
        }

        
            
    }

    return;
}

void __interrupt() myIsr(void) {
    if (PIR8bits.SMT1PRAIF) {
        dataAvailable = 1;
        LATCbits.LATC4 = ~LATCbits.LATC4; //just for debug
        PIR8bits.SMT1PRAIF = 0;
    }
}

void putch(char c)//printf redirect
{
    TX1REG = c;
    while (TX1STAbits.TRMT != 1);
}