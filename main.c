/* Authors: Nick Scamardi, Eric Schroeder & Nick Setaro
 * Written: December 5, 2018
 * Last Update: December 16, 2018
 */

#include <intrinsics.h>
#include <stdint.h>
#include <msp430.h>

#define TRIGGER_PIN BIT1   // P6.1
#define ECHO_PIN BIT3  // P1.3
#define LED_PIN BIT0   // P1.0
#define MEASURE_INTERVAL 2048  // ~250 ms

//distance variables
    uint32_t D_1 = 10;
    uint32_t D_2 = 20;
    uint32_t D_3 = 30;
    uint32_t D_4 = 40;
    uint32_t D_5 = 50;
    int byte = 0;

void triggerMeasurement() {
     // Start trigger
     P6OUT |= TRIGGER_PIN;

     // Wait a small amount of time with trigger high, > 10us required (~10 clock cycles at 1MHz MCLK)
     __delay_cycles(10);

     // End trigger
     P6OUT &= ~TRIGGER_PIN;
}
 int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   //disable watchdog timer



    // Configure trigger pin, low to start
    P6DIR |= TRIGGER_PIN;
    P6OUT &= ~TRIGGER_PIN;

    // Configure LEDs, off to start
    P1DIR |= BIT2;      // Red LED
    P1OUT &= ~BIT2;     // Red LED
    P1DIR |= BIT4;      // Yellow LED
    P1OUT &= ~BIT4;     // Yellow LED
    P1DIR |= BIT5;      // Green LED
    P1OUT &= ~BIT5;     // Green LED
    P1DIR |= BIT6;      // Blue LED
    P1OUT &= ~BIT6;      // Blue LED
    P2DIR |= BIT0;      // White LED
    P2OUT &= ~BIT0;     // White LED

    // Configure echo pin as capture input to TA0CCR2
    P1DIR &= ~ECHO_PIN;
    P1SEL |= ECHO_PIN;

    // Set up TA0 to capture in CCR2 on both edges from P1.3 (echo pin)
    TA0CCTL2 = CM_3 | CCIS_0 | SCS | CAP | CCIE;

    // Set up TA0 to compare CCR0 (measure interval)
    TA0CCR0 = MEASURE_INTERVAL;
    TA0CCTL0 = CCIE;

    // Set up TA0 with ACLK / 4 = 8192 Hz
    TA0CTL = TASSEL__ACLK | ID__4 | MC__CONTINUOUS | TACLR;
    int16_t lastCount = 0;
    int32_t distance = 0;

    //UART setup
    P4SEL |= BIT4 | BIT5;                                       // Pin4.4 set as TXD output,  Pin4.5 set as RXD input

    UCA1CTL1 |= UCSWRST;                                        // State Machine Reset + Small Clock Initialization
    UCA1CTL1 |= UCSSEL_1;                                       // Sets USCI Clock Source to SMCLK (32kHz)
    UCA1BR0 = 0x03;                                             // Setting the Baud Rate to be 9600
    UCA1BR1 = 0x00;                                             // Setting the Baud Rate to be 9600
    UCA1MCTL = UCBRS_3+UCBRF_0;                                 // Modulation UCBRSx=3, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;                                       // Initialize USCI State Machine
    UCA1IE |= UCRXIE;
    UCA1CTL1 |= UCSWRST;                                        // State Machine Reset + Small Clock Initialization
    UCA1CTL1 |= UCSSEL_1;                                       // Sets USCI Clock Source to SMCLK (32kHz)
    UCA1BR0 = 0x03;                                             // Setting the Baud Rate to be 9600
    UCA1BR1 = 0x00;                                             // Setting the Baud Rate to be 9600
    UCA1MCTL = UCBRS_3+UCBRF_0;                                 // Modulation UCBRSx=3, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;                                       // Initialize USCI State Machine
    UCA1IE |= UCRXIE;

    for(;;)
    {
         triggerMeasurement();

        // Wait for echo start
        __low_power_mode_3();

         lastCount = TA0CCR2;

        // Wait for echo end
        __low_power_mode_3();

        distance = TA0CCR2 - lastCount;
        distance *= 34000;
        distance >>= 14;  // division by 16384 (2 ^ 14)

        if (distance <= D_1)
        {
            // Turn on LED Red, Yellow, Green, Blue, White
            P1OUT |= BIT2;
            P1OUT |= BIT4;
            P1OUT |= BIT5;
            P1OUT |= BIT6;
            P2OUT |= BIT0;

        } else if (distance <= D_2)
        {
            //Turn on LED Yellow, Green, Blue, White
            P1OUT &= ~BIT2;
            P1OUT |= BIT4;
            P1OUT |= BIT5;
            P1OUT |= BIT6;
            P2OUT |= BIT0;

        }else if (distance <= D_3)
        {
            //Turn on LED Green, Blue, White
            P1OUT &= ~BIT2;
            P1OUT &= ~BIT4;
            P1OUT |= BIT5;
            P1OUT |= BIT6;
            P2OUT |= BIT0;

        }else if (distance <= D_4) {
            //Turn on LED Blue
            P1OUT &= ~BIT2;
            P1OUT &= ~BIT4;
            P1OUT &= ~BIT5;
            P1OUT |= BIT6;
            P2OUT |= BIT0;

        }else if (distance <= D_5) {
            //Turn on LED White
            P1OUT &= ~BIT2;
            P1OUT &= ~BIT4;
            P1OUT &= ~BIT5;
            P1OUT &= ~BIT6;
            P2OUT |= BIT0;
        }
        else
        {
                   // Turn off LEDs
                   P1OUT &= ~BIT2;
                   P1OUT &= ~BIT4;
                   P1OUT &= ~BIT5;
                   P1OUT &= ~BIT6;
                   P2OUT &= ~BIT0;
         }
        if (distance > 0 || distance > 200) {
            UCA1TXBUF = distance;
            // Wait for the next measure interval tick
            __low_power_mode_3();
        }

     }
 }

#pragma vector = TIMER0_A0_VECTOR
 __interrupt void TIMER0_A0_ISR (void) {
     // Measure interval tick
     __low_power_mode_off_on_exit();
     TA0CCR0 += MEASURE_INTERVAL;
 }

#pragma vector = TIMER0_A1_VECTOR
 __interrupt void TIMER0_A1_ISR (void) {
     // Echo pin state toggled
     __low_power_mode_off_on_exit();
     TA0IV = 0;
 }

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch(byte)
    {
    case 0:
            D_1 = UCA1RXBUF;
            byte += 1;  // Increment the byte variable by 1
            break;
    case 1:
            D_2 = UCA1RXBUF;
            byte += 1;  // Increment the byte variable by 1
            break;
    case 2:
            D_3 = UCA1RXBUF;
            byte += 1;  // Increment the byte variable by 1
            break;
    case 3:
            D_4 = UCA1RXBUF;
            byte += 1;  // Increment the byte variable by 1
            break;
    case 4:
            D_5 = UCA1RXBUF;
            byte = 0;
            break;
    default:
            break;
    }

}
