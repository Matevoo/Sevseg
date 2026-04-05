#include <avr/io.h>
#include <util/delay.h>

// Compile command: avr-gcc -mmcu=atmega328p -DF_CPU=8000000UL -Os sevseg.c -o sevseg.elf && \
avr-objcopy -O ihex -R .eeprom sevseg.elf sevseg.hex && \
avrdude -c usbasp -p m328p -B 50kHz -e -U flash:w:sevseg.hex:i

// ── Display ────────────────────────────────────────────────
#define ALL_DIGITS  ((1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3))
#define DIGIT1      (1<<PB0)
#define DIGIT2      (1<<PB1)
#define DIGIT3      (1<<PB2)
#define DIGIT4      (1<<PB3)
#define SEG_DP      (1<<PD7)
#define BLANK       0x00

// ── ADC ────────────────────────────────────────────────────
#define ADC_THRESHOLD   4       // below this = pot disconnected (0–1023)
#define VAL_MIN         870     // 87.0 in tenths
#define VAL_MAX         1080    // 108.0 in tenths

// ── Segment patterns (A=bit0 ... G=bit6) ───────────────────
const uint8_t seg[] = {
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

void show(uint8_t digit, uint8_t segments) {
    PORTB |= ALL_DIGITS;    // all digits off
    PORTD  = segments;
    PORTB &= ~digit;        // enable selected digit
    _delay_ms(2);
}

int main(void) {
    // Segments as output
    DDRD   = 0xFF;
    // Digit selects as output, all off
    DDRB  |= ALL_DIGITS;
    PORTB |= ALL_DIGITS;

    // ADC: AVCC ref, ADC0 (PC0), prescaler /128
    ADMUX  = (1<<REFS0);
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

    while (1) {
        ADCSRA |= (1<<ADSC);
        while (ADCSRA & (1<<ADSC));
        uint16_t adc = ADC;

        if (adc < ADC_THRESHOLD) {
            // Pot disconnected — blank display
            show(DIGIT1, BLANK);
            show(DIGIT2, BLANK);
            show(DIGIT3, BLANK);
            show(DIGIT4, BLANK);
            continue;
        } 

        // Map 0–1023 to VAL_MIN–VAL_MAX (tenths)
        // e.g. 872 = 87.2, 1080 = 108.0
        uint16_t val = VAL_MIN + (uint16_t)((uint32_t)adc * (VAL_MAX - VAL_MIN) / 1023);
        show(DIGIT1, (val >= 1000) ? seg[1] : BLANK);
        show(DIGIT2, seg[(val % 1000) / 100]);
        show(DIGIT3, seg[(val % 100)  /  10] | SEG_DP);
        show(DIGIT4, seg[ val % 10]);
        
    }
}
