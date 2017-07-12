// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSC oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB4/PGM pin has digital I/O function, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <pic.h>
#include <pic16f628a.h>
#define _XTAL_FREQ 4000000

// display lookup array
unsigned char lookup7segCa[] = {
    0b01011111, //0
    0b00000110, //1
    0b10011011, //2
    0b10001111, //3
    0b11000110, //4
    0b11001101, //5
    0b11011101, //6
    0b00000111, //7
    0b11011111, //8
    0b11001111, //9
    0b10000000, //-
    0b11011001  //E
};

// display a char to PORTA
void display(char n) {
    if (n >= '0' && n <= '9')
        PORTA = lookup7segCa[n - '0'];
    if (n == ' ')
        PORTA = 0;
    if (n == 'E')
        PORTA = lookup7segCa[11];
    if (n == '-')
        PORTA = lookup7segCa[10];
}

// get button pressed row number
int getRow() {
    int row = 0;
    if (PORTBbits.RB0) {
        row = 0;
    } else if (PORTBbits.RB1) {
        row = 1;
    } else if (PORTBbits.RB2) {
        row = 2;
    } else if (PORTBbits.RB3) {
        row = 3;
    }
    return row;
}

// read user input
char read_input() {

    char kb[4][4] = {
        {'7', '8', '9', '/'},
        {'4', '5', '6', '*'},
        {'1', '2', '3', '-'},
        {'0', 'c', '=', '+'}
    };

    while (1) {

        __delay_ms(10);

        // set next row
        PORTB <<= 1;
        if (!(PORTB & 0b00001111)) {
            PORTB = 0b00000001;
        }

        // read button press
        if (PORTBbits.RB4) {
            return kb[getRow()][0];
        } else if (PORTBbits.RB5) {
            return kb[getRow()][1];
        } else if (PORTBbits.RB6) {
            return kb[getRow()][2];
        } else if (PORTBbits.RB7) {
            return kb[getRow()][3];
        }

    }

}

unsigned PORTAcache;
unsigned cnt;                   // Define variable cnt

void interrupt clock() {        // Interrupt routine called evey 1ms
    cnt++;                      // Interrupt causes cnt to be incremented by 1
    TMR0 = 6;                   // Timer (or counter) TMR0 returns its initial value
    if (cnt == 1000) {
        PORTAcache = PORTA;
        PORTA = 0;              // Display off
    }
    if (cnt == 1500) {
        cnt = 0;
        PORTA = PORTAcache; //display on
    }

    INTCONbits.T0IF=0;          // Clean bit TMR0IF
}

void enable_interrupt() {
    cnt = 0;
    TMR0 = 6;                   // Overflow in 250 cicles (0.001s = 1ms)
    INTCONbits.T0IE = 1;        // Enable The TMR0 Overflow Interrupt
}

void disable_interrupt() {
    INTCONbits.T0IE = 0;        // Disable The TMR0 Overflow Interrupt
}

int main() {

    OPTION_REGbits.PS   = 0x3;          // Set TMR0 Rate to 1:16 (250Khz)
    OPTION_REGbits.T0CS = 0x0;          // Set source as internal clock
    OPTION_REGbits.PSA  = 0x0;

    INTCONbits.GIE = 1;                 // Enable The Global Interrupt

    CMCON |= 7;                         // Turn comparators off, use as I/O
    T1CON = 0b00000001;                 // set clock

    PORTB = 0;                          // All PORTB pins are cleared
    TRISB = 0b11110000;
    PORTB = 0b00000001;

    TRISA = 0;
    PORTA = 0;

    unsigned state = 0;
    char op = ' ';
    int error = 0;
    int result = 0;

    while (1) {
        char in = read_input();

        switch (state) {
            case 0:                             // wait for first number
                if (in >= '0' && in <= '9') {   // read a number
                    display(in);
                    error = 0;
                    result = in - '0';
                    state = 1;
                }
                break;
            case 1:                             // wait for operator
                if (in == 'c') {
                    display(' ');
                    state = 0;
                } else if (in == '+' || in == '-' || in == '*' || in == '/') {
                    display('-');
                    op = in;
                    state = 2;
                } else if (in == '=') {
                    if (error || result < 0 || result > 9) {
                        display('E');
                    }
                    else {
                        display(result + '0');
                    }
                    state = 3;
                    enable_interrupt();
                }
                break;
            case 2:                             // wait for number
                if (in >= '0' && in <= '9') {
                    display(in);
                    state = 1;
                    if(error) {
                        break;
                    }
                    switch (op) {
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
                            if (in == '0') error = 1;
                            else result /= (in - '0');
                    }
                } else if (in == '+' || in == '-' || in == '*' || in == '/') {
                    display('-');
                    op = in;
                } else if (in == 'c') {
                    display(' ');
                    state = 0;
                } else if (in == '=') {
                    if (error || result < 0 || result > 9) {
                        display('E');
                    }
                    else {
                        display(result + '0');
                    }
                    state = 3;
                    enable_interrupt();
                }
                break;
            case 3:                             // blink the result
                if (in == 'c') {
                    disable_interrupt();
                    display(' ');
                    state = 0;
                } else if (in == '+' || in == '-' || in == '*' || in == '/') {
                    disable_interrupt();
                    display('-');
                    op = in;
                    state = 2;
                }
                break;
        }
    }

    return 0;
}
