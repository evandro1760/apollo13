#pragma config FOSC = INTOSCIO
#pragma config WDTE = OFF
#include <xc.h>
#include <pic.h>
#include <pic16f628a.h>
#define _XTAL_FREQ 4000000

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
    0b11011001 //E
};

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

        __delay_ms(5);

        // set next row
        PORTB <<= 1;
        if (PORTB == 0x00) {
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

        __delay_ms(5);

    }

}

int main() {

    CMCON |= 7; // Turn comparators off and enable pins for I/O    
    T1CON = 0b00000001; // set clock

    PORTB = 0; // All PORTB pins are cleared
    TRISB = 0b11110000;
    PORTB = 0b00000001;
    
    TRISA = 0;
    PORTA = 0;

    unsigned state = 0;
    char op = ' ';
    int result = 0;

    while (1) {
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
