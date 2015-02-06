#include <msp430.h>	
#include "Energia.h"

#ifndef LED_TILE_H_
#define LED_TILE_H_
#define period 1023 //period - 1 for PWM output

//Defines the on-board devices
#define F1 A7
#define F2 A5
#define F3 A4
#define MIC A0
#define S1 BIT3 //Port 1
#define S2 BIT0 //Port 2
#define R BIT0 //Port 1
#define G BIT1 //Port 2
#define B BIT4 //Port 2

unsigned char ADCDone;
int ADCValue;

//define three unsigned chars to keep track of linear representations of RGB values, prior to srgb conversion, to prevent the need to take sqrt.
unsigned char _r;
unsigned char _g;
unsigned char _b;


//Bullshit to make new energia happy
void dec_mode();
void inc_mode();

//CCR values for use with 'mangler'
int r_ccr = 0;
int g_ccr = 0;
int b_ccr = 0;

//Array of 'pseudorandom' values to mangle output registers with to reduce resonances of drive electronics
signed int mangler[] = {-317,167,-13,317,-167,13};
char rand_index = 0; //index of current mangler value (changed every 8ms)

void setup_piled(void);


void set_lrgb(unsigned char r_new, unsigned char g_new, unsigned char b_new);
void set_rgb(unsigned char r_new, unsigned char g_new, unsigned char b_new);
void target_rgb(unsigned char r, unsigned char g, unsigned char  b, unsigned long time);
void sleep(int cycles);
int Single_Measure(unsigned int chan);
int _Single_Measure(unsigned int chan);

void setup_piled(){
  
   // start serial port at 9600 bps:
        Serial.begin(9600);
	P1DIR |= BIT6;             // P1.2 to output
	P1SEL |= BIT6;             // P1.2 to TA0.1
	P2DIR |= BIT1;             // P2.1 to output
	P2SEL |= BIT1;             // P2.1 to TA1.1
	P2DIR |= BIT4;             // P2.4 to output
	P2SEL |= BIT4;             // P2.4 to TA1.2
 	//*************************
 
        //Set up Pin 1.3 as a button input
        P1DIR &= ~BIT3;
        P1REN |= BIT3;
        P1OUT |= BIT3;
        P1IE |= BIT3; // P1.3 interrupt enabled
        P1IFG &= ~BIT3; // P1.3 IFG cleared
        
        //Set up Pin 2.0 as a button input
        P2DIR &= ~BIT0;
        P2REN |= BIT0;
        P2OUT |= BIT0;
        P2IE |= BIT0; // P2.0 interrupt enabled
        P2IFG &= ~BIT0; // P2.0 IFG cleared
        
        
        digitalWrite(P2_0, HIGH);
        //Set up timer
	CCTL1 = OUTMOD_7;// reset/set mode
	CCTL2 = OUTMOD_7;
	TA1CCTL1 = OUTMOD_7;
	TA1CCTL2 = OUTMOD_7;
	TACTL = TASSEL_2 + MC_1;           // SMCLK/8, upmode
	TA1CTL = TASSEL_2 + MC_1;
	CCR0 = period;        // period timer 0
	TA1CCR0 = period; //period timer 1
	CCR1 = 0;         // R
	CCR2 = 0;         // G
        
       attachInterrupt(P1_3, dec_mode, FALLING);
       attachInterrupt(P2_0, inc_mode, FALLING);
       
}

void sleep(int cycles){ //wastes cpu cycles in increments of 1ms...for horribly innefficient software delays. Luckily, they do not interfere with timers, which is important...
	while (cycles>0){
		__delay_cycles(15999);
		cycles--;
	}
}



int _Single_Measure(unsigned int chan)
{
	ADC10CTL0 &= ~ENC;				// Disable ADC
	ADC10CTL0 = ADC10SHT_3 + ADC10ON + ADC10IE;	// 64 clock ticks, ADC On, enable ADC interrupt
	ADC10CTL1 = ADC10SSEL_3 + chan;				// Set 'chan', SMCLK
	ADC10CTL0 |= ENC + ADC10SC;             	// Enable and start conversion
	notdone:
	if(ADCDone==1)							// If the ADC is done with a measurement
			{
				ADCDone = 0;
				ADC10CTL0 |= ENC;				// Disable ADC

				return ADCValue;
			}
	else goto notdone;
}

int Single_Measure(unsigned int chan){
	return (_Single_Measure(chan) + _Single_Measure(chan) + _Single_Measure(chan) + _Single_Measure(chan) - 2) / 4; //take 4 readings, add them up and subtract 2 to take a true average
}

void target_rgb(unsigned char r, unsigned char b, unsigned char g, unsigned int time){


	unsigned long interval = time;
	float r_change =  _r;
	r_change -= r;
	r_change /= interval;

	float g_change =  _g;
	g_change -= g;
	g_change /= interval;

	float b_change =  _b;
	b_change -= b;
	b_change /= interval;

	//use existing values as a starting place - as floats, to allow very slow fading (change of <1 possible per iteration...)
	float r_temp = _r;
	float g_temp = _g;
	float b_temp = _b;


	long i;
	for(i = 0; i < interval; ++i){
			__delay_cycles(100);
			r_temp -= r_change;
			g_temp -= g_change;
			b_temp -= b_change;
			set_rgb(r_temp,b_temp,g_temp); //set the output to this intermediate color
	}
	//if all else failed, make sure the end result is right on target..., and update the internal linear values
	_r = r;
	_g = g;
	_b = b;
	set_rgb(r,b,g);
}



void set_lrgb(unsigned char r_new, unsigned char b_new, unsigned char g_new){ //linear RGB color space - not recommended for use!
	//update internal linear values
	_r = r_new;
	_g = g_new;
	_b = b_new;
	CCR1 = r_new*4;
        r_ccr = CCR1;
	TA1CCR1 = g_new*4;
        g_ccr = TA1CCR1;
	TA1CCR2 = b_new*4;
        b_ccr = TA1CCR2;
}

void set_rgb(unsigned char r_new, unsigned char b_new, unsigned char g_new){ //non-linear colorspace: instantly set output color
	//update internal linear values
	_r = r_new;
	_g = g_new;
	_b = b_new;

	 r_ccr = (unsigned int)r_new *(unsigned int)r_new / 64;
        CCR1 = r_ccr;
	g_ccr = (float)((float)g_new / 255)*((float)g_new / 255)*period;
        TA1CCR1 = g_ccr;
	b_ccr = (float)((float)b_new / 255)*((float)b_new / 255)*period;
        TA1CCR2 = b_ccr;

}

#endif /* LED_TILE_H_ */


