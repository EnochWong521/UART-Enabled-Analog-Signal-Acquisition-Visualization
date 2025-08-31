/*
 * File name: Lab7_1.c
 * Name: Enoch Wong
 * PI Name: Matthew Stormant
 * Description: ADC conversion
 */ 

#include <avr/io.h>

void adc_init(void) {
	PORTA.DIRCLR = 0xFF;
	
	// signed 12 bit results
	ADCA.CTRLB = ADC_CONMODE_bm |ADC_RESOLUTION_12BIT_gc;
	
	// set reference voltage to 2.5V
	ADCA.REFCTRL = ADC_REFSEL_AREFB_gc;
	
	// set ADC clock
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	
	// configure differential ended mode
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_1X_gc;
	
	// measure voltage in PA1
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN6_gc;
	
	// enable ADC
	ADCA.CTRLA = ADC_ENABLE_bm;
}


int main(void)
{
	//initialize channel 0 of ADCA
	adc_init();

    // temporary 8 bit variable to store result
	int16_t temp = 0;
    while (1) 
    {
		// start conversion
		ADCA.CH0.CTRL |= ADC_CH_START_bm;
		
		// wait for conversion to be complete
		while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));
		
		// read 12 bit result
		temp = ADCA.CH0.RES;
		
		// clear interrupt flag
		ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
    }
	return 0;
}