// Sine wave inverter with feedback control
// Design by Youtube:  ElectroMux
// https://www.facebook.com/groups/889800674889268

int i = 0;
int x = 0;
bool OK = 0;
#define led_on digitalWrite(2, HIGH)
#define led_off digitalWrite(2, LOW)

float h = 0;
int volvalue=0;
int battvalue=1024;
int temp=0;
int sinPWM[] = {0, 16, 32, 48, 65, 81, 97, 113, 129, 145, 161, 177, 192, 208, 224, 239, 255, 270, 286, 301, 316, 331, 346, 361, 376, 390, 405, 419, 434, 448, 462, 475, 489, 503, 516, 529, 542, 555, 567, 580, 592, 604, 616, 628, 639, 651, 662, 672, 683, 693, 704, 714, 723, 733, 742, 751, 760, 768, 777, 785, 793, 800, 807, 814, 821, 828, 834, 840, 845, 851, 856, 861, 865, 870, 874, 877, 881, 884, 887, 890, 892, 894, 896, 897, 898, 899, 900, 900, 900, 900, 899, 898, 897, 896, 894, 892, 890, 887, 884, 881, 877, 874, 870, 865, 861, 856, 851, 845, 840, 834, 828, 821, 814, 807, 800, 793, 785, 777, 768, 760, 751, 742, 733, 723, 714, 704, 693, 683, 672, 662, 651, 639, 628, 616, 604, 592, 580, 567, 555, 542, 529, 516, 503, 489, 475, 462, 448, 434, 419, 405, 390, 376, 361, 346, 331, 316, 301, 286, 270, 255, 239, 224, 208, 192, 177, 161, 145, 129, 113, 97, 81, 65, 48, 32, 16};
void setup() {
  Serial.begin(9600);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(12, INPUT_PULLUP);
  cli();
  TIMSK1=(0<<ICIE1) | (0<<OCIE1B) | (0<<OCIE1A) | (0<<TOIE1);
  TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
  TCCR1B=(0<<ICNC1) | (0<<ICES1) | (1<<WGM13) | (1<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10);
  TCNT1=0x00;
  ICR1=900;
  OCR1A=0x00;
  OCR1B=0x00;
  sei();
}
ISR(TIMER1_OVF_vect) {
  
  if (i > 175) {
    i = 0;
    OK = !OK;
    if(OK==0) TCCR1A=(1<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
    else      TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (1<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
    
    if(volvalue>512 && h>0.1)  h-=0.01;
    if(volvalue<512 && h<0.99) h+=0.01;
  }

  x = h * sinPWM[i];
  i = i + 1;
  if (OK == 0) {
    OCR1A = x;
  }
  if (OK == 1) {
    OCR1B = x;
  }
  if(i==140)
  {
    volvalue=analogRead(A0);
  }
}

void loop() {
  if(digitalRead(12)==LOW) 
  {
    Serial.print("Button press");
    while(digitalRead(12)==LOW){}
    Inverter();
  }



}
 int Inverter()
{
  delay(500);
  temp=0;
  led_on;
  h=0.1;
  TCCR1A=(1<<COM1A1) | (0<<COM1A0) | (1<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
  TIMSK1=0x01;
  OCR1A=OCR1B=0x00;
  while(1)
   {

    if(digitalRead(12)==LOW)  // Button off
        {
          while(digitalRead(12)==LOW){}
          led_off;
          while(1)
          {
            TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
            TIMSK1=0x00;
            OCR1A=OCR1B=0x00;
            if(digitalRead(12)==LOW) 
            {
              while(digitalRead(12)==LOW){}
              Inverter();
            }
          }
        }
     if(h>0.99 && volvalue<50)  //Short protect
        {
          TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
          TIMSK1=0x00;
          OCR1A=OCR1B=0x00;
          while(1)
          {
            led_on;
            delay(200);
            led_off;
            delay(200);
            if(digitalRead(12)==LOW) 
            {
              while(digitalRead(12)==LOW){}
              Inverter();
            }
          }
       }
   }
   
}
