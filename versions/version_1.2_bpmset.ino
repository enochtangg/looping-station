#include <LiquidCrystal.h> // library for the LCD display
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // initialize the library with the numbers of the interface pins see tutorial link

const float input_bpm;
volatile bool toggle = 0;
bool is_bpm_set = 0;
float number_of_events;
float period;
float interrupt_frequency;
float bpm = 60;  
float temp_bpm = 60; 
float events;

int x;

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
  
  interrupt_frequency = bpm/60;
  events = 16000000/(1024*interrupt_frequency) - 1; 
  
  OCR1A = events; // excutes interrupt when events is reached 
  
  sei();
  Serial.begin(9600); //sets up communication wit the serial monitor
}

void loop() {
  x = analogRead(0); //Read analog input pin 0 (section 2.2, figure 2.3)
  lcd.setCursor(0, 0);
  lcd.print("BPM: ");
  lcd.println(temp_bpm);
  lcd.setCursor(0, 1);
  
  if (toggle) {
//    digitalWrite(2, HIGH);
//    delay(100);
//    digitalWrite(2, LOW);
    Serial.print("BPM: ");
    Serial.println(bpm);
    toggle = 0;
    TCNT1=0;
  }

  if (x < 50) {
    //Serial.print ("Value at pin A0 'Right Key Pressed' is : ");
    //Serial.println(x, DEC);
  }
  else if ( x < 200) {
    temp_bpm++;
    //Serial.print ("Value at pin A0 'Up Key Pressed' is : ");
    //Serial.println(x, DEC);
  }
  else if ( x < 400) {
    temp_bpm--;
    //Serial.print ("Value at pin A0 'Down Key Pressed' is : ");
    //Serial.println(x, DEC);
  }
  else if ( x < 600) {
    //Serial.print ("Value at pin A0 'Left Key Pressed' is : ");
    //Serial.println(x, DEC);
  }
  else if ( x < 800) {
    bpm = temp_bpm;
    interrupt_frequency = bpm/60;
    events = 16000000/(1024*interrupt_frequency) - 1; 
  
    OCR1A = events; // excutes interrupt when events is reached 
    //Serial.print ("Value at pin A0 'Select Key Pressed' is : ");
    //Serial.println(x, DEC);
  }
 
}

ISR (TIMER1_COMPA_vect) {
  toggle = 1;
}


