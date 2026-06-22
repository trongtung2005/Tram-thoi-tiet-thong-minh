#include <mega328p.h>
#include <delay.h>
#include <stdlib.h>

// Thu vien he thong
#include "TWI_Lib2.h"
#include "lcd2004_i2c.h"
#include "rtc_hw111.h"
#include "hall_speed.h"
#include "wind_dir.h"
#include "rain_sensor.h"
#include "dht11_lite.h"
#include "UART_Lib.h" // Include thu vien UART cua ban

// --- C?U HÌNH CHÂN CÒI CHÍP (BUZZER) ---
#define BUZZER_PIN 7 // Dùng chân D7

void main(void)
{
    unsigned char page = 1;       
    unsigned char tick = 0;       
    unsigned char dht_status = 0; 
    
    // Cac bien xu ly logic coi bao mua
    unsigned char is_raining = 0;
    unsigned char was_raining = 0;
    unsigned char buzzer_timer = 0; 

    // Cau hinh chan D7 la Output. Xuat muc HIGH de TAT coi mac dinh
    DDRD |= (1 << BUZZER_PIN);
    PORTD |= (1 << BUZZER_PIN); 

    // 1. Khoi tao he thong
    uart_init(9600); // Khoi tao UART cuc ky ngan gon nho thu vien moi
    TWI_Init(1, 0, 100000);
    LCD_Init(0x27); 
    Hall_Speed_Init(); 
    Wind_Dir_Init(); 
    Rain_Sensor_Init();

    // Hien thi man hinh khoi dong
    LCD_Clear();
    LCD_Set_Cursor(4, 1);
    LCD_Print("System Ready");
    delay_ms(1000);
    LCD_Clear();  
    
    while (1)
    {
        // 2. Doc du lieu
        HW111_Read_Time();
        HW111_Read_Date();         
        dht_status = DHT11_Read(); 
        is_raining = Rain_Check(); 
        
        // --- LOGIC CÒI BÁO MUA (BÍP... BÍP... NG?T QUÃNG) ---
        if (is_raining == 1 && was_raining == 0) {
            buzzer_timer = 10; 
        }
        was_raining = is_raining; 

        if (buzzer_timer > 0) {
            if (buzzer_timer % 2 == 0) {
                PORTD &= ~(1 << BUZZER_PIN); 
            } else {
                PORTD |= (1 << BUZZER_PIN);  
            }
            buzzer_timer--;                  
        } else {
            PORTD |= (1 << BUZZER_PIN);      
        }

        // --- G?I D? LI?U SANG ESP32 B?NG UART_LIB ---
        // Ghep noi cac chuoi va bien de tao thanh cau truc: *T:...,H:...,S:...,D:...,R:...#
        uart_putstring("*T:");
        uart_put_int(temp);
        
        uart_putstring(",H:");
        uart_put_int(hum);
        
        uart_putstring(",S:");
        uart_put_float(wind_speed_ms, 1); // Gui toc do gio voi 1 chu so thap phan
        
        uart_putstring(",D:");
        uart_putstring(Wind_Dir_Read());
        
        uart_putstring(",R:");
        uart_put_int(is_raining);
        
        uart_putstring("#\n"); // Chot chuoi bang dau # va xuong dong

        // --- HIEN THI TRANG 1: GIO & MUA ---
        if (page == 1) 
        {
            LCD_Set_Cursor(0, 0); 
            LCD_Print("Spd: "); LCD_Print_Fake_Float(wind_speed_ms); LCD_Print(" m/s      ");
            
            LCD_Set_Cursor(0, 2); 
            LCD_Print("Dir: "); LCD_Print(Wind_Dir_Read()); LCD_Print("      ");
            
            LCD_Set_Cursor(0, 3);
            if(is_raining) { LCD_Print("Rain: DANG MUA!     "); }
            else { LCD_Print("Rain: Khong mua     "); }
        }
        
        // --- HIEN THI TRANG 2: THOI GIAN & NHIET DO ---
        else if (page == 2)
        {
            LCD_Set_Cursor(0, 0); 
            LCD_Print("Date: ");
            LCD_Print_Time_Format(rtc_date); LCD_Print("/");
            LCD_Print_Time_Format(rtc_month); LCD_Print("/20");
            LCD_Print_Int(rtc_year);
            LCD_Print("      ");
            
            LCD_Set_Cursor(0, 1); 
            LCD_Print("Time: "); 
            LCD_Print_Time_Format(rtc_hour); LCD_Print(":"); 
            LCD_Print_Time_Format(rtc_min);  LCD_Print(":"); 
            LCD_Print_Time_Format(rtc_sec);  LCD_Print("   ");

            if (dht_status == 1) {
                LCD_Set_Cursor(0, 2); LCD_Print("Temp: "); LCD_Print_Int(temp); LCD_Print(" C       ");
                LCD_Set_Cursor(0, 3); LCD_Print("Hum : "); LCD_Print_Int(hum);  LCD_Print(" %       ");
            } else {
                LCD_Set_Cursor(0, 2); LCD_Print("Temp: Loi cam bien! ");
                LCD_Set_Cursor(0, 3); LCD_Print("Hum : Mat ket noi!  ");
            }
        }

        delay_ms(500); 

        // 3. Logic luan phien chuyen trang
        tick++;
        if (tick >= 6) 
        {
            tick = 0; 
            page = (page == 1) ? 2 : 1; 
            LCD_Clear(); 
        }
    }
}