#ifndef _LCD2004_I2C_H_
#define _LCD2004_I2C_H_

#include <mega328p.h>
#include <delay.h>
#include "TWI_Lib2.h" // Bat buoc phai include thu vien I2C da toi uu

// Dia chi I2C cua module LCD (Thuong la 0x27 hoac 0x3F)
unsigned char LCD_ADDR = 0x27; 
unsigned char backlight_val = 0x08; // Mac dinh bat den nen

// Ham gui 1 byte truc tiep xuong PCF8574
void LCD_Write_I2C(unsigned char data) {
    unsigned char arr[1];
    arr[0] = data;
    TWI_Master_Send(LCD_ADDR, arr, 1);
}

// Ham tao xung Enable (EN) de LCD nhan du lieu
void LCD_Pulse_EN(unsigned char data) {
    LCD_Write_I2C(data | 0x04); // EN = 1
    delay_us(1);
    LCD_Write_I2C(data & ~0x04); // EN = 0
    delay_us(50);
}

// Ham gui lenh (Command) hoac Du lieu (Data) xuong LCD (Giao tiep 4-bit)
void LCD_Send(unsigned char value, unsigned char mode) {
    unsigned char high_nib = value & 0xF0;
    unsigned char low_nib  = (value << 4) & 0xF0;
    
    // mode = 0: Gui lenh (RS=0) | mode = 1: Gui chu (RS=1)
    LCD_Write_I2C(high_nib | backlight_val | mode);
    LCD_Pulse_EN(high_nib | backlight_val | mode);
    
    LCD_Write_I2C(low_nib | backlight_val | mode);
    LCD_Pulse_EN(low_nib | backlight_val | mode);
}

// Gui lenh dieu khien
void LCD_Cmd(unsigned char cmd) {
    LCD_Send(cmd, 0);
}

// Gui 1 ky tu
void LCD_Char(unsigned char data) {
    LCD_Send(data, 1);
}

// Khoi tao LCD 2004
void LCD_Init(unsigned char addr) {
    LCD_ADDR = addr;
    delay_ms(50);
    
    // Trinh tu khoi tao 4-bit theo Datasheet cua Hitachi HD44780
    LCD_Write_I2C(0x30 | backlight_val);
    LCD_Pulse_EN(0x30 | backlight_val);
    delay_ms(5);
    
    LCD_Write_I2C(0x30 | backlight_val);
    LCD_Pulse_EN(0x30 | backlight_val);
    delay_ms(1);
    
    LCD_Write_I2C(0x30 | backlight_val);
    LCD_Pulse_EN(0x30 | backlight_val);
    delay_ms(1);
    
    LCD_Write_I2C(0x20 | backlight_val); // Chuyen sang che do 4-bit
    LCD_Pulse_EN(0x20 | backlight_val);
    delay_ms(1);
    
    LCD_Cmd(0x28); // 4-bit, 2 lines (LCD 2004 van dung lenh nay), 5x8 font
    LCD_Cmd(0x0C); // Bat man hinh, Tat con tro
    LCD_Cmd(0x06); // Tu dong tang con tro
    LCD_Cmd(0x01); // Xoa toan man hinh
    delay_ms(2);
}

// Dat con tro toi vi tri Cot (0-19) va Hang (0-3) cua LCD 2004
void LCD_Set_Cursor(unsigned char col, unsigned char row) {
    // Bang ma dia chi dau hang cua LCD 20x4
    unsigned char row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row > 3) row = 3;
    LCD_Cmd(0x80 | (col + row_offsets[row]));
}

// In mot chuoi ky tu ra man hinh
void LCD_Print(char *str) {
    while (*str) {
        LCD_Char(*str++);
    }
}

// Xoa man hinh
void LCD_Clear() {
    LCD_Cmd(0x01);
    delay_ms(2);
}


      // --- HAM IN SO DA MO RONG ---

// In s? nguyên
void LCD_Print_Int(int value) { 
    char buf[8]; 
    itoa(value, buf); 
    LCD_Print(buf); 
}

// In s? th?p phân (ví d? 2.5)
void LCD_Print_Fake_Float(float value) {
    int val_nhan10 = (int)(value * 10.0);
    LCD_Print_Int(val_nhan10 / 10);
    LCD_Print(".");
    LCD_Print_Int(val_nhan10 % 10);
}

 // Ham bo tro hien thi thoi gian co so 0 o dau (VD: 09:05)
void LCD_Print_Time_Format(int value) {
    if (value < 10) LCD_Print("0");
    LCD_Print_Int(value);
}

#endif