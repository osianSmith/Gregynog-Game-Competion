#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

// pin connection
#define up 3
#define down 4
#define left 2
#define right 5
#define a 6
#define speaker A0

// used for button values, high or low depending on whether pressed
int upVAL = 0;
int downVAL = 0;
int leftVAL = 0;
int rightVAL = 0;
int aVAL = 0;

// player intial position
int yPlayer = 1;
int xPlayer = 0;

// player points
int points = 0;

// non-player charachter (NPC) initial position
int yNPC = 1;
int xNPC = 19;

// NPC symbols
char npc = '$';
char npcBad = '?';
boolean isNPCbad = false;

// create the player character
byte player[8] = {
  B01110,
  B01110,
  B00100,
  B11111,
  B10101,
  B00100,
  B11111,
  B10001,
};

char playerChar; // Used to store/call the player symbol

unsigned long moveTimePlayer = 175; // Number of milliseconds for Player to move
unsigned long nextMoveTimePlayer = 0;

unsigned long moveTimeNPC = 1000; // Number of milliseconds for NPC to move
unsigned long nextMoveTimeNPC = 0;

void setup() {
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  
  lcd.createChar(0, player);
  playerChar = 0; // Set the player char as the player symbol
  
  Serial.begin(9600);

  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(left, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  pinMode(a, INPUT_PULLUP);
  pinMode(speaker, OUTPUT);
  
  reDraw();
}
 
void loop() {
  detectButtonPresses();
  
  if (millis() > nextMoveTimeNPC) { // If enough time has passed for the NPC to move again
    moveNPC();
    nextMoveTimeNPC = millis() + moveTimeNPC; // Next time NPC can move is current timestamp plus the time it takes for an NPC to move (e.g. 1000 milliseconds)
    reDraw();
  }
}

/**
  * Detects which buttons the player is pressing
  */
void detectButtonPresses() {
  upVAL = digitalRead(up);
  downVAL = digitalRead(down);
  leftVAL = digitalRead(left);
  rightVAL = digitalRead(right);
  aVAL = digitalRead(a);

  if (upVAL == LOW) { // 'Up' button has been pressed
     movePlayer(0, -1);
  } else if (downVAL == LOW) { // 'Down' button has been pressed
     movePlayer(0, 1);
  } else if (leftVAL == LOW) { // 'Left' button has been pressed
     movePlayer(-1, 0);
  } else if (rightVAL == LOW) { // 'Right' button has been pressed
     movePlayer(1, 0);
  } else if (aVAL == LOW) { // 'A' button has been pressed
    // Do nothing
  }
  
}

/**
  * This function moves
  * the player position
  * at x and y axis.
  */
void movePlayer(int x, int y) {
  if (millis() > nextMoveTimePlayer) { // If enough time has passed for the Player to move again
    xPlayer += x;
    yPlayer += y;
    
    // Constrain the player by the bounds of the screen
    yPlayer = constrain(yPlayer, 1, 3);
    xPlayer = constrain(xPlayer, 0, 19);
    
    reDraw();
    nextMoveTimePlayer = millis() + moveTimePlayer; // Next time Player can move is current timestamp plus the time it takes for a Player to move (e.g. 175 milliseconds)
  } 
}

/**
  * Move the NPC
  * at x,y position 
  */
void moveNPC(){
  int xOrY = random(5); // 1 case out of 5 is 20% chance of moving up or down
  
  int amountToMove;
  
  switch(xOrY) {
    case 0: // Y 
      amountToMove = random(-1,2); // Random number from -1 to 1
      yNPC += amountToMove;
      yNPC = constrain(yNPC, 1, 3); // Constrain by the bounds of the screen
      break;
    default: // X
      int weightingRandom = random(1,101); // Depending on the NPC's position on the screen, it's next move is weighted. This is to use the full screen
      
      if (xNPC >= 12) { // Probably want to move left
        if (weightingRandom <= 20) { // 20% Chance of going right
          amountToMove = 1;
        } else { // 80% Chance of going left
          amountToMove = -1;
        }
      } else if (xNPC >= 8) { // Probably want to move left more
        if (weightingRandom <= 40) { // 40% Chance of going right
          amountToMove = 1;
        } else { // 60% Chance of going left
          amountToMove = -1;
        }
      } else if (xNPC <= 7) { // Probably want to move right
        if (weightingRandom <= 40) { // 40% Chance of going left
          amountToMove = -1;
        } else { // 60% Chance of going right
          amountToMove = 1;
        }
      }  else if (xNPC <= 3) { // Probably want to move right more
        if (weightingRandom <= 20) { // 20% Chance of going left
          amountToMove = -1;
        } else { // 80% Chance of going right
          amountToMove = 1;
        }
      }
      
      xNPC += amountToMove;
      xNPC = constrain(xNPC, 0, 19); // Constrain by the bounds of the screen
      break;
   }
   
   // Is NPC going to be good or bad????
   
   // Generate a random number, 0 to 4
   int randomNo = 0;
   randomNo = random(5);
  
   // (1 out of 5) 20% chance of bad NPC being drawn to the screen
   if(randomNo == 0){
     isNPCbad = true; // Save the NPC state as bad
   } else{
     isNPCbad = false; // Save the NPC state as good
   }
}


/**
  * reDraw functions, 
  * displays the points, player, character 
  * and NPC
  */
void reDraw() {
  // Clear the LCD screen
  lcd.clear();
  
  // Draw Points
  drawPoints();
  
  // Draw Player
  drawPlayer();
  
  // Work out whether Player and NPC are ontop of eachother
  if (playerNPCCollision()){ // Player and NPC at same position, give point and respawn NPC
    
    lcd.setCursor(0,3); // Set the LCD cursor position to bottom left to draw points given
  
    if(isNPCbad){ // We know NPC is bad, lose a point
      points -= 1;   
      lcd.print("-1");
   
      // Play random, long low tone between 50 and 100Hz
      int dur = random(50) + 50;
      int freq = random(50) + 50;
      
      tone(speaker, freq, dur); // Plays a tone
      delay(dur);  
    } else { // NPC is good, gain a point
      points += 1; 
      lcd.print("+1");
      
      // Play 25 random tones, one after another
      for(int i = 0; i < 25; i++) {    
        int dur = random(15) + 5;
        int freq = random(9080) + 20;
      
        tone(speaker, freq, dur); // Plays a tone
        delay(dur);
      }
    }
     
    // Generate a new random position for the NPC
    xNPC = random(19); // 0 to 15
    yNPC = random(1,3);  // 1 to 3
  } 
  
  // Draw the NPC
  drawNPC();
}

/**
  * This function displays the 
  * score on the top right cner
  * of the LCD screen 
  */
void drawPoints() {
  String textPoints = String(points);
  
  // Padding
  if (points > 999) {       // Quad Digits
    lcd.setCursor(16,0);
  } else if (points > 99) { // Triple Digits
    lcd.setCursor(17,0);
  } else if (points > 9) {  // Double Digits
    lcd.setCursor(18,0);
  } else if (points < -99){ // Negative Triple Digits 
    lcd.setCursor(16,0);
  } else if (points < -9){  // Negative Double Digits  
    lcd.setCursor(17,0);
  } else if (points < 0){   // Negative Single Digit
    lcd.setCursor(18,0);
  } else {                  // One Digit
    lcd.setCursor(19,0);    
  }
  
  lcd.print(textPoints);
}

/**
  * Draw the player 
  * at position x, y
  */
void drawPlayer() {
  // Set the LCD cursor position to where the Player should be drawn
  lcd.setCursor(xPlayer,yPlayer);
  
  // Print the character to the LCD.
  lcd.write(playerChar);  
}


/**
 * Draw the non-player Character
 * at position x, y
 */
void drawNPC() {
  // Set the LCD cursor position to where the NPC should be drawn
  lcd.setCursor(xNPC, yNPC);
  
  if(isNPCbad){ // If NPC is bad, draw bad NPC character
    lcd.write(npcBad);
  } else { // Otherwise, draw good NPC character (dollar sign)
    lcd.write(npc); 
  }
}


/**
 * Checks whether the
 * player charachter and NPC
 * are at the same possition and
 * returns true or false 
*/
boolean playerNPCCollision() {
  return (xNPC == xPlayer) && (yNPC == yPlayer);
}
