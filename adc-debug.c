#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// ── Display ────────────────────────────────────────────────
#define ALL_DIGITS  ((1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3))
#define DIGIT1      (1<<PB0)
#define DIGIT2      (1<<PB1)
#define DIGIT3      (1<<PB2)
#define DIGIT4      (1<<PB3)
#define BLANK       0x00

// ── ADC ────────────────────────────────────────────────────
#define ADC_THRESHOLD   2       // below this = pot disconnected

// ── Segment patterns (A=bit0 ... G=bit6) ───────────────────
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

static inline void show(uint8_t digit, uint8_t segments) {
    PORTB |= ALL_DIGITS;        // all digits off
    _delay_us(10);
    PORTD = segments;           // Set segments (bez DP)
    PORTB &= ~digit;            // enable selected digit
    _delay_ms(2);
}

int main(void) { 
    // Segments as output
    DDRD   = 0xFF;
    // Digit selects as output, all off initially
    DDRB  |= ALL_DIGITS;
    PORTB |= ALL_DIGITS;

    // ADC: AVCC reference, ADC0 (PC0), prescaler /128
    ADMUX  = (1<<REFS0);
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

    while (1) {
        // Start conversion
        ADCSRA |= (1<<ADSC);
        while (ADCSRA & (1<<ADSC));
        uint16_t adc = ADC;

        if (adc < ADC_THRESHOLD) {
            // Pot disconnected – blank all digits
            show(DIGIT1, BLANK);
            show(DIGIT2, BLANK);
            show(DIGIT3, BLANK);
            show(DIGIT4, BLANK);
            continue;
        }

        // --- DEBUG ZOBRAZENIE SUROVEJ HODNOTY ADC ---
        uint16_t val = adc; 

        // Rozdelenie na jednotlivé číslice
        uint8_t tisicky = (val / 1000) % 10;
        uint8_t stovky = (val / 100) % 10;
        uint8_t desiatky = (val / 10) % 10;
        uint8_t jednotky = val % 10;

        // Vypisovanie na displej s ignorovaním núl na začiatku
        show(DIGIT1, (val >= 1000) ? seg[tisicky] : BLANK);
        show(DIGIT2, (val >= 100)  ? seg[stovky]  : BLANK);
        show(DIGIT3, (val >= 10)   ? seg[desiatky]: BLANK); 
        show(DIGIT4, seg[jednotky]); 
    }
}
