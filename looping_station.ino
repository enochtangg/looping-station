const float input_bpm;
volatile bool toggle = 0;
float number_of_events;
float period;
float interrupt_frequency;
float bpm = 60;   
float events;


void setup() {
  pinMode(2, OUTPUT); // enable pin 2 for metronome light
  cli();
  
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1A = 0x0;
  TCCR1B = 0x0;

  // set prescaler for Timer1 (metronome)
  TCCR1B |= _BV(CS02); 
  TCCR1B &= ~_BV(CS01); 
  TCCR1B |= _BV(CS00);
  
  TIMSK1 = 0x6; // enable OCR1A and OCR1B
  
  interrupt_frequency = 180/60;
  events = 16000000/(1024*interrupt_frequency) - 1; 
  
  OCR1A = events; // excutes interrupt when events is reached 
  
  sei();
  Serial.begin(9600); //sets up communication wit the serial monitor
}

void loop() {
  if (toggle) {
    digitalWrite(2, HIGH);
    delay(100);
    digitalWrite(2, LOW);
    toggle = 0;
    TCNT1=0;
  }
}

ISR (TIMER1_COMPA_vect) {
  toggle = 1;
}


