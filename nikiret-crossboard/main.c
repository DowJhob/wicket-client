#include <xc.h>
#include <stdint.h>

#include <string.h>

#include "onewire.h"

#pragma config IOL1WAY = ON // Peripheral Pin Select Configuration (Allow only one reconfiguration)
#pragma config POSCMOD = HS // Primary Oscillator Configuration (Primary osc disabled)
#pragma config FNOSC = PRIPLL // Oscillator Selection Bits (Fast RC Osc w/Div-by-N (FRCDIV))
#pragma config OSCIOFNC = ON // Secondary Oscillator Enable (Enabled)
#pragma config FCKSM = CSDCMD // Internal/External Switch Over (Enabled)
#pragma config ICS = PGx1
#pragma config JTAGEN = OFF
#pragma config IESO = OFF
#pragma config FWDTEN = OFF
#pragma config FWPSA = PR128
#pragma config WDTPS = PS2048

#define baudRate 57600
#define debounce 625

#define OFF          0
#define ON           1

unsigned char pred_PIN5_PASSED;
unsigned char pred_PIN4_RDY;
unsigned char pred_PIN6_ERROR;

#define PIN1_ENTRY             LATCbits.LATC0       // pin latch for LED
#define PIN2_EXIT              LATCbits.LATC1       // pin latch for LED
#define PIN3_LOCK_UNLOCK       LATCbits.LATC2       // pin latch for LED
#define PIN5_PASSED            PORTCbits.RC7       // pin latch for LED
#define PIN4_RDY               PORTCbits.RC6       // pin latch for LED
#define PIN6_ERROR             PORTCbits.RC8       // pin latch for LED

#define CN_PIN5_PASSED         CNPU2bits.CN17PUE
#define CN_PIN4_RDY            CNPU2bits.CN18PUE
#define CN_PIN6_ERROR          CNPU2bits.CN20PUE

#define CNEN_PIN5_PASSED       CNEN2bits.CN17IE
#define CNEN_PIN4_RDY          CNEN2bits.CN18IE
#define CNEN_PIN6_ERROR        CNEN2bits.CN20IE

#define LIGHT_                 LATBbits.LATB4       // pin latch for LED
#define PHER_                  LATBbits.LATB12       // pin latch for LED
#define RED_                   LATBbits.LATB14       // pin latch for LED
#define GREEN_                 LATBbits.LATB15       // pin latch for LED
#define TION_POWER_            LATCbits.LATC9      // pin latch for LED
#define HEATER_                LATBbits.LATB7

 char state_buf[97] =
        "ALARM 0\r"
        "TEMP 25.0\r"
        "PIN4 0\r"
        "PIN5 0\r"
        "PIN6 0\r"
        "HEATER DISABLED\r"
        "PHER ENABLED\r"
        "VER 2.35\r"
        "HARDRST 0\r";

unsigned int rx_pos = 0;
 char buf_rx_uart[256];
char Ready[7] = "Ready\r\n";
char PIN[7] = "PIN\0 x\r";
 char LIGHT[6] = "LIGHT\0";
 char PHER[5] = "PHER\0";
 char RED[4] = "RED\0";
 char GREEN[6] = "GREEN\0";
 char TL[3] = "TL\0";
 char STATE[6] = "STATE\0";
 char _OFF[4] = "OFF\0";
 char _ON[3] = "ON\0";

 char *buf_tx_uart;
unsigned int rx_count = 0;
unsigned int tx_lenght = 0; //?????? ?????? ?? ????????
unsigned int tx_count = 256; //??????? ??? ???????? (?????? ???? ??????????!)

void UART_Init(void) {
    // Unlock Registers
    __builtin_write_OSCCONL(OSCCON & 0xBF);
    // Configure Input Functions (Table 10-2))
    // Assign U1RX To Pin RP6
    RPINR18bits.U1RXR = 6;
    // Configure Output Functions (Table 10-3)
    // Assign U1TX To Pin RP5
    RPOR2bits.RP5R = 3;
    // Lock Registers
    __builtin_write_OSCCONL(OSCCON | 0x40);
    U1MODE = 0;
    //????????? UART1
    IPC3bits.U1TXIP = 4; //Set Uart TX Interrupt Priority
    IPC2bits.U1RXIP = 4; //Set Uart RX Interrupt Priority
    U1STA = 0;
    U1STAbits.URXISEL1 = 0;
    U1STAbits.URXISEL0 = 0; // UARTx Receive Interrupt Mode Selection bits
    //11 = Interrupt is set on RSR transfer, making the receive buffer full (i.e., has 4 data characters)
    //10 = Interrupt is set on RSR transfer, making the receive buffer 3/4 full (i.e., has 3 data characters)
    //0x = Interrupt is set when any character is received and transferred from the RSR to the receive buffer; receive buffer has one or more characters
    U1STAbits.UTXISEL1 = 0;
    U1STAbits.UTXISEL0 = 1; // UARTx Transmission Interrupt Mode Selection bits
    //11 = Reserved; do not use
    //10 = Interrupt when a character is transferred to the Transmit Shift Register (TSR) and as a result, the transmit buffer becomes empty
    //01 = Interrupt when the last character is shifted out of the Transmit Shift Register; all transmit operations are completed
    //00 = Interrupt when a character is transferred to the Transmit Shift Register (this implies there is at leastone character open in the transmit buffer)
    U1MODEbits.RXINV = 0;
    U1STAbits.UTXEN = 1; //Enable Transmit
    //IFS0bits.U1TXIF = 0;
    IEC0bits.U1TXIE = 1; //Enable Transmit Interrupt
    //IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1; //Enable Receive Interrupt
    //U1STAbits.UTXEN = 1;//Enable the interrupt system
    U1MODE = 0x8000; //Enable Uart for 8-bit data
    U1MODEbits.BRGH = 1;
    //no parity, 1 STOP bit
    //PLIB_USART_BaudRateHighSet(USART_ID_1, SYS_DEVCON_SYSTEM_CLOCK, baudRate); // calculation error will always round down value of UxBRG
    //unsigned int b = (FCY / ( 2 * baudRate)) - 1; // b = 2 * required UxBRG value
    //U1BRG = (b >> 1) + (b & 1); // UxBRG = b / 2 with rounding

    U1BRG = (FCY / ( 4 * baudRate)) - 1;

}

char state_nandle( char *pos) {
    if ( pos[1] == 'O' && pos[2] == 'N')
        return  ON;
    if ( pos[1] == 'O' && pos[2] == 'F')
        return  OFF;
    return  2;
}

void start_TX(char *buf, unsigned int lenght) {
    while (!U1STAbits.TRMT) {
    }
    tx_lenght = lenght;
    buf_tx_uart = buf;
    U1TXREG = buf_tx_uart[0];
    tx_count = 1;
    //IFS0bits.U1TXIF = 0;
}

void state_response() {
    state_buf[23] = (~PIN4_RDY&0x01) + 0x30;
    state_buf[30] = (~PIN5_PASSED&0x01) + 0x30;
    state_buf[37] = (~PIN6_ERROR&0x01) + 0x30;
    start_TX(state_buf, 97);
}

void recieve_handler() {
    int temp_state = OFF;
    char *s;
    if ( (s  = strstr(buf_rx_uart, PHER)) != NULL) {
        if ( (temp_state = state_nandle(s+4)) < 2)
        PHER_ = temp_state;
        //return;
    }
    PIN[3] = 0;
    if ( (s = strstr(buf_rx_uart, PIN)) != NULL) {
        if ( (temp_state = state_nandle(s+4)) < 2){
        if (s[3] == '1')
            PIN1_ENTRY = temp_state;
        if (s[3] == '2')
            PIN2_EXIT = temp_state;
        if (s[3] == '3')
            PIN3_LOCK_UNLOCK = temp_state;
        //return;
    }
    }

    if ((s = strstr(buf_rx_uart, LIGHT)) != NULL) {
        if ( (temp_state = state_nandle(s+5)) < 2)
            LIGHT_ = temp_state;
        //return;
    }
    if ((s = strstr(buf_rx_uart, RED)) != NULL) {
        RED_ = ON;
        //return;
    }
    if ((s = strstr(buf_rx_uart, GREEN)) != NULL) {
        GREEN_ = ON;
        //return;
    }
    if ((s = strstr(buf_rx_uart, TL)) != NULL) {
        RED_ = OFF;
        GREEN_ = OFF;
        //return;
    }
    if ((s = strstr(buf_rx_uart, STATE)) != NULL) {
        state_response();
        //return;
    }

}

void __attribute__((interrupt, auto_psv))_U1RXInterrupt(void) {
    U1STAbits.OERR = 0;
    IFS0bits.U1RXIF = 0;
    //while (U1STAbits.URXDA)
    {
        buf_rx_uart[rx_pos] = U1RXREG;
        if (buf_rx_uart[rx_pos] == 0)
        {
                rx_pos = 0;
                return;
        }
        if (buf_rx_uart[rx_pos] == '\r') {
            buf_rx_uart[rx_pos] = 0;
            recieve_handler();
            rx_pos = 0;
            return;
        }
        rx_pos++;
        if (rx_pos >= 256) {
            rx_pos = 0;
            //    recieve_handler();///?????????
            // bufferfull=1;
            //break; // Note any extra chars received will overite the begining of the buffer again
        }
    }
}

void __attribute__((interrupt, auto_psv))_U1TXInterrupt(void) {
    IFS0bits.U1TXIF = 0;
    if (tx_count < tx_lenght) {
        U1TXREG = buf_tx_uart[tx_count];
        tx_count++;
        return;
    }
}

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    T1CONbits.TON = 0;
    TMR1 = 0;
    _T1IF = 0;
    if (PIN4_RDY != pred_PIN4_RDY) {
        pred_PIN4_RDY = PIN4_RDY;
        PIN[3] = '4';
        PIN[5] = (~PIN4_RDY&0x01) + 0x30;
        start_TX(PIN, 7);
    }
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) {
    T2CONbits.TON = 0;
    TMR2 = 0;
    _T2IF = 0;
    if (PIN5_PASSED != pred_PIN5_PASSED) {
        pred_PIN5_PASSED = PIN5_PASSED;
        PIN[3] = '5';
        PIN[5] = (~PIN5_PASSED&0x01) + 0x30;
        start_TX(PIN, 7);
    }
}

void __attribute__((interrupt, auto_psv)) _T3Interrupt(void) {
    T3CONbits.TON = 0;
    TMR3 = 0;
    _T3IF = 0;
    if (PIN6_ERROR != pred_PIN6_ERROR) {
        pred_PIN6_ERROR = PIN6_ERROR;
        PIN[3] = '6';
        PIN[5] = (~PIN6_ERROR&0x01) + 0x30;
        start_TX(PIN, 7);
    }
}

void __attribute__((interrupt, auto_psv))_CNInterrupt(void) {
    if (PIN4_RDY != pred_PIN4_RDY)
        T1CONbits.TON = 1;
    if (PIN5_PASSED != pred_PIN5_PASSED)
        T2CONbits.TON = 1;
    if (PIN6_ERROR != pred_PIN6_ERROR)
        T3CONbits.TON = 1;
    _CNIF = 0;
}

void Init_PIC(void) {
    __builtin_write_OSCCONL(0x46);
    __builtin_write_OSCCONL(0x57);
    __builtin_write_OSCCONL(0b0011001101000000);

    CLKDIVbits.ROI = 0;
    //CLKDIVbits.DOZEN = 0;
    //CLKDIVbits.DOZE = 0b010;
    AD1PCFG = 0xffff;
    //  ADCON1 = 0x0F; /* turn off analog inputs */
    //CMCON  = 0x07; /* tuen off comparators */
    //__builtin_write_OSCCONL(OSCCON | 0x40);
    //  RCONbits.IPEN = 0; /* use legacy interrupt model */
}

void CN_init() {
    CN_PIN5_PASSED = 1;
    CN_PIN4_RDY = 1;
    CN_PIN6_ERROR = 1;

    CNEN_PIN5_PASSED = 1;
    CNEN_PIN4_RDY = 1;
    CNEN_PIN6_ERROR = 1;
CNPU1bits.CN13PUE = 1;
    pred_PIN5_PASSED = PIN5_PASSED;
    pred_PIN4_RDY = PIN4_RDY;
    pred_PIN6_ERROR = PIN6_ERROR;

    _CNIF = 0;
    _CNIE = 1;

}

void timer_init() {
    T1CONbits.TON = 0;
    T1CONbits.TCKPS = 3; //Timerx Input Clock Prescale Select bits
    //11 = 1:256
    //10 = 1:64
    //01 = 1:8
    //00 = 1:1
    T1CONbits.TCS = 0; //0 = Internal clock (FOSC/2)
    T1CONbits.TGATE = 0;
    IPC0bits.T1IP = 2; // Set Timer 2 Interrupt Priority Level
    PR1 = debounce; // 5 - ???? ??? ??????? ????? 0.1?? (0.0001?) ??? 25???, 500 - 10ms
    IEC0bits.T1IE = 1;
    IFS0bits.T1IF = 0;
    T1CONbits.TSIDL = 1;
    TMR1 = 0;

    T2CONbits.TON = 0;
    T2CONbits.T32 = 0; //1 = Timerx and Timery form a single 32-bit timer
    T2CONbits.TCKPS = 3; //Timerx Input Clock Prescale Select bits
    //11 = 1:256
    //10 = 1:64
    //01 = 1:8
    //00 = 1:1
    T2CONbits.TCS = 0; //0 = Internal clock (FOSC/2)
    T2CONbits.TGATE = 0;
    IPC1bits.T2IP = 2; // Set Timer 2 Interrupt Priority Level
    PR2 = debounce; // 5 - ???? ??? ??????? ????? 0.1?? (0.0001?) ??? 25???, 500 - 10ms
    IEC0bits.T2IE = 1;
    IFS0bits.T2IF = 0;
    T2CONbits.TSIDL = 1;
    TMR2 = 0;

    T3CONbits.TON = 0;
    T3CONbits.TCKPS = 3; //Timerx Input Clock Prescale Select bits
    //11 = 1:256
    //10 = 1:64
    //01 = 1:8
    //00 = 1:1
    T3CONbits.TCS = 0; //0 = Internal clock (FOSC/2)
    T3CONbits.TGATE = 0;
    IPC2bits.T3IP = 2; // Set Timer 2 Interrupt Priority Level
    PR3 = debounce; // 5 - ???? ??? ??????? ????? 0.1?? (0.0001?) ??? 25???, 500 - 10ms
    IEC0bits.T3IE = 1;
    IFS0bits.T3IF = 0;
    T3CONbits.TSIDL = 1;
    TMR3 = 0;


}

int main(void) {
    UART_Init();
    Init_PIC();
    timer_init();
    //
    CN_init();
    U1STAbits.UTXEN = 1; //Enable the interrupt system
    start_TX(Ready, 7);
    TRISB = 0;
    TRISC = 0;
    LATB = 0;
    LATC = 0;
    TRISBbits.TRISB2 = 1;
    TRISBbits.TRISB6 = 1;//??????
    TRISCbits.TRISC6 = 1; //??????
    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC8 = 1;
    TRISCbits.TRISC4 = 1;
    init_ow();

    ftoa(ds1820_read(), state_buf + 13, 1);
    //PHER_= ON;
    RED_ = ON;
    TION_POWER_ = ON;
    char y = 0;
    while (1) {
        y++;
        //PIN3_LOCK_UNLOCK = !PIN3_LOCK_UNLOCK;
        ClrWdt();
//start_TX(state_buf, 97);
        if (y > 30) {
            ftoa(ds1820_read(), state_buf + 13, 1);
            y = 0;
        }
        __delay_ms(500);
    }
}
