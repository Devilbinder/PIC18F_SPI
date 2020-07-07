#include <xc.h>
#include <pic18f4520.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "conbits.h"
#include "uart_layer.h"

#define SPI_DIR_SDO TRISCbits.RC5
#define SPI_DIR_CLK TRISCbits.RC3
#define SPI_DIR_SDI TRISCbits.RC4
#define SPI_DIR_CS  TRISEbits.RE2

#define SPI_CS LATEbits.LE2


volatile bool spi_rx_data_ready = false;

const uint8_t program_start[18]="\r\nProgram start\n\r";
uint8_t print_buffer[100] = {0}; // buffer to print stuff to serial

volatile uint8_t uart_char = 0;
volatile bool uart_rcv_data = false;

void spi_write(uint8_t data_in, uint8_t * data_out){
    SSPBUF = data_in;
    while(spi_rx_data_ready == false){
        Nop();
    }
    spi_rx_data_ready = false;
    *data_out = SSPBUF;
}

void spi_transfer(uint8_t * tx_data,uint16_t tx_size, uint8_t *rx_data, uint16_t rx_size){
    uint16_t i = 0; 
    for(i = 0; i < tx_size; i++){
        spi_write(tx_data[i],&rx_data[i]);
    }
}


void spi_master_init(void){
    SSPCON1bits.SSPEN = 0;
    ADCON1bits.PCFG = 0x0F;
    
    SPI_DIR_SDO = 0;
    SPI_DIR_SDI = 1;
    SPI_DIR_CLK = 0;
    SPI_DIR_CS = 0;
    
    SPI_CS = 1;
    
    SSPSTATbits.SMP = 0;
    SSPSTATbits.CKE = 1;
    
    SSPCON1bits.CKP = 1;
    SSPCON1bits.SSPM = 0x02;
    
    SSPCON1bits.SSPEN = 1;
    
    PIE2bits.BCLIE = 1;
    IPR2bits.BCLIP = 1;
    
    PIE1bits.SSPIE = 1;
    IPR1bits.SSPIP = 1;
}

void main(void){
    
    uint8_t tx_data[2] = {0};
    uint8_t rx_data[2] = {0};

    OSCCONbits.IDLEN = 0;
    OSCCONbits.IRCF = 0x07;//0x01;//0x07;
    OSCCONbits.SCS = 0x03;
    while(OSCCONbits.IOFS!=1); // 8Mhz

    TRISB = 0x00;    
    LATB = 0x00;

    uart_init(51,0,1,0);//baud 9600
    spi_master_init();
    

    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;// base interrupt setup
    
    __delay_ms(2000);
    uart_send_string((uint8_t *)program_start); // everything works in setup
    
    for(;;){ // while(1)
        
        memset(tx_data,0,sizeof(tx_data));
        memset(rx_data,0,sizeof(rx_data));
        
        tx_data[0] = 0x8F;
        
        SPI_CS = 0;
        spi_transfer(tx_data,sizeof(tx_data),rx_data,sizeof(rx_data));
        SPI_CS = 1;
        
        sprintf((char *)print_buffer,"tx : 0x%02x 0x%02x | rx: 0x%02x 0x%02x\r",
                tx_data[0],
                tx_data[1],
                rx_data[0],
                rx_data[1]);
        uart_send_string(print_buffer);

        if(uart_rcv_data){
            uart_send(uart_char);
            uart_rcv_data = false;
        }
    } 
}



void __interrupt() high_isr(void){
    INTCONbits.GIEH = 0;

    if(PIR2bits.BCLIF == 1)
    {
        SSPCON1 &= 0x3F;
        PIR2bits.BCLIF = 0;
    } 
    if(PIR1bits.SSPIF){
        
        spi_rx_data_ready = true;
        PIR1bits.SSPIF = 0;
    }
    
    if(PIR1bits.RCIF){
        uart_receiver(&uart_char,&uart_rcv_data);
        PIR1bits.RCIF=0;
    }
    
    INTCONbits.GIEH = 1;
}

void __interrupt(low_priority) low_isr(void){
    INTCONbits.GIEH = 0;

    if(0){

    }

    INTCONbits.GIEH = 1;
}

