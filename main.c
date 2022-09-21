/* 
 * File:   main.c
 * Author: Diego RP
 *
 * ADC calculator
 */

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "LiquidCrystal.h"
# define bit0 PORTDbits.RD3 // defining inputs for readibility
# define bit1 PORTDbits.RD4

# define _XTAL_FREQ 1000000
void __interrupt() adc_sample(void);    // prototype function

volatile int num1,num2,sum,sub; // our adc inputs, with respective result of operation 
volatile long mult, x, y;       // used to perform a calculation for multiplication
                                // XC8 has 16-bit integers, in this case, we reach up to 18-bit

int main() 
{
    
    // Data pins connected to PORTC
    TRISC = 0x00; 
    
    // RS = RD0
    // RW = RD1
    // E  = RD2
    TRISD = 0x18; // 0001 1000 (RD3 and RD4 are inputs)
    
    // connect the LCD pins to the appropriate PORT pins
    pin_setup(&PORTC, &PORTD);
    
    // initialize the LCD to be 16x2
    begin(16, 2, LCD_5x8DOTS);
    
    
    //--------------------------------------------------------------------------
    // 1 - Configure the A/D Module

    // * Configure analog pins, voltage reference and digital I/O 
    // Reference voltages are VSS and VDD
    ADCON1 = 0x0D;
    TRISAbits.RA0 = 1; // connected to potentiometer
    TRISAbits.RA1 = 1; // connected to photo sensor

    // * Select A/D acquisition time
    // * Select A/D conversion clock
    // Right justified, ACQT = 2 TAD, ADCS = FOSC/2
    ADCON2bits.ADCS = 0; // FOSC/2
    ADCON2bits.ACQT = 1; // ACQT = 2 TAD
    ADCON2bits.ADFM = 1; // Right justified

    // * Select A/D input channel
    ADCON0bits.CHS = 0; // Channel 0 (AN0)

    // * Turn on A/D module
    ADCON0bits.ADON = 1;   
    
    // 2 - Configure A/D interrupt (if desired)
    // * Clear ADIF bit
    // * Set ADIE bit
    // * Select interrupt priority ADIP bit
    // * Set GIE bit
    
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
    IPR1bits.ADIP = 1;
    RCONbits.IPEN = 0;   // disable priority levels
    INTCONbits.PEIE = 1; // enable peripheral interrupts
    INTCONbits.GIE = 1;
      
    while(1)
    {
        // Set GO/DONE(bar) bit
        ADCON0bits.GO = 1;   
        
        if(bit0==0 && bit1==0)              // clear when switches are low
        {
            clear();
        }
        
            
        else if (bit0==0 && bit1==1)        // add when 01
        {
            print("(");
            print_int(num1);
            print(")+(");
            print_int(num2);                // use the print_int function
            print(")  ");    

            setCursor(13,0);                // positioning the = sing
            print("=");

            setCursor(0, 1);                // result in the lower row
            print_int(sum);
            print("       ");
           
        }
                                            // Repeat process
        else if (bit0==1 && bit1==0)        // subtract when 10
        {
            print("(");
            print_int(num1);
            print(")-(");
            print_int(num2);
            print(")  ");  
            
            setCursor(13,0);
            print("=");
            
            setCursor(0, 1);
            print_int(sub);
            print("       ");
            
            }
            
        else if (bit0==1 && bit1==1)
        {
            print("(");
            print_int(num1);
            print(")x(");
            print_int(num2);
            print(")  ");  
            
            setCursor(13,0);
            print("=");
            
            setCursor(0, 1);
            print_long(mult);           // use our function to print a long
            print("     ");
           
            }
        
        home();                         // return cursor to home
    }
    return 0;
}

void __interrupt() adc_sample(void)
{
    // ADC Interrupt
    if (PIR1bits.ADIF && PIE1bits.ADIE)
    {
        // 5 Wait for A/D conversion to complete by either
        // * Polling for the GO/Done bit to be cleared
        // * Waiting for the A/D interrupt
  
        // 6 - Read A/D result registers (ADRESH:ADRESL); clear bit ADIF, if required
        
        // reset the flag to avoid recursive interrupt calls
        PIR1bits.ADIF = 0;
        
        if (ADCON0bits.CHS == 0) // channel AN0 (potentiometer 1)
        {
            num1 = (ADRESH << 8) | ADRESL;  // read and define with right justification
            num1 = num1 - 511;              // subtract half of 511 to obtain a signed value 2^9 - 1
            ADCON0bits.CHS = 1;
        }
        else if (ADCON0bits.CHS == 1) // channel AN1 (potentiometer 2)
        {
            num2 = (ADRESH << 8) | ADRESL;
            num2 = num2 - 511;
            ADCON0bits.CHS = 0;
        }
        x = num1;                     // define longs
        y = num2; 
        sum = num1+num2;              // define operations
        sub = num1-num2;
        mult = x*y;
    }
}