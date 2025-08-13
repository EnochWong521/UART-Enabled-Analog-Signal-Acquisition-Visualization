/*
 * File name: Lab7_3.c
 * Name: Enoch Wong
 * PI Name: Matthew Stormant
 * Description: Output voltage values to UART using event system triggered by timer overflow
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile float temp = 0.0f;
volatile uint16_t temp_hex = 0;
volatile uint8_t global_flag = 0;

void adc_init(void) {
	// signed 12 bit results
	ADCA.CTRLB = ADC_CONMODE_bm | ADC_RESOLUTION_12BIT_gc;
	
	// set reference voltage to 2.5V
	ADCA.REFCTRL = ADC_REFSEL_AREFB_gc;
	
	// set ADC clock
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	
	// configure differential ended mode
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_1X_gc;
	
	// measure voltage in PA1
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN6_gc;
	
	// set event system to channel 0
	EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc;
	
	// ADCA ch0 interrupt when conversion is complete. Set interrupt level to low
	ADCA.CH0.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_LO_gc;
	
	// set channel 0 to trigger ADC conversion
	ADCA.EVCTRL = ADC_EVACT_CH0_gc | ADC_EVSEL_0123_gc;
	
	// enable ADC
	ADCA.CTRLA = ADC_ENABLE_bm;
}

void tcc0_init(void) {
	// set timer count to 0
	TCC0.CNT = 0;
	
	// initialize timer period
	TCC0.PER = 53250;
	
	// set pre-scaler to 8
	TCC0.CTRLA = TC_CLKSEL_DIV8_gc;
} 

void usartd0_init(void) {
	uint8_t bsel = 9;
	int8_t bscale = -7;
	
	// Configure relevant TxD and RxD pins.
	PORTD.OUTSET = PIN3_bm;
	PORTD.DIRSET = PIN3_bm;
	PORTD.DIRCLR = PIN2_bm;
	
	// Configure baud rate as 116500
	USARTD0.BAUDCTRLA = (uint8_t)bsel;
	USARTD0.BAUDCTRLB = (uint8_t)((bscale << 4)|(bsel >> 8));
	
	// Configure remainder of serial protocol
	// 8 data bits, no parity, and one stop bit is chosen.
	USARTD0.CTRLC = (USART_CMODE_ASYNCHRONOUS_gc |
					USART_PMODE_ODD_gc |
					USART_CHSIZE_8BIT_gc) &
					~USART_SBMODE_bm;
					
	// Enable receiver and/or transmitter systems
	USARTD0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
}

void usartd0_out_char(char c)
{
	while(!(USARTD0.STATUS & USART_DREIF_bm));
	USARTD0.DATA = c;
}

void intr_init(void) {
	// turn on low level interrupts
	PMIC.CTRL = PMIC_LOLVLEN_bm;
	
	sei();
}

void output_voltage(float temp1) {
	// transmit sign
	if (temp1 >= 0 && temp1 <= 2.5) {
		usartd0_out_char('+');
	}
	else {
		usartd0_out_char('-');
		temp1 *= -1;
	}
	
	// transmit first digit
	int Int1 = (int)temp1;
	usartd0_out_char(Int1 + '0');
	
	// transmit decimal point
	usartd0_out_char('.');
	
	// transmit second digit
	float temp2 = 10 * (temp1 - Int1);
	int Int2 = (int)temp2;
	usartd0_out_char(Int2 + '0');
	
	// transmit third digit
	float temp3 = 10 * (temp2 - Int2);
	int Int3 = (int)(temp3);
	usartd0_out_char(Int3 + '0');
	
	// transmit unit V
	usartd0_out_char(' ');
	usartd0_out_char('V');
	usartd0_out_char(' ');
}

void output_hex(int16_t adc_value) {
    const char hex_digits[] = "0123456789ABCDEF";
    usartd0_out_char('(');
    usartd0_out_char('0');
    usartd0_out_char('x');

    // Cast to uint16_t to preserve the bit pattern
    uint16_t uvalue = (uint16_t)adc_value;

    // Extract and output the three hex digits
    uint8_t nibble;

    // Most significant nibble (bits 8-11)
    nibble = (uvalue >> 8) & 0x0F;
    usartd0_out_char(hex_digits[nibble]);

    // Middle nibble (bits 4-7)
    nibble = (uvalue >> 4) & 0x0F;
    usartd0_out_char(hex_digits[nibble]);

    // Least significant nibble (bits 0-3)
    nibble = uvalue & 0x0F;
    usartd0_out_char(hex_digits[nibble]);

    usartd0_out_char(')');
}

void output_new_line(void) {
	uint8_t cr = 13;
	uint8_t lf = 10;
	usartd0_out_char(cr);
	usartd0_out_char(lf);
}

ISR(ADCA_CH0_vect){
	// read ADC value
	int16_t adc_value = ADCA.CH0.RES;
	temp_hex = adc_value;
	temp = ((float)adc_value / 2047.0) * 2.5;
	
	// reset timer count
	TCC0.CNT = 0;
	
	// set global flag
	global_flag = 1;
}

int main(void) {
	tcc0_init();
	adc_init();
	intr_init();
	usartd0_init();
	
	// initialize io
	PORTA.DIRCLR = 0xFF;
	
	while(1) {
		if(global_flag) {
			// reset global flag	
			global_flag = 0;
			
			// output voltage value
			output_voltage(temp);
			
			// output hex value
			output_hex(temp_hex);
			
			// output new line
			output_new_line();
		}
	}
	return 0;
}