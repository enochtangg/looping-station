#include <LiquidCrystal.h>
LiquidCrystal lcd(8,9,4,5,6,7);

float bpm = 60;
bool set_bpm_state = true;

void setup() {
  // put your setup code here, to run once:
  lcd.begin(12,2);
  lcd.setCursor(0,0);
  lcd.print("Select a BPM");
  lcd.setCursor(0,1);
  lcd.print("Current BPM:");
  lcd.println(bpm);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (set_bpm_state) {
    int button_pressed;
    float rounded_bpm;
    button_pressed = analogRead(0);
    lcd.setCursor(0,1);
  
    if (button_pressed < 200) {
      bpm += 0.1;
      rounded_bpm = round(bpm);
      Serial.println("Up button pressed");
      lcd.print("Current BPM:");
      lcd.println(rounded_bpm);
    }
    else if (button_pressed < 400) {
      bpm -= 0.1;
      rounded_bpm = round(bpm);
      Serial.println("Down button pressed");
      lcd.print("Current BPM:");
      lcd.println(rounded_bpm);
    }
    else if (button_pressed < 800) {
      Serial.println("Select button pressed");
      set_bpm_state = false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("BPM set to:");
      lcd.println(rounded_bpm);
      lcd.setCursor(0,1);
      lcd.print("Start Looping!");
    }
  }
  
}
