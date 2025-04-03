#include <avr/io.h>
#include <avr/interrupt.h>

//   Left bridge used for fundamental signal (50Hz/60Hz), Right bridge for SPWM (10kHz carrier freq.)
//   Sampling per Cycle
//   ---> 10kHz/50Hz = 200
//   Look Up table entries use only half cycles (identical positive and negative cycles)
//   50 Hz --->  200/2 = 100 entries
//   SPWM clock = fXTAL/freq. carrier = 16.000.000/10.000 = 1.600 clock.
//   WGM mode 8 is used, so ICR1 = 1.600/2 = 800 clk
//   Look up tables for a half cycle (100 or 83 entries), max value = 800 (100% duty cycle)is loaded into register ICR1.
//
//  This code is for 50Hz !!!

//---------------------------------Look Up Table for 50 Hz---------------------------------------
int lookUp1[] = {
  0, 25, 50, 75, 100, 125, 150, 175, 199, 223, 247, 271, 294, 318, 341, 363, 385, 407, 429, 450,
  470, 490, 510, 529, 548, 566, 583, 600, 616, 632, 647, 662, 675, 689, 701, 713, 724, 734, 744, 753,
  761, 768, 775, 781, 786, 790, 794, 796, 798, 800, 800, 800, 798, 796, 794, 790, 786, 781, 775, 768,
  761, 753, 744, 734, 724, 713, 701, 689, 675, 662, 647, 632, 616, 600, 583, 566, 548, 529, 510, 490,
  470, 450, 429, 407, 385, 363, 341, 318, 294, 271, 247, 223, 199, 175, 150, 125, 100, 75, 50, 25, 0
};


void setup() {
  Serial.begin(9600);

  // Register initilisation, see datasheet for more detail.
  TCCR1A = 0b10110000;
  /*      10xxxxxx Clear OC1A/OC1B on compare match when up-counting. Set OC1A/OC1B on compare match when down counting
          xx11xxxx Set OC1A/OC1B on compare match when up-counting. Clear OC1A/OC1B on compare match when down counting.
          xxxxxx00 WGM1 1:0 for waveform 8 (phase freq. correct).
  */
  TCCR1B = 0b00010001;
  /*      000xxxxx
          xxx10xxx WGM1 3:2 for waveform mode 8.
          xxxxx001 no prescale on the counter.
  */
  TIMSK1 = 0b00000001;
  /*      xxxxxxx1 TOV1 Flag interrupt enable. */

  ICR1 = 800; /* Counter TOP value (at 16MHz XTAL, SPWM carrier freq. 10kHz, 200 samples/cycle).*/
  // ICR1 = 255;
  sei();             /* Enable global interrupts.*/
  DDRB = 0b00011110; /* Pin 9, 10, 11, 12 as outputs.*/

  // pinMode(13,OUTPUT);
}

void loop() {
  // digitalWrite(13, HIGH);
  // delay(200);
  // digitalWrite(13, LOW);
  // delay(200);
}

/*---------------------------------------------------------------------------------------------------------*/
ISR(TIMER1_OVF_vect) {
  static int num;
  static int ph;
  static int dtA = 0;
  static int dtB = 5;

  if (num >= 99) {  // <------------------ 50 Hz !!!
    if (ph == 0) {          // OC1A as SPWM out
      TCCR1A = 0b10110000;  // clear OC1A, set OC1B on compare match
      dtA = 0;              // no dead time
      dtB = 5;              // adding dead time to OC1B
    } else {
      TCCR1A = 0b11100000;  // OC1B as SPWM out
      dtA = 5;
      dtB = 0;
    }
    ph ^= 1;
  }
  OCR1A = int(lookUp1[num] * 0.7) + dtA;  // SPWM width update
  OCR1B = int(lookUp1[num] * 0.7) + dtB;  // note: 0.7 used to reduce inveter output voltage

  num++;
  if (num >= 100) {  // toggle left bridge (50Hz) !!!
    delayMicroseconds(50);
    if (ph == 1) {
      digitalWrite(12, LOW);
      digitalWrite(11, HIGH);
    } else {
      digitalWrite(11, LOW);
      digitalWrite(12, HIGH);
    }
    num = 0;
  }
}
