#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define ALL_DIGITS  ((1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3))
#define DIGIT1      (1<<PB0)
#define DIGIT2      (1<<PB1)
#define DIGIT3      (1<<PB2)
#define DIGIT4      (1<<PB3)
#define SEG_DP      (1<<PD7)
#define BLANK       0x00

#define ADC_THRESHOLD   2

static const uint8_t seg[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
};

#define NUM_POINTS 5
uint16_t cal_adc[NUM_POINTS]  = { 0,   414, 528, 871,  1023 }; 
uint16_t cal_freq[NUM_POINTS] = { 870, 943, 966, 1048, 1080 }; // 943 = 94.3 MHz

uint16_t get_calibrated_freq(uint16_t adc_val) {
    if (adc_val <= cal_adc[0]) return cal_freq[0];
    if (adc_val >= cal_adc[NUM_POINTS - 1]) return cal_freq[NUM_POINTS - 1];

    for (uint8_t i = 0; i < NUM_POINTS - 1; i++) {
        if (adc_val >= cal_adc[i] && adc_val <= cal_adc[i+1]) {
            uint32_t adc_diff = adc_val - cal_adc[i];
            uint32_t adc_range = cal_adc[i+1] - cal_adc[i];
            uint32_t freq_range = cal_freq[i+1] - cal_freq[i];
            
            return cal_freq[i] + (uint16_t)((adc_diff * freq_range) / adc_range);
        }
    }
    return cal_freq[0];
}

static inline void show(uint8_t digit, uint8_t segments) {
    PORTB |= ALL_DIGITS;        
    _delay_us(10);
    PORTD = segments;           
    PORTB &= ~digit;    
    _delay_ms(2);
}

int main(void) { 
    DDRD   = 0xFF;
    DDRB  |= ALL_DIGITS;
    PORTB |= ALL_DIGITS;
    ADMUX  = (1<<REFS0);
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

    while (1) {
        ADCSRA |= (1<<ADSC);
        while (ADCSRA & (1<<ADSC));
        uint16_t adc = ADC;

        if (adc < ADC_THRESHOLD) {
            show(DIGIT1, BLANK);
            show(DIGIT2, BLANK);
            show(DIGIT3, BLANK);
            show(DIGIT4, BLANK);
            continue;
        }

        uint16_t val = get_calibrated_freq(adc);

        uint16_t hundreds = val / 1000;
        uint16_t tens_units = val % 1000;
        
        show(DIGIT1, hundreds ? seg[hundreds] : BLANK);
        show(DIGIT2, seg[tens_units / 100]);
        show(DIGIT3, seg[(tens_units % 100) / 10] | SEG_DP);
        show(DIGIT4, seg[val % 10]); 
    }
}
