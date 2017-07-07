
// CONFIG
#pragma config FOSC = EXTRCCLK  // Oscillator Selection bits (RC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, Resistor and Capacitor on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable bit (RB4/PGM pin has PGM function, low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

#define _XTAL_FREQ 4000000

int getRow() {
    int row = 0;
    if (PORTBbits.RB0) {
        row = 0;
    }
    else if (PORTBbits.RB1) {
        row = 1;
    }
    else if (PORTBbits.RB2) {
        row = 2;
    }
    else if (PORTBbits.RB3) {
        row = 3;
    }
    return row;
}

// read user input
char read_input() {
    
    char kb[4][4] = {
                {'7','8','9','/'},
                {'4','5','6','*'},
                {'1','2','3','-'},
                {'0','c','=','+'}
            };
    
    while(1){
        
        __delay_ms(5);
        
        // set next row
        PORTB <<= 1;
        if(PORTB == 0x00) {
            PORTB = 0b00000001;
        }
        
        // read button press
        if (PORTBbits.RB4) {
            return kb[getRow()][0];
        }
        else if (PORTBbits.RB5) {
            return kb[getRow()][1];
        }
        else if (PORTBbits.RB6) {
            return kb[getRow()][2];
        }
        else if (PORTBbits.RB7) {
            return kb[getRow()][3];
        }
        
        __delay_ms(5);
        
    }
    
}

int main() {
    
    CMCON |= 7;             // Turn comparators off and enable pins for I/O    
    T1CON = 0b00000001;     // set clock

    PORTB = 0;              // All PORTB pins are cleared
    TRISB = 0b11110000;
    PORTB = 0b00000001;
    
    unsigned state = 0;
    char op = ' ';
    int result = 0;
    
    while(1) {
        char in = read_input();
    
        switch (state) {
        case 0: // wait for first number
            if (in >= '0' && in <= '9') { // read a number
                display(in);
                result = in - '0';
                state = 1;
            }
            break;
        case 1: // wait for operator
            if (in == 'c') {
                display(' ');
                state = 0;
            } else if (in == '+' || in == '-' || in == '*' || in == '/') {
                display('-');
                op = in;
                state = 2;
            } else if (in == '=') {
                display(result + '0');
                state = 3;
            }
            break;
        case 2: // wait for number
            if (in >= '0' && in <= '9') {
                display(in);
                switch (op) {
                    case 'E': // is a error, do nothing
                        break;
                    case '+':
                        result += (in - '0');
                        break;
                    case '-':
                        result -= (in - '0');
                        break;
                    case '*':
                        result *= (in - '0');
                        break;
                    case '/':
                        if (in == '0') op = 'E';
                        else result /= (in - '0');
                }
                state = 1;
            } else if (in == '+' || in == '-' || in == '*' || in == '/') {
                display('-');
                op = in;
            } else if (in == 'c') {
                display(' ');
                state = 0;
            } else if (in == '=') {
                display(result + '0');
                state = 3;
            }
            break;
        case 3: // blinck the result
            if (in == 'c') {
                display(' ');
                state = 0;
            } else if (in == '+' || in == '-' || in == '*' || in == '/') {
                display('-');
                op = in;
                state = 2;
            }
            break;
        }
    }
    
    return 0;
}
