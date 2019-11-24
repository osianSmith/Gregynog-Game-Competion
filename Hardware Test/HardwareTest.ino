#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

int main_up = 3;
int main_down = 4;
int main_left = 2;
int main_right = 5;
int main_a = 6;
int main_b = 7;

int cont_up = 9;
int cont_down = 10;
int cont_left = 8;
int cont_right = 11;
int cont_a = 12;
int cont_b = 13;

int a3 = A3;
int a2 = A2;
int a1 = A1;
int a0 = A0;

int speakerPin = A0;


void setup()
{
  Serial.begin(9600);

  pinMode(main_up, INPUT_PULLUP);
  pinMode(main_down, INPUT_PULLUP);
  pinMode(main_left, INPUT_PULLUP);
  pinMode(main_right, INPUT_PULLUP);
  pinMode(main_a, INPUT_PULLUP);
  pinMode(main_b, INPUT_PULLUP);

  pinMode(cont_up, INPUT_PULLUP);
  pinMode(cont_down, INPUT_PULLUP);
  pinMode(cont_left, INPUT_PULLUP);
  pinMode(cont_right, INPUT_PULLUP);
  pinMode(cont_a, INPUT_PULLUP);
  pinMode(cont_b, INPUT_PULLUP);

  pinMode(a0, INPUT_PULLUP);
  pinMode(a1, INPUT_PULLUP);
  pinMode(a2, INPUT_PULLUP);
  pinMode(a3, INPUT_PULLUP);

  pinMode(speakerPin, OUTPUT);

  lcd.init();                      // initialize the lcd
  //lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 2);
  lcd.print("hello everyone");
  lcd.setCursor(0, 3);
  lcd.print("konichiwaa");

  int dur = 100;
  int freq = 80;
  tone(speakerPin, freq, dur); // Plays a tone for 100 milliseconds at 80Hz
  
  delay(500);
  
  dur = 100;
  freq = 60;
  tone(speakerPin, freq, dur); // Plays a tone for 100 milliseconds at 60Hz
  
  delay(500);
  
  dur = 1000;
  freq = 40;
  tone(speakerPin, freq, dur); // Plays a tone for 1000 milliseconds at 40Hz
}


void loop()
{
//  Serial.println(digitalRead(a3));
  //delay(500);
  lcd.setCursor(0, 0);

  if (digitalRead(main_up) == LOW) {
    lcd.print("main_up");
    delay(200);
  } else if (digitalRead(main_down) == LOW) {
    lcd.print("main_down");
    delay(200);
  } else if (digitalRead(main_left) == LOW) {
    lcd.print("main_left");
    delay(200);
  } else if (digitalRead(main_right) == LOW) {
    lcd.print("main_right");
    delay(200);
  } else if (digitalRead(main_a) == LOW) {
    lcd.print("main_a");
    delay(200);
  } else if (digitalRead(main_b) == LOW) {
    lcd.print("main_b");
    delay(200);
  } else if (digitalRead(cont_up) == LOW) {
    lcd.print("cont_up");
    delay(200);
  } else if (digitalRead(cont_down) == LOW) {
    lcd.print("cont_down");
    delay(200);
  } else if (digitalRead(cont_left) == LOW) {
    lcd.print("cont_left");
    delay(200);
  } else if (digitalRead(cont_right) == LOW) {
    lcd.print("cont_right");
    delay(200);
  } else if (digitalRead(cont_a) == LOW) {
    lcd.print("cont_a");
    delay(200);
  } else if (digitalRead(cont_b) == LOW) {
    lcd.print("cont_b");
    delay(200);
  } else {
    lcd.print("                    ");
  }
}
