/*
 * File name: Lab7_5.c
 * Name: Enoch Wong
 * PI Name: Matthew Stormant
 * Description: Output voltage values to UART using event system triggered by timer overflow
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t adc = 0;
volatile uint8_t adc_flag = 0;
volatile uint8_t key = 0;

void adc_init(void) {
	// signed 12 bit results
	ADCA.CTRLB = ADC_CONMODE_bm | ADC_RESOLUTION_12BIT_gc;
	
	// set reference voltage to 2.5V
	ADCA.REFCTRL = ADC_REFSEL_AREFB_gc;
	
	// set ADC clock
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	
	// configure differential ended mode
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_1X_gc;
	
	// set event system to channel 0
	EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc;
	
	// ADCA ch0 interrupt when conversion is complete. Set interrupt level to low
	ADCA.CH0.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_MED_gc;
	
	// set channel 0 to trigger ADC conversion
	ADCA.EVCTRL = ADC_EVACT_CH0_gc | ADC_EVSEL_0123_gc;
	
	// enable ADC
	ADCA.CTRLA = ADC_ENABLE_bm;
}

void tcc0_init(void) {
	// set timer count to 0
	TCC0.CNT = 0;
	
	// initialize timer period
	TCC0.PER = 13158;
	
	// set pre-scaler to 8
	TCC0.CTRLA = TC_CLKSEL_DIV1_gc;
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
	
	// configure interrupt levels of USART receive and transmit
	USARTD0.CTRLA = USART_RXCINTLVL_LO_gc;
}

char usartd0_in_char(void)
{
	while(!(USARTD0_STATUS & USART_RXCIF_bm));
	return USARTD0_DATA;
}

void usartd0_out_char(char c)
{
	while(!(USARTD0.STATUS & USART_DREIF_bm));
	USARTD0.DATA = c;
}

void intr_init(void) {
	// turn on low level interrupts
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	
	sei();
}

void output_voltage(uint16_t adc_value) {
	// access byte 1 of raw ADC value
	uint8_t adc_byte1 = adc_value & 0xFF;
	
	// access byte 2 of raw ADC value
	uint8_t adc_byte2 = (adc_value & 0xFF00) >> 8;
	
	// transmit data through UART
	usartd0_out_char(adc_byte1);
	usartd0_out_char(adc_byte2);
}

ISR(ADCA_CH0_vect){
	// read ADC value
	adc = ADCA.CH0.RES;
		
	// reset timer count
	TCC0.CNT = 0;
	
	// set global adc flag
	adc_flag = 1;
}

ISR(USARTD0_RXC_vect){
	// read keyboard input
	key = USARTD0_DATA;
	
	if (key == 'B') {
		// measure voltage in photo-resistor
		ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN6_gc;
	}
	else if (key == 'F') {
	// measure voltage in analog input
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN4_gc | ADC_CH_MUXNEG_PIN5_gc;
	}
}

int main(void) {
	usartd0_init();
	tcc0_init();
	adc_init();
	intr_init();
	
	// initialize io
	PORTA.DIRCLR = 0xFF;
	
	while(1) {
		if(adc_flag) {
			// reset global flag	
			adc_flag = 0;
			output_voltage(adc);
		}
	}
	return 0;
}