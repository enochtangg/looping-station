#include <LiquidCrystal.h> // library for the LCD display
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // initialize the library with the numbers of the interface pins see tutorial link

volatile bool toggle = 0;
bool SET_BPM_STATE = 1;
bool SELECT_CHANNEL_STATE = 0;
bool RECORDING_STATE = 0;
bool CHANNEL_ONE = 0;
bool CHANNEL_TWO = 0;
bool CHANNEL_THREE = 0;

bool flagged_countdown = 0;

float number_of_events;
float period;
float interrupt_frequency;
float bpm = 60;  
float rounded_bpm = 60; 
float channel = 1;
float rounded_channel = 1;
float events;
const float TIMER_FREQUENCY = 16000000;
const float PRESCALE = 1024;

const int BPM_LED_PIN = 3;
const int INTERRUPT_PIN = 2;
const int DISPLAY_PIN = 0;
const int BUTTON_PIN = 1;
const int GREEN = 1;
const int WHITE = 2;
const int RED = 3;

int display_button_resistance; // analog 0 for LCD display buttons
int note_button_resistance;
float countdown = 0;

volatile bool pressed = 0;

void setup() {
  pinMode(BPM_LED_PIN, OUTPUT); // enable pin 2 for metronome light
  pinMode(INTERRUPT_PIN, INPUT);
  digitalWrite(INTERRUPT_PIN, LOW);
  cli();

  pinMode(A2, OUTPUT); // row 1
  pinMode(A3, OUTPUT); // row 2
  pinMode(A4, OUTPUT); // row 3
  pinMode(A5, OUTPUT); // col 1
  pinMode(0, OUTPUT); // col 2
  pinMode(1, OUTPUT); // col 3

  reset_leds();

  
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1A = 0x0;
  TCCR1B = 0x0;
  TIMSK1 = 0x6; // enable OCR1A and OCR1B

  calculate_bpm(bpm);

  // set prescaler for Timer1 (metronome)
  TCCR1B |= _BV(CS02); 
  TCCR1B &= ~_BV(CS01); 
  TCCR1B |= _BV(CS00);
  
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), buttonDetector, CHANGE);
  
  lcd.begin(14,2);
  lcd_display_row("Select a BPM", -1, 0);
  lcd_display_row("Current BPM: ", rounded_bpm, 1);

  sei();
  //Serial.begin(9600); //sets up communication wit the serial monitor
}

void loop() {
  display_button_resistance = analogRead(DISPLAY_PIN); //Read analog input pin 0 (section 2.2, figure 2.3)
  
  if (pressed) {
    note_button_resistance = analogRead(BUTTON_PIN);

    if (RECORDING_STATE) {
      if (note_button_resistance > 500 && note_button_resistance < 520) {
        Serial.println("Green");
        power_LED(rounded_channel, GREEN);
      } else if (note_button_resistance > 660 && note_button_resistance < 680) {
        Serial.println("White");
        power_LED(rounded_channel, WHITE);
      } else if (note_button_resistance > 930 && note_button_resistance < 980) {
        Serial.println("Red");
        power_LED(rounded_channel, RED);
      }
    
   }
    if (note_button_resistance == 0){
      Serial.println ("stuck");
      pressed = 0;
    }
  }

  if (!pressed) {
    reset_leds();
  }
  
  if (toggle) {
    if (countdown == 5) { // finished counting down.. user can now record
      flagged_countdown = 0;
      countdown = 0;
      lcd.clear();
      lcd_display_row("Rec. Channel: ", rounded_channel, 0);
      lcd_display_row("Record!", -1, 1);
      RECORDING_STATE = 1;
      // start new timer
    }
    if (flagged_countdown) {
      lcd_display_row("Start loop in ", countdown, 1);
      countdown++;
    }
    
    if (SELECT_CHANNEL_STATE) { // IN SELCT CHANNEL STATE
      flash_bpm_led(BPM_LED_PIN, 50);
    }
    toggle = 0;
    TCNT1=0;
    
  }
  
  if (display_button_resistance > 390 && display_button_resistance < 415 && SELECT_CHANNEL_STATE) {  // LEFT (browsing channels)
    channel -= 0.03;
    if (channel < 1) {
      channel = 3;
    }
    rounded_channel = round(channel);
    
    lcd_display_row("Loop Channel: ", rounded_channel, 1);
  }
  else if (display_button_resistance < 50 && SELECT_CHANNEL_STATE) {   // RIGHT (browsing channels)
    channel += 0.03;
    if (channel > 3) {
      channel = 1;
    }
    rounded_channel = round(channel);
    
    lcd_display_row("Loop Channel: ", rounded_channel, 1);
  }
  else if (display_button_resistance > 90 &&  display_button_resistance < 110 && SELECT_CHANNEL_STATE) {   // UP (select channel)
    lcd.clear();
    lcd_display_row("Rec. Channel: ", rounded_channel, 0);
    flagged_countdown = 1;
  }
  else if (display_button_resistance > 90 && display_button_resistance < 110 && SET_BPM_STATE) {    // UP (browsing bpm)
    bpm += 0.05;
    rounded_bpm = round(bpm);
 
    lcd_display_row("Current BPM: ", rounded_bpm, 1);
  }
  else if (display_button_resistance > 250 && display_button_resistance < 270 && SET_BPM_STATE) { // DOWN (browsing bpm)
    bpm -= 0.05;
    rounded_bpm = round(bpm);
    
    lcd_display_row("Current BPM: ", rounded_bpm, 1);
  }
  else if (display_button_resistance > 600 && display_button_resistance < 700 && SET_BPM_STATE) {  // SELECT (select bpm)
    bpm = rounded_bpm;    
    calculate_bpm(bpm);
    SET_BPM_STATE = 0; // BPM HAS BEEN SET, IN LOOPING STATE RIGHT NOW
    
    lcd.clear();
    lcd_display_row("BPM set to: ", bpm, 0);
    lcd_display_row("Loop Channel: ", channel, 1);


    SELECT_CHANNEL_STATE = 1;
  }
}


void lcd_display_row (String row_string, float value, int row) {
  lcd.setCursor(0,row);
  lcd.print(row_string);
  if (value != -1){ lcd.println(value); }
}

void flash_bpm_led(int pin, int delay_time) {
  digitalWrite(pin, HIGH);
  delay(delay_time);
  digitalWrite(pin, LOW);
}

void calculate_bpm(float input_bpm) {
  toggle = 0;
  TCNT1=0;
  interrupt_frequency = input_bpm/60;
  events = TIMER_FREQUENCY/(PRESCALE * interrupt_frequency) - 1; 
  
  OCR1A = events; // excutes interrupt when events is reached 
}

void buttonDetector() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 100) {
    pressed = !pressed;
  }
  
  last_interrupt_time = interrupt_time;
}

void power_LED(int channel, int color) {
  if (channel == 1) {
    digitalWrite(A2, HIGH);
    if (color == GREEN) {
      digitalWrite(A5, LOW);
    }
    else if (color == WHITE) {
      digitalWrite(0, LOW);
    }
    else if (color == RED) {
      digitalWrite(1, LOW);
    }
  }
  else if (channel == 2) {
    digitalWrite(A3, HIGH);
    if (color == GREEN) {
      digitalWrite(A5, LOW);
    }
    else if (color == WHITE) {
      digitalWrite(0, LOW);
    }
    else if (color == RED) {
      digitalWrite(1, LOW);
    }
  }
  else if (channel == 3) {
    digitalWrite(A4, HIGH);
    if (color == GREEN) {
      digitalWrite(A5, LOW);
    }
    else if (color == WHITE) {
      digitalWrite(0, LOW);
    }
    else if (color == RED) {
      digitalWrite(1, LOW);
    }
  }
}

void reset_leds() {
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  digitalWrite(A4, LOW);
  digitalWrite(A5, HIGH);
  digitalWrite(0, HIGH);
  digitalWrite(1, HIGH);
}

ISR (TIMER1_COMPA_vect) {
  toggle = 1;
}




