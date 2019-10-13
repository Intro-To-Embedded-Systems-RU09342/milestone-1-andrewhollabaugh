/*
 * Individually addressable RGB LED program for MSP430G2553
 *
 * Created: 10/12/19
 * Last Edited: 10/12/19
 * Author: Andrew Hollabaugh
 */

#include <msp430.h> //msp identifiers
#include <stdint.h>

//R - P1.6 - TA0CCR1
//G - P2.1 - TA1CCR1
//B - P2.5 - TA1CCR2

//use P1.6 (PWM pin) for red led; define generic names for gpio registers and gpio pin
#define LEDR_DIR_R P1DIR
#define LEDR_OUT_R P1OUT
#define LEDR_SEL_R P1SEL
#define LEDR_PIN 6

//use P2.1 (PWM pin) for green led; define generic names for gpio registers and gpio pin
#define LEDG_DIR_R P2DIR
#define LEDG_OUT_R P2OUT
#define LEDG_SEL_R P2SEL
#define LEDG_PIN 1

//use P2.5 (PWM pin) for blue led; define generic names for gpio registers and gpio pin
#define LEDB_DIR_R P2DIR
#define LEDB_OUT_R P2OUT
#define LEDB_SEL_R P2SEL
#define LEDB_PIN 5

//generic names for pwm duty cycle registers
#define DUTY_RED TA0CCR1
#define DUTY_GREEN TA1CCR1
#define DUTY_BLUE TA1CCR2

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; //stop watchdog timer

    //led setup
    LEDR_DIR_R |= (1 << LEDR_PIN); //set led direction to output
    LEDR_SEL_R |= (1 << LEDR_PIN); //set led pin to use pwm function
    LEDG_DIR_R |= (1 << LEDG_PIN); //set led direction to output
    LEDG_SEL_R |= (1 << LEDG_PIN); //set led pin to use pwm function
    LEDB_DIR_R |= (1 << LEDB_PIN); //set led direction to output
    LEDB_SEL_R |= (1 << LEDB_PIN); //set led pin to use pwm function

    //timer0 setup, no prescaling, used for red PWM
    TA0CTL |= TASSEL_2; //select SMCLK as clk src
    TA0CTL |= MC_1; //set to up mode
    TA0CCR0 = 255; //max pwm value
    DUTY_RED = 0; //red init
    TA0CCTL1 |= OUTMOD_7; //set pwm output mode to reset/set; resets output at CCR0, sets output at CCR1

    //timer1 setup, no prescaling, used for green and blue PWM
    TA1CTL |= TASSEL_2; //select SMCLK as clk src
    TA1CTL |= MC_1; //set to up mode
    TA1CCR0 = 255; //max pwm value
    DUTY_GREEN = 0; //grn init
    DUTY_BLUE = 0; //blu init
    TA1CCTL1 |= OUTMOD_7; //set pwm output mode to reset/set; resets output at CCR0, sets output at CCR1
    TA1CCTL2 |= OUTMOD_7; //set pwm output mode to reset/set; resets output at CCR0, sets output at CCR2

    //UART setup
    DCOCTL = 0; //select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ; //set DC0 to 1 MHz
    DCOCTL = CALDCO_1MHZ;
    P1SEL |= BIT1 + BIT2; //P1.1 = RXD, P1.2 = TXD
    P1SEL2 |= BIT1 + BIT2; //P1.1 = RXD, P1.2 = TXD
    UCA0CTL1 |= UCSSEL_2; //SMCLK
    UCA0BR0 = 104; //set baud to 9600 from 1 MHz
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0; //modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST; //initialize USCI state machine
    IE2 |= UCA0RXIE; //enable USCI_A0 RX interrupt

    __bis_SR_register(LPM0_bits + GIE); //enter low power mode and enable interrupts

    while(1); //loop infinitely
}

//sends a byte of data over UART
void send_byte(uint8_t byte)
{
    while(!(IFG2 & UCA0TXIFG)); //wait until USCI_A0 TX buffer is ready
    UCA0TXBUF = byte; //put byte in transmission buffer
}

volatile uint8_t byte = 0; //current byte being parsed
volatile uint8_t len = 0; //length of transmission
volatile uint8_t i = 0; //location of current byte in transmission

/*
 * UART RX ISR
 * run when a byte is received
 */

void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
{
    byte = UCA0RXBUF; //copy rx buffer to variable

    //do something with the byte based on where the byte is in the transmission
    switch(i)
    {
        case 0: //first byte; will contain length of transmission
            len = byte;
            break;
        case 1: //contains duty cycle for red led
            DUTY_RED = byte;
            break;
        case 2: //contains duty cycle for green led
            DUTY_GREEN = byte;
            break;
        case 3: //contains duty cycle for blue led
            DUTY_BLUE = byte;
            send_byte(len - 3); //send length header message to start the new transmission
            break;
        default:
            send_byte(UCA0RXBUF); //send byte that was received; occurs for all bytes after byte 3
            break;
    }

    //increment i after each new byte is processed; reset to 0 once it reaches the length of transmission
    if(i < len - 1)
        i++;
    else
        i = 0;
}

