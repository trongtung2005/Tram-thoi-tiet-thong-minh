#ifndef _TWILIB_
#define _TWILIB_

#include <mega328p.h>
#include <delay.h>

// Them hau to UL de tranh tran bo nho
#define F_CPU 16000000UL 

#define BR400   12   
#define BR200   32 
#define BR100   72

// Global variables
unsigned char TWI_Rx_Buf[50];
unsigned char TWI_Tx_Buf[50];
int TWI_Rx_Index = 0, TWI_Tx_Index = 0, TWI_Data_In = 0;
unsigned char Status_Code;
int i;

void TWI_Init(char sla, char gcall, unsigned long sclock){
    // Gan dia chi Slave
    TWAR = (sla<<1) + gcall;
    
    // Thiet lap xung nhip sclock (Prescale = 1)
    TWSR &= ~((1<<TWPS1) | (1<<TWPS0)); 
    TWBR = (unsigned char)(((F_CPU / sclock) - 16) / 2);
    
    // Bat TWI, bat ngat TWI, cho phep tra loi ACK
    TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
} 

void TWI_Interupt_Enable(void){
    TWCR |= (1<<TWEA) | (1<<TWIE);      
}

void TWI_Interupt_Disable(void){
    TWCR &= ~(1<<TWIE);
}

unsigned char TWI_Start(void){
    // Gui tin hieu Start
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    return (TWSR & 0xF8);
}      

void TWI_Stop(void) {
    // Gui tin hieu Stop
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN); 
} 
    
unsigned char TWI_SLA_RW(unsigned char add, unsigned char rw){
    TWDR = (add<<1) + rw;
    TWCR = (1<<TWINT) | (1<<TWEN);
    
    while (!(TWCR & (1<<TWINT)));
    return (TWSR & 0xF8); 
}

unsigned char TWI_Send_Byte(unsigned char b){
    TWDR = b;
    TWCR = (1<<TWINT) | (1<<TWEN);
    
    while (!(TWCR & (1<<TWINT)));
    return (TWSR & 0xF8); 
}

unsigned char TWI_Send_Array(unsigned char* arr, int length){
    for (i = 0; i < length; i++){       
        if (TWI_Send_Byte(arr[i]) != 0x28) 
            return (TWSR & 0xF8);
    }
    return (TWSR & 0xF8);  
}

void TWI_Error(void){
    TWI_Stop();
    TWI_Interupt_Enable(); 
}

unsigned char TWI_Master_Send(unsigned char sla, unsigned char* arr, int length){
   TWI_Interupt_Disable();
   if (TWI_Start() != 0x08) return 1;               
   if (TWI_SLA_RW(sla,0) != 0x18) return 1;
   if (TWI_Send_Array(arr, length) != 0x28) return 1;
   
   TWI_Stop();
   TWI_Interupt_Enable();
   return 0;
} 

unsigned char TWI_Read_Byte(char* b, char ACK){
    if (ACK) 
        TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN); // Xoa TWINT, set TWEA gui ACK 
    else 
        TWCR = (1<<TWINT) | (1<<TWEN);             // Xoa TWINT, clear TWEA gui NACK
        
    while (!(TWCR & (1<<TWINT)));
    *b = TWDR; 
    return (TWSR & 0xF8); 
}

unsigned char TWI_Read_Array(char* arr, int length){
    for (i = 0; i < length - 1; i++) { 
        if (TWI_Read_Byte(arr + i, 1) != 0x50) return (TWSR & 0xF8);
    }   
    if (TWI_Read_Byte(arr + i, 0) != 0x58) return (TWSR & 0xF8);
    return 0x58;    
}

unsigned char TWI_Master_Receive(unsigned char sla, unsigned char* arr, int length){
   TWI_Interupt_Disable();
   if (TWI_Start() != 0x08) return 1; 
   if (TWI_SLA_RW(sla,1) != 0x40) return 1;
   
   for (i = 0; i < (length-1); i++){
       TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN); 
       while (!(TWCR & (1<<TWINT)));
       if ((TWSR & 0xF8)!= 0x50) return 1;
       arr[i] = TWDR;       
   }
   
   TWCR = (1<<TWINT) | (1<<TWEN);   
   while (!(TWCR & (1<<TWINT)));
   if ((TWSR & 0xF8)!= 0x58) return 1;
   arr[length-1] = TWDR;
   
   TWI_Stop();
   TWI_Interupt_Enable();
   return 0;       
}

//=======================================================
// TRINH PHUC VU NGAT TWI (ISR) 
//=======================================================
interrupt [TWI] void TWI_isr(void){
    Status_Code = (TWSR & 0xF8);
    switch (Status_Code){
    
    //============= Slave Receive Mode ==============
    case 0x60: 
        TWI_Rx_Index = 0;
        TWCR |= (1<<TWINT) | (1<<TWEA); 
        break;
        
    case 0x80: 
        if (TWI_Rx_Index < 49){ 
            TWI_Rx_Buf[TWI_Rx_Index++] = TWDR;
            TWCR |= (1<<TWINT) | (1<<TWEA); 
            break;
        }
        else { 
            TWI_Rx_Buf[TWI_Rx_Index] = TWDR;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE); 
            break;
        }
        
    case 0x88: 
        TWCR |= (1<<TWINT) | (1<<TWEA);
        break;
        
    //============= For General Call ================   
    case 0x70: 
        TWI_Rx_Index = 0;
        TWCR |= (1<<TWINT) | (1<<TWEA); 
        break;
        
    case 0x90: 
        if (TWI_Rx_Index < 49){ 
            TWI_Rx_Buf[TWI_Rx_Index++] = TWDR;
            TWCR |= (1<<TWINT) | (1<<TWEA);
            break;
        }
        else { 
            TWI_Rx_Buf[TWI_Rx_Index] = TWDR;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE); 
            break;
        }
        
    case 0x98: 
        TWCR |= (1<<TWINT) | (1<<TWEA); 
        break;
        
    case 0xA0: 
        TWI_Data_In = 1;
        TWCR |= (1<<TWINT) | (1<<TWEA);
        break;

    //============= Slave Transmit Mode ==============   
    case 0xA8: 
        TWI_Tx_Index = 0;       
        TWDR = TWI_Tx_Buf[TWI_Tx_Index++];
        TWCR |= (1<<TWINT) | (1<<TWEA); 
        break;
        
    case 0xB8: 
        if (TWI_Tx_Index < 49){
            TWDR = TWI_Tx_Buf[TWI_Tx_Index++];
            TWCR |= (1<<TWINT) | (1<<TWEA); 
            break;
        }
        else {
            TWDR = TWI_Tx_Buf[TWI_Tx_Index];    
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE); 
            break;
        }
        
    case 0xB0: 
        TWDR = TWI_Tx_Buf[TWI_Tx_Index++];
        TWCR |= (1<<TWINT) | (1<<TWEA); 
        break;
        
    case 0xC0: 
        TWCR |= (1<<TWINT) | (1<<TWEA); 
        break;
        
    case 0xC8: 
        TWCR |= (1<<TWINT) | (1<<TWEA); 
        break;  
    }
} 

#endif