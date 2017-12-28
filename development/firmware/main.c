#include <avr/io.h> 
#include <avr/wdt.h> 
#include <util/delay.h>
#include <math.h>

#include "frequency_counter_PCI.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define MAX_FREQUENCY_DROP        600
#define MAX_EXPECTED_FREQ_NOISE   30
#define CL_WEIGHT                 0.2
#define BL_WEIGHT                 0.995

// Both a current line and a baseline frequency maintained
float current_line = 0;
float baseline = 0; 
 
/* ------------------------------------------------------------------------- */

int main(void)
{
	// Set up PWM for Timer0 (PD5, PD6: pin 5, 6) and Timer1 (PB1, PB2: pin 9, 10). 
    // Timer2 is used by frequency counter

	// set none-inverting mode
    TCCR0A |= _BV(COM0A1) | _BV(COM0B1); 
	// set fast PWM Mode
    TCCR0A |= _BV(WGM01) | _BV(WGM00);
	// set prescaler to 64 (Arduino default)
    TCCR0B |= _BV(CS01) | _BV(CS00);
    
	// set none-inverting mode
    TCCR1A |= _BV(COM1A1) | _BV(COM1B1);
	// set Fast PWM mode (8 bit)	
    TCCR1A |= _BV(WGM10) | _BV(WGM12);
    // set prescaler to 64 (Arduino default)
    TCCR1B |= _BV(CS01) | _BV(CS00);
  
	// Set pin directions

	DDRB = 0b00000110; // PWM output: PB1, PB2	
	DDRD = 0b01100000; // PWM output: PD5, PD6
	DDRC = 0b00000000;

	PORTD = 0b01100000; 

	// Enable watchdog. Can be important for USB
    wdt_enable(WDTO_1S); 

	// Fake USB disconnect for > 250 ms 
    uint8_t i = 0;
    while(--i){             
        wdt_reset();
        _delay_ms(1);
    } 

	current_line = baseline = count_frequency_INT1(); 

	// Main event loop
    for(;;){                
        wdt_reset(); 

		unsigned long f = count_frequency_INT1();

		// Current line is calculated by an exponential filter with a small weight -> changes fast
		current_line = current_line * CL_WEIGHT + (1 - CL_WEIGHT) * f;
		// Baseline is calculated by an exponential filter with a big weight -> changes slow
		baseline = baseline * BL_WEIGHT + (1 - BL_WEIGHT) * f;

		// distance is reciprocal, bigger value indicates smaller distance
		// and its range is 0-255 
		int diff = max(0, baseline - current_line);
		if(diff < MAX_EXPECTED_FREQ_NOISE)
		{ 
			diff = 0;
		}
		else
		{
			diff -= MAX_EXPECTED_FREQ_NOISE;
		}
		int distance = (diff) * 255L / MAX_FREQUENCY_DROP;
		distance = min(distance, 255);

		// Set PWM outputs
		OCR0A = distance;
		OCR0B = distance;
		OCR1A = distance;
		OCR1B = distance;
	}
}

/* ------------------------------------------------------------------------- */
