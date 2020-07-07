#include <xc.h>
#include <stdint.h>
#include <pic18f45k50.h>
#include "conbits.h"



void spi_slave_init(void){
    
    SSP1CON1bits.SSPEN = 0;
    
    ANSELAbits.ANSA5 = 0;
    ANSELBbits.ANSB3 = 0;
    ANSELBbits.ANSB1 = 0;
    ANSELBbits.ANSB0 = 0;
    
    TRISAbits.RA5 = 1; //SS
    TRISBbits.RB0 = 1; //SDI
    TRISBbits.RB1 = 1; //CLK
    TRISBbits.RB3 = 0; //SDO
    
    SSP1STATbits.CKE = 1;
    SSP1CON1 &= 0x3F;
    SSP1CON1bits.CKP = 1;
    SSP1CON1bits.SSPM = 0x04;
    
    SSP1CON3bits.BOEN = 0;
    
    SSP1STATbits.SMP = 0;
    
    IPR2bits.BCL1IP = 1;
    PIE2bits.BCL1IE = 1;
    
    IPR1bits.SSP1IP = 1;
    PIE1bits.SSP1IE = 1;

    SSP1CON1bits.SSPEN = 1;
    
}



void main(void) {
    
    LATB = 0x00;
    TRISBbits.RB7 = 0;
    TRISBbits.RB6 = 0;
    
    spi_slave_init();
    
    
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    
    
    for(;;){
        LATBbits.LB7 = 1;
        LATBbits.LB6 = 0;
        __delay_ms(1000);
        LATBbits.LB7 = 0;
        LATBbits.LB6 = 1;
        __delay_ms(1000);
    }
    return;
}



void __interrupt() high_isr(void){
    INTCONbits.GIEH = 0;

    if(PIR2bits.BCLIF == 1)
    {
        
        SSP1CON1 &= 0x3F;
        PIR2bits.BCLIF = 0;
    } 
    if(PIR1bits.SSPIF){
        
        uint8_t var = SSP1BUF;
        if(var == 0x8F){
            SSP1BUF = 0xBE;
        }else{
            SSP1BUF = 0xEF;
        }//BEEF
        
        
        PIR1bits.SSPIF = 0;
    }
    
    INTCONbits.GIEH = 1;
}

void __interrupt(low_priority) low_isr(void){
    INTCONbits.GIEH = 0;

    INTCONbits.GIEH = 1;
}
