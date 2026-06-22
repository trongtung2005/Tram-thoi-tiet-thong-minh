#ifndef _UARTLIB_
#define _UARTLIB_

#include <mega328p.h>

// Da va loi chong tran so nguyen (them hau to UL)
#define F_OSC 16000000UL 

void uart_init(unsigned long BAUDRATE)
{
    // Cau hinh BAUD Rate an toan
    unsigned int n = (F_OSC / (16UL * BAUDRATE)) - 1;
    UBRR0H = (unsigned char)(n >> 8);
    UBRR0L = (unsigned char)n;

    // Cau hinh mode va data frame: Asynchronous, 8 data bit, 1 stop bit, no Parity
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

    // Bat transmiter, receiver va RX interupt
    UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);  
    #asm ("sei")
}

void uart_putchar(unsigned char data)
{
    // Dung dich bit de cho thanh ghi trong thay vi dung so magic number
    while (!(UCSR0A & (1<<UDRE0))); 
    UDR0 = data;
}

void uart_putstring(char *str)
{
   while (*str)
   {
        uart_putchar(*str); 
        // Neu thay ki tu xuong dong, tu dong them ki tu ve dau dong
        if (*str == '\n') uart_putchar('\r');
        str++;
   }
}

// ==========================================
// GIU NGUYEN THUAT TOAN GOC CUA BAN
// ==========================================

void uart_put_int(int value)
{
    unsigned char buf[8];
    int index = 0, i, j; 
    
    if (value < 0) j = -value;
    else j = value;
    
    do {
        buf[index] = j % 10 + 48; // Chuyen gia tri sang ki tu ASCII
        j = j / 10;
        index += 1;   
    } while(j);
    
    // In dau tru neu la so am
    if (value < 0) uart_putstring("-");
    
    // In nguoc mang de ra dung thu tu
    for (i = index; i > 0; i--)
        uart_putchar(buf[i-1]);
}

// In so x voi n chu so sau dau phay
void uart_put_float(float x, int n){
    int i;
    float temp;    
    
    if (x < 0) {
        temp = -x; 
        uart_putstring("-");
    }    
    else temp = x;
    
    uart_put_int((int)temp); 
    uart_putstring(".");
    
    temp = temp - (int)temp;
    for (i = 0; i < n; i++) {
        temp = temp * 10; 
        if (temp < 1) uart_putstring("0");
    }    
    uart_put_int((int)temp);
}

#endif