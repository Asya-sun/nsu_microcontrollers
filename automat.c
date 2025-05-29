#define F_CPU 1000000UL
// 
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>


#define LCD_RS PD0
#define LCD_E  PD1
#define LCD_D4 PD4
#define LCD_D5 PD5
#define LCD_D6 PD6
#define LCD_D7 PD7


void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_init(void);
void lcd_print(const char* str);
void lcd_clear(void);
void lcd_goto(uint8_t pos);

// create short imp on en to prove sending data or command
void lcd_pulse_enable() {
	//en = 1
	PORTD |= (1 << LCD_E);
	_delay_us(1);
	// en = 0
	PORTD &= ~(1 << LCD_E);
	_delay_us(100);
}

void lcd_send_nibble(uint8_t nibble) {
	PORTD = (PORTD & 0x0F) | (nibble & 0xF0);
	lcd_pulse_enable();
}

uint16_t adc_read() {
	// start preobrazovanie acp
	ADCSRA |= (1 << ADSC);   
	// wait ending of preobrazovanie acp
	while (ADCSRA & (1 << ADSC));   
	return ADC;                     
}


void lcd_command(uint8_t cmd) {
	// rs => 0 send command
	PORTD &= ~(1 << LCD_RS);
	lcd_send_nibble(cmd);
	lcd_send_nibble(cmd << 4);
	_delay_ms(2);
}

void lcd_data(uint8_t data) {
	// rs => 1 send data
	PORTD |= (1 << LCD_RS);
	lcd_send_nibble(data);
	lcd_send_nibble(data << 4);
	_delay_ms(2);
}

void lcd_init(void) {
	// out port d - to send signals
	DDRD |= (1 << LCD_RS) | (1 << LCD_E) |
	(1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7);
	
	_delay_ms(50);
	uint8_t cmd = 0;
	// DB5=1, DB4=1 (Function Set), DB3=0 (8-bit)
	cmd |= (1 << 5) | (1 << 4); 
	// 0x30 
	lcd_send_nibble(cmd);  
	
	_delay_ms(5);
	cmd = 0;
	// DB5=1, DB4=1 (Function Set), DB3=0 (8-bit)
	cmd |= (1 << 5) | (1 << 4);
	// 0x30
	lcd_send_nibble(cmd);
	
	_delay_us(150);
	cmd = 0;
	// DB5=1, DB4=1 (Function Set), DB3=0 (8-bit)
	cmd |= (1 << 5) | (1 << 4);
	// 0x30
	lcd_send_nibble(cmd);
	
	cmd = 0;
	// DB5=1 (Function Set), DB4=0 (4-bit)
	cmd |= (1 << 5); 
	// 0x20      
	// 4-bit mode
	lcd_send_nibble(cmd);  


	// 4-bit, 2 line, 5x7 font
	cmd = 0;
	// DB5=1 (Function Set), DB4=0 (4-bit), DB3=1 (2 ??????), DB2=0 (5x8)
	cmd |= (1 << 5) | (1 << 3);  
	// 0x28
	lcd_command(cmd);  

	
	// Display ON, cursor OFF
	cmd = 0;
	cmd |= (1 << 3) | (1 << 2);  // DB3=1 (Display ON), DB2=1 (Cursor OFF), DB1=0 (Blink OFF)
	lcd_command(cmd);  // 0x0C
	
	// Entry mode
	cmd = 0;
	cmd |= (1 << 1);  // DB1=1 (Increment), DB0=0 (No Display Shift)
	lcd_command(cmd);  // 0x06
	
	// Clear display
	cmd = 0;
	cmd |= (1 << 0);  // DB0=1 (Clear Display)
	lcd_command(cmd);  // 0x01
	_delay_ms(2);
}

void lcd_print(const char* str) {
	while (*str) {
		lcd_data(*str++);
	}
}

void lcd_clear(void) {
	// full clear of display
	lcd_command(0x01);
	_delay_ms(2);
}

void lcd_goto(uint8_t pos) {
	lcd_command(0x80 + pos);
}


void adc_init() {
	// AVcc as reference, ADC0
	ADMUX = (1 << REFS0);              
	// Enable, prescaler 8
	ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0); 
}

int main(void) {
	char buffer[16];
	lcd_init();
	adc_init();
	
	while (1) {
		uint16_t adc_val = adc_read();
		lcd_clear();
		
		lcd_goto(0);
		lcd_print("RAW=");
		lcd_print(itoa(adc_val, buffer, 10));
		
		
		lcd_goto(0x40);
		int voltage_mV = (adc_val * 5000L) / 1023;
		sprintf(buffer, "U=%d.%02d V", voltage_mV / 1000, (voltage_mV % 1000) / 10);
		lcd_print(buffer);
		
		_delay_ms(500);
	}
}
