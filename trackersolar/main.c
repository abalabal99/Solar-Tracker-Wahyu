/*
 * trackersolar.c
 *
 * Created: 06/06/2024 22:21:12
 * Author : HP
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

// Histeresis threshold
#define HYSTERESIS_THRESHOLD 10

void adc_init(void) {
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, set prescaler to 128
	ADMUX = (1 << REFS0); // Set reference voltage to AVcc (5V)
}

uint16_t adc_read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF8) | (channel & 0x07); // Select ADC channel
	ADCSRA |= (1 << ADSC); // Start conversion
	while (ADCSRA & (1 << ADSC)); // Wait for conversion to complete
	return ADC;
}

void pwm_init(void) {
	DDRD |= (1 << PD5) | (1 << PD4); // Set PD5 and PD4 as output
	TCCR1A = (1 << WGM11) | (1 << COM1A1) | (1 << COM1B1); // Fast PWM, non-inverted output
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler 8
	ICR1 = 39999; // TOP value for 50Hz frequency
}

void set_servo_angle(uint8_t servo, uint8_t angle) {
	uint16_t pulse_width = ((angle * 11) + 600); // Convert angle to pulse width
	if (servo == 1) {
		OCR1A = pulse_width;
		} else if (servo == 2) {
		OCR1B = pulse_width;
	}
}

void calibrate_ldr(uint16_t *ldr_values) {
	for (uint8_t i = 0; i < 4; i++) {
		ldr_values[i] = adc_read(i);
		_delay_ms(10);
	}
}

uint8_t calculate_angle(uint16_t top, uint16_t bottom, uint16_t left, uint16_t right) {
	// Placeholder simple algorithm for demonstration; should be improved for real application
	uint8_t vertical_angle = 90;
	uint8_t horizontal_angle = 90;

	if (top > bottom + HYSTERESIS_THRESHOLD) {
		vertical_angle = 45; // Move up
		} else if (bottom > top + HYSTERESIS_THRESHOLD) {
		vertical_angle = 135; // Move down
	}

	if (left > right + HYSTERESIS_THRESHOLD) {
		horizontal_angle = 45; // Move left
		} else if (right > left + HYSTERESIS_THRESHOLD) {
		horizontal_angle = 135; // Move right
	}

	return (vertical_angle << 8) | horizontal_angle;
}

int main(void) {
	adc_init();
	pwm_init();
	
	uint16_t ldr_values[4];
	uint8_t vertical_angle = 90;
	uint8_t horizontal_angle = 90;
	
	while (1) {
		calibrate_ldr(ldr_values);
		
		uint16_t avg_top = (ldr_values[0] + ldr_values[1]) / 2;
		uint16_t avg_bottom = (ldr_values[2] + ldr_values[3]) / 2;
		uint16_t avg_left = (ldr_values[0] + ldr_values[2]) / 2;
		uint16_t avg_right = (ldr_values[1] + ldr_values[3]) / 2;
		
		uint16_t angles = calculate_angle(avg_top, avg_bottom, avg_left, avg_right);
		vertical_angle = (angles >> 8) & 0xFF;
		horizontal_angle = angles & 0xFF;
		
		set_servo_angle(1, vertical_angle);
		set_servo_angle(2, horizontal_angle);
		
		_delay_ms(100);
	}
}


