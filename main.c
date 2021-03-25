/*******************************************************************************
*                       MSP432 Notch Filter                                    *
*                                                                              *
* Author:  Long Tran                                                           *
* Device:  MSP432P401R LaunchPad                                               *
* Program: Eliminate unwanted frequency (60 Hz)                                *
*                                                                              *
* Demo: https://www.youtube.com/watch?v=BwOXYYQE5To                            *
*******************************************************************************/

// Include header files
#include "msp.h"

// Define global constants
#define CCR_0 375       // 750kHz/2kHz = 375
#define B2 1.0          // transfer function global constants
#define B1 -1.965       // y = -a1*y1-a0*y2+b2*u+b1*u1+b0*u2  
#define B0 0.9999
#define A2 1.0
#define A1 -1.955
#define A0 0.99

// Define prototypes
void init_clock(void);
void init_ADC(void);
uint16_t get_ADC_conversion_result(void);
void convert_ADC_result_to_Vin(uint16_t ADC_result);
void init_DAC(void);
void init_NVIC(void);
void init_Timer_A1(void);

void output_to_DAC(uint16_t result);

// Define global variables
float u0 = 0.0, u1 = 0.0, u2 = 0.0, y0 = 0.0, y1 = 0.0, y2 = 0.0;

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

    init_clock();
    init_ADC();
    init_DAC();
    init_NVIC();
    init_Timer_A1();

    while(1){
        // empty loop
    }
}

void init_clock(void){

    // Set power mode
    while((PCM->CTL1 & PCM_CTL1_PMR_BUSY));
    PCM->CTL0 = PCM_CTL0_KEY_VAL | PCM_CTL0_AMR__AM_LDO_VCORE1;  // AM_LDO_VCORE1 (required for 48 MHz clock)
    while((PCM->CTL1 & PCM_CTL1_PMR_BUSY));
    PCM->CTL0 &= 0x0000FFFF;    // lock PCM

    // Set flash read wait state number
    FLCTL->BANK0_RDCTL |= FLCTL_BANK0_RDCTL_WAIT_1; // bank0 wait state to 1
    FLCTL->BANK1_RDCTL |= FLCTL_BANK1_RDCTL_WAIT_1; // bank1 wait state to 1

    // Enable 48 MHz DCO
    CS->KEY = CS_KEY_VAL;           // unlock clock system registers (MSP432 Ref. Manual, pg.394)
    CS->CTL0 |= CS_CTL0_DCORSEL_5 | // DCO frequency range to 48 MHz
                CS_CTL0_DCOEN;      // enable DCO oscillator

    // Select DCO as the source for MCLK
    CS->CTL1 |= CS_CTL1_SELS__DCOCLK;  // MCLK source: DCOCLK
    CS->CLKEN |= CS_CLKEN_MCLK_EN;     // enable MCLK

    /* TESTING*/
    CS->CTL1 |= CS_CTL1_DIVS__64; // SMCLK runs at 75kHz
    /* TESTING*/

    // Lock clock system
    CS->KEY =0x0;

}

void init_NVIC(void){
    NVIC->ISER[0] = 1 << ((TA1_0_IRQn) & 31); // enable Timer_A1 interrupt
}

void init_Timer_A1(void){

    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // clear CC interrupt flag
    TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_CCIE;   // enable CC interrupt
    TIMER_A1->CCR[0] = CCR_0;   // CC register value

    TIMER_A1->CTL |= TIMER_A_CTL_SSEL__SMCLK | // SMCLK as clock source
                     TIMER_A_CTL_MC__UP;       // up mode

}

void TA1_0_IRQHandler(void){

    u0 = get_ADC_conversion_result();
    u0 = u0 * 4096/16384;   // map 14bit ADC to 12 bit DAC
    y0 = (-A1*y1) - (A0*y2) + (B2*u0) + (B1*u1) + (B0*u2);


    output_to_DAC((unsigned int) y0);

    y2 = y1;
    y1 = y0;
    u2 = u1;
    u1 = u0;

    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;  // clear interrupt flag
}

void init_ADC(void){

    // Unlock and disable ADC
    ADC14->CTL0 &= !ADC14_CTL0_ENC;

    // Configure ADC
    ADC14->CTL0 |= ADC14_CTL0_PDIV__4    | // predivider to 4
                   ADC14_CTL0_SHS_0      | // sample-and-hold source select to ADC14SC bit
                   ADC14_CTL0_SHP        | // SAMPCON signal source to sampling timer
                   ADC14_CTL0_DIV__8     | // divider to 8
                   ADC14_CTL0_SSEL__MCLK | // MCLK as clock source
                   ADC14_CTL0_SHT0__32   | // sampling period to 32 ADC14CLK cycles for MEM0-7 and MEM14-31
                   ADC14_CTL0_ON;          // turn ADC on

    ADC14->CTL1 |= ADC14_CTL1_RES__14BIT;  // resolution to 14 bit

    ADC14->MCTL[0] |= ADC14_MCTLN_INCH_15; // input channel A15

    // Lock and enable ADC
    ADC14->CTL0 |= ADC14_CTL0_ENC;

    // Set P6.0 as ADC analog input A15
    P6->DIR  &= !BIT0;
    P6->SELC |=  BIT1; // (MSP432P401R Texas Instruments data sheet, pg. 153)

}

uint16_t get_ADC_conversion_result(void){

    // Start ADC14 conversion
    ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;

    // Wait for ADC14 to finish conversion operation
    while(ADC14->CTL0 & ADC14_CTL0_BUSY)

    // Read conversion results from ADC14MEM0
    return ADC14->MEM[0];

}

void output_to_DAC(uint16_t result){// result is 12-bit

    // To output analog voltage, output to 12 bit DBx, select channel A or B
    // toggle /WR pin
    P10->OUT |= (BIT5);        // disable write to DAC
    P10->OUT = result >> 8;           // DB11-8, write to P10.3-0
    P7->OUT  = (result << 8) >> 8;    // DB7-0,  write to P7.7-0
    P10->OUT &= ~(BIT5);       // enable write to DAC

}

void init_DAC(void){

    // External AD7247 DAC Documentation: https://www.analog.com/media/en/technical-documentation/data-sheets/AD7237A_7247A.pdf
    P10->DIR |= 0x0F;    // DB11-8, most significant 4-bit latch
    P7->DIR  |= 0xFF;    // DB7-0 , least significant 8-bit latch
    P10->DIR |= BIT4 | BIT5;   // /CSA and /WR  pins
    P10->OUT &= ~BIT4;         // select /CSA
    P6->OUT  |= BIT3;          // disable /CSB
    
}
