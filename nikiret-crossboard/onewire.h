#ifndef XC_HEADER_TEMPLATE_H
// C program for implementation of ftoa() 
#include<stdio.h> 
#include<math.h> 

#define	XC_HEADER_TEMPLATE_H

//#define _XTAL_FREQ 8000000UL

#include <xc.h>
#include <libpic30.h>


#define HIGH 1
#define LOW 0
#define OUTPUT 0
#define INPUT 1
#define SET 1
#define CLEAR 0

/**PORT D E F I N I T I O N S ****************************************************/

//ONE WIRE PORT PIN DEFINITION
///****************************************************
// This Configuration is required to make any PIC MicroController
// I/O pin as Open drain to drive 1-wire.
///****************************************************
#define OW_PIN_DIRECTION TRISBbits.TRISB13
#define OW_WRITE_PIN LATBbits.LATB13
#define OW_READ_PIN PORTBbits.RB13
#define OW_OPEN_DRAIN ODCBbits.ODB13

/**********************************************************************
 * Function: void drive_OW_low (void)
 * PreCondition: None
 * Input: None
 * Output: None
 * Overview: Configure the OW_PIN as Output and drive the OW_PIN LOW.
 ***********************************************************************/
void drive_OW_low(void) {
    OW_PIN_DIRECTION = OUTPUT;
    OW_OPEN_DRAIN = HIGH;
    OW_WRITE_PIN = LOW;
}

/**********************************************************************
 * Function: void drive_OW_high (void)
 * PreCondition: None
 * Input: None
 * Output: None
 * Overview: Configure the OW_PIN as Output and drive the OW_PIN HIGH.
 ***********************************************************************/
void drive_OW_high(void) {
    OW_PIN_DIRECTION = OUTPUT;
    OW_OPEN_DRAIN = HIGH;
    OW_WRITE_PIN = HIGH;
}

/**********************************************************************
 * Function: unsigned char read_OW (void)
 * PreCondition: None
 * Input: None
 * Output: Return the status of OW pin.
 * Overview: Configure as Input pin and Read the status of OW_PIN
 ***********************************************************************/
unsigned int read_OW(void) {
    unsigned int read_data = 3;
//    int i;

    OW_PIN_DIRECTION = INPUT;

    read_data = OW_READ_PIN;

    return read_data;
}

/**********************************************************************
 * Function: unsigned char OW_reset_pulse(void)
 * PreCondition: None
 * Input: None
 * Output: Return the Presense Pulse from the slave.
 * Overview: Initialization sequence start with reset pulse.
 * This code generates reset sequence as per the protocol
 ***********************************************************************/
unsigned int OW_reset_pulse(void) {

    unsigned int presence_detect;
//    int i;

    drive_OW_low(); // Drive the bus low

    __delay_us(480);

    drive_OW_high(); // Release the bus

    __delay_us(70); // delay 70 microsecond (us)

    presence_detect = !read_OW(); //Sample for presence pulse from slave. Slave present if '0' returned

    __delay_us(410); // delay 410 microsecond (us)

    drive_OW_high(); // Release the bus

    return presence_detect;
}

/**********************************************************************
 * Function: void OW_write_bit (unsigned char write_data)
 * PreCondition: None
 * Input: Write a bit to 1-wire slave device.
 * Output: None
 * Overview: This function used to transmit a single bit to slave device.
 *
 ***********************************************************************/

void OW_write_bit(unsigned char write_bit) {
    if (write_bit) {
        //writing a bit '1'
        drive_OW_low(); // Drive the bus low
        __delay_us(6); // delay 6 microsecond (us)
        drive_OW_high(); // Release the bus
        __delay_us(64); // delay 64 microsecond (us)
    } else {
        //writing a bit '0'
        drive_OW_low(); // Drive the bus low
        __delay_us(60); // delay 60 microsecond (us)
        drive_OW_high(); // Release the bus
        __delay_us(10); // delay 10 microsecond for recovery (us)
    }
}

/**********************************************************************
 * Function: unsigned char OW_read_bit (void)
 * PreCondition: None
 * Input: None
 * Output: Return the status of the OW PIN
 * Overview: This function used to read a single bit from the slave device.
 *
 ***********************************************************************/

unsigned char OW_read_bit(void) {
    unsigned char read_data;
    //reading a bit
    drive_OW_low(); // Drive the bus low
    __delay_us(3); // delay 6 microsecond (us)
    drive_OW_high(); // Release the bus
    __delay_us(9); // delay 9 microsecond (us)

    read_data = read_OW(); //Read the status of OW_PIN

    __delay_us(55); // delay 55 microsecond (us)
    return read_data;
}

/**********************************************************************
 * Function: void OW_write_byte (unsigned char write_data)
 * PreCondition: None
 * Input: Send byte to 1-wire slave device
 * Output: None
 * Overview: This function used to transmit a complete byte to slave device.
 *
 ***********************************************************************/
void OW_write_byte(unsigned char write_data) {
    unsigned char loop;

    for (loop = 0; loop < 8; loop++) {
        OW_write_bit(write_data & 0x01); //Sending LS-bit first
        write_data >>= 1; // shift the data byte for the next bit to send
    }
}

/**********************************************************************
 * Function: unsigned char OW_read_byte (void)
 * PreCondition: None
 * Input: None
 * Output: Return the read byte from slave device
 * Overview: This function used to read a complete byte from the slave device.
 *
 ***********************************************************************/
unsigned char OW_read_byte(void) {
    unsigned char loop, result = 0;

    for (loop = 0; loop < 8; loop++) {

        result >>= 1; // shift the result to get it ready for the next bit to receive
        if (OW_read_bit())
            result |= 0x80; // if result is one, then set MS-bit
    }
    return result;
}

/********************************************************************************************
                  E N D O F 1 W I R E . C
 *********************************************************************************************/
float resolution_dtermine(int temp_digital) {
    float temp_c;

    {
        if (temp_digital >= 0x800) //temperture is negative 
        {
            temp_c = 0;
            //calculate the fractional part 
            if (temp_digital & 0x0001) temp_c += 0.06250;
            if (temp_digital & 0x0002) temp_c += 0.12500;
            if (temp_digital & 0x0004) temp_c += 0.25000;
            if (temp_digital & 0x0008) temp_c += 0.50000;

            //calculate the whole number part 
            temp_digital = (temp_digital >> 4) & 0x00FF;
            temp_digital = temp_digital - 0x0001; //subtract 1 
            temp_digital = ~temp_digital; //ones compliment 
            temp_c = temp_c - (float) (temp_digital & 0xFF);
        }
        else //temperture is positive 
        {
            temp_c = 0;
            //calculate the whole number part 
            temp_c = (temp_digital >> 4) & 0x00FF;
            //calculate the fractional part 
            if (temp_digital & 0x0001) temp_c = temp_c + 0.06250;
            if (temp_digital & 0x0002) temp_c = temp_c + 0.12500;
            if (temp_digital & 0x0004) temp_c = temp_c + 0.25000;
            if (temp_digital & 0x0008) temp_c = temp_c + 0.50000;
        } //end if else 

    }
    return temp_c;
}

float ds1820_read() {
    //unsigned char busy = 0;
    unsigned char temp2;
    char temp1;
    float result;

    OW_reset_pulse();
    OW_write_byte(0xCC);
    OW_write_byte(0x44);

    //while (busy == 0) 
    // busy = OW_read_byte();

    __delay_ms(800);
    OW_reset_pulse();
    OW_write_byte(0xCC);
    OW_write_byte(0xBE);
    temp2 = OW_read_byte();
    temp1 = OW_read_byte();
    if(temp1!=0)
        result = (~temp2 + 1)*0.5;
    else 
        result = temp2*0.5; //Calculation for DS18S20 with 0.5 deg C resolution 

    return result;
}
// reverses a string 'str' of length 'len' 
void reverse(unsigned char *str, int len) {
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
// Converts a given integer x to string str[].  d is the number 
// of digits required in output. If d is more than the number 
// of digits in x, then 0s are added at the beginning. 
int intToStr(int x, unsigned char str[], int d) {
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }

    // If number of digits required is more, then 
    // add 0s at the beginning 
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\r';
    return i;
}
// Converts a floating point number to string, return last index. 
int ftoa(float n, unsigned char *res, int afterpoint) {
    int i = 0;
    // Extract integer part 
    int ipart = (int) n;
    if (n < 10)
    {
        afterpoint++;
    }
    // convert integer part to string 
    i = intToStr(ipart, res, i);
    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot
        // Extract floating part
        float fpart = n - (float) ipart;
        // Get the value of fraction part upto given no. 
        // of points after dot. The third parameter is needed 
        // to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);
         i += intToStr((int) fpart, res + i + 1, afterpoint);
    }
    return i;
}

void toHexString(unsigned char *buf, int _lenght) {
    for (int lenght = _lenght - 1; lenght >= 0; lenght--) {
        unsigned char low = (buf[lenght] & 0x0F);

        if (low < 0x0A)
            buf[2 * lenght + 1] = low + 0x30;
        else
            buf[2 * lenght + 1] = low + 0x37;

        unsigned char hi = (buf[lenght] & 0xF0) >> 4;
        if (hi < 0x0A)
            buf[2 * lenght] = hi + 0x30;
        else
            buf[2 * lenght] = hi + 0x37;
    }
}
void init_ow()
{
     OW_reset_pulse();
    OW_write_byte(0xCC);
    OW_write_byte(0x4E);
    __delay_ms(20);
    OW_write_byte(0);
    OW_write_byte(0);
    OW_write_byte(0x1F );

    //while (busy == 0) 
    // busy = OW_read_byte();

    //__delay_ms(950);
}
#endif	/* XC_HEADER_TEMPLATE_H */

