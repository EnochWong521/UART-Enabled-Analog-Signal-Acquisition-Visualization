/*
 * File name: Lab7_2.c
 * Name: Enoch Wong
 * PI Name: Matthew Stormant
 * Description: Flash blue PWM using event system, triggered by timer overflow
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

int16_t temp = 0;

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

void intr_init(void) {
	// turn on low level interrupts
	PMIC.CTRL = PMIC_LOLVLEN_bm;
	
	sei();
}

ISR(ADCA_CH0_vect){
	// store result into variable
	temp = ADCA.CH0.RES;
	
	// toggle blue pwm
	PORTD.OUTTGL = PIN6_bm;
	
	// reset timer count
	TCC0.CNT = 0;
}

int main(void) {
	tcc0_init();
	adc_init();
	intr_init();
	
	// initialize io
	PORTA.DIRCLR = 0xFF;
	PORTD.OUTSET = PIN6_bm;
	PORTD.DIRSET = PIN6_bm;
	
	while(1);
	return 0;
}