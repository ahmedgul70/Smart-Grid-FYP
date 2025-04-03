#include <avr/io.h>
#include <avr/interrupt.h>

// Look up tables with 200 entries each, normalised to have max value of 1600 which is the period of the PWM loaded into register ICR1.
int lookUp1[] = {
  50 , 100 , 151 , 201 , 250 , 300 , 349 , 398 , 446 , 494 ,
  542 , 589 , 635 , 681 , 726 , 771 , 814 , 857 , 899 , 940 ,
  981 , 1020 , 1058 , 1095 , 1131 , 1166 , 1200 , 1233 , 1264 , 1294 ,
  1323 , 1351 , 1377 , 1402 , 1426 , 1448 , 1468 , 1488 , 1505 , 1522 ,
  1536 , 1550 , 1561 , 1572 , 1580 , 1587 , 1593 , 1597 , 1599 , 1600 ,
  1599 , 1597 , 1593 , 1587 , 1580 , 1572 , 1561 , 1550 , 1536 , 1522 ,
  1505 , 1488 , 1468 , 1448 , 1426 , 1402 , 1377 , 1351 , 1323 , 1294 ,
  1264 , 1233 , 1200 , 1166 , 1131 , 1095 , 1058 , 1020 , 981 , 940 ,
  899 , 857 , 814 , 771 , 726 , 681 , 635 , 589 , 542 , 494 ,
  446 , 398 , 349 , 300 , 250 , 201 , 151 , 100 , 50 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0
};

int lookUp2[] = {
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
  50 , 100 , 151 , 201 , 250 , 300 , 349 , 398 , 446 , 494 ,
  542 , 589 , 635 , 681 , 726 , 771 , 814 , 857 , 899 , 940 ,
  981 , 1020 , 1058 , 1095 , 1131 , 1166 , 1200 , 1233 , 1264 , 1294 ,
  1323 , 1351 , 1377 , 1402 , 1426 , 1448 , 1468 , 1488 , 1505 , 1522 ,
  1536 , 1550 , 1561 , 1572 , 1580 , 1587 , 1593 , 1597 , 1599 , 1600 ,
  1599 , 1597 , 1593 , 1587 , 1580 , 1572 , 1561 , 1550 , 1536 , 1522 ,
  1505 , 1488 , 1468 , 1448 , 1426 , 1402 , 1377 , 1351 , 1323 , 1294 ,
  1264 , 1233 , 1200 , 1166 , 1131 , 1095 , 1058 , 1020 , 981 , 940 ,
  899 , 857 , 814 , 771 , 726 , 681 , 635 , 589 , 542 , 494 ,
  446 , 398 , 349 , 300 , 250 , 201 , 151 , 100 , 50 , 0
};


void setup() {
  Serial.begin(9600);
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);


  // Register initilisation, see datasheet for more detail.
  TCCR1A = 0b10100010;
  /*10 clear on match, set at BOTTOM for compA.
    10 clear on match, set at BOTTOM for compB.
    00
    10 WGM1 1:0 for waveform 15.
  */
  TCCR1B = 0b00011001;
  /*000
    11 WGM1 3:2 for waveform 15.
    001 no prescale on the counter.
  */
  TIMSK1 = 0b00000001;
  /*0000000
    1 TOV1 Flag interrupt enable.
  */
  ICR1   = 800;     // Period for 16MHz crystal, for a switching frequency of 100KHz for 200 subdevisions per 50Hz sin wave cycle.
  sei();             // Enable global interrupts.
  DDRB = 0b00000110; // Set PB1 and PB2 as outputs.


  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

uint32_t pmillis = 0;
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 100000.0;
float R2 = 1000.0;
int value = 0;

float vOUT2 = 0.0;
float vIN2 = 0.0;
float R11 = 10000.0;
float R22 = 3300.0;
int value2 = 0;

void loop() {
  //  if (millis() - pmillis > 2500) {
  //    //    digitalWrite(13, HIGH);
  //    pmillis = millis();
  //
  ////    for (int i = 0; i < 25; i++)
  ////    {
  ////      value = (value + ((5 * analogRead(A4)) / 1024)); // (5 V / 1024 (Analog) = 0.0049) which converter Measured analog input voltage to 5 V Range
  ////      delay(2);
  ////    }
  ////    vOUT = ( value / 25);
  ////    vIN = vOUT / (R2 / (R1 + R2));
  ////    value = 0;
  //
  //    value = analogRead(A4);
  //    vOUT = (value * 5.0) / 1024.0;
  //    vIN = vOUT / (R2 / (R1 + R2));
  //
  //    value2 = analogRead(A0);
  //    vOUT2 = (value2 * 5.0) / 1024.0;
  //    vIN2 = vOUT2 / (R22 / (R11 + R22));
  //
  //
  //    Serial.print("\t AC_Volt: " + String(vIN));
  //    Serial.print("\t BAT_Volt: " + String(vIN2));
  ////    Serial.println("\t Amps: " + String(measureCurrent(A1)));
  //    //    digitalWrite(13, LOW);
  //  }
}

double measureCurrent(int pin)
{
  double Voltage = 0;
  double Current = 0;
  // Voltage is Sensed 1000 Times for precision
  for (int i = 0; i < 25; i++)
  {
    Voltage = (Voltage + ((5 * analogRead(pin)) / 1024)); // (5 V / 1024 (Analog) = 0.0049) which converter Measured analog input voltage to 5 V Range
    delay(2);
  }
  Voltage = ( Voltage / 25);
  Current = ((Voltage - 2.5) / 0.1); // Sensed voltage is converter to current   here offset volt = 2.49 because at 0A , its gives 2.49v and 66 is factor for 30A ACS712 sesnor
  if (Current < 0.2) {
    Current = 0.0;
  }
  return Current;
}


ISR(TIMER1_OVF_vect) {
  static int num;
  // change duty-cycle every period.
  OCR1A = lookUp1[num];
  OCR1B = lookUp2[num];

  if (++num >= 200) { // Pre-increment num then check it's below 200.
    num = 0;       // Reset num.
  }
}
