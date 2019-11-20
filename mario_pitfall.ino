#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 20 chars and 4 line display


// IMPORTANT: LEVELS MUST START AND END WITH DARK MATTER: \x3\x3. This avoids walking off the edge of the level
// Refer to setup() for custom characters
String level1Top = String("\x3\x3                     \x7            \x6\x3\x3"); // Level 1 top half of screen
String level1Bot = String("\x3\x3____\x5\x5____\x4__\x7_\x4_\x5\x5\x7___\x5\x5\x5\x7____\x4__\x6\x3\x3"); // Level 1 bottom half of screen

String level2Top = String("\x3\x3              \x3\x3"); // Level 2 top half of screen
String level2Bot = String("\x3\x3______\x5_______\x3\x3"); // Level 2 bottom half of screen

String topDisplay; // Level substring currently at the top of the display
String botDisplay; // Level substring currently at the bottom of the display

// Set the pin numbers as constants (they never change)
#define upPin 3
#define downPin 4
#define leftPin 2
#define rightPin 5
#define aPin 6
#define speakerPin A0

// used for button values, high or low depending on whether pressed
int upVAL = 0;
int downVAL = 0;
int leftVAL = 0;
int rightVAL = 0;
int aVAL = 0;

// The game starts at level 1
int level = 1; 

int screenPos = 0; // The position of the level on the screen. Starts at 0
int playerScreenPos = 2; // The position the player sits on the screen (e.g. 2 squares across)
boolean isJumping = false; // True when player is in the air

// Used for collision detection and avoidance
char objectToLeft; // Symbol in the screen block to the left of the player
char objectToRight; // Symbol in the screen block to the right of the player
char objectAbove; // Symbol in the screen block above the player
char objectBelow; // Symbol in the screen block below the player
char objectInside; // Symbol in the same screen block as the player

// Set left and right as constants (-1 to move the screen left and +1 to move the screen right)
#define left -1
#define right 1

unsigned long moveTimePlayer = 175; // Time each player move takes
unsigned long nextMoveTimePlayer = 0; // The time the next move can occur

unsigned long jumpTimePlayer = 750; // Time the player is in the air for
unsigned long endJumpTimePlayer = 0; // The time when the player's jump ends
unsigned long nextJumpTimePlayer = 0; // The next time the player can jump

unsigned long forcedRedrawTime = 150; // Time there is a forced redraw (used to end jumps)
unsigned long nextForcedRedrawTime = 0; // The next time a forced redraw will occur

// Mario symbol design (5x8 matrix), 1 turns a pixel on
byte mario[8] = {
        B00000,
        B01110,
        B01011,
        B01001,
        B01110,
        B01010,
        B01101,
        B10001
};

// A full block symbol design (5x8 matrix), 1 turns a pixel on
byte fullblock[8] = {
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111
};

// Finishing flag symbol design (5x8 matrix), 1 turns a pixel on
byte finishFlag[8] = {
        B10101,
        B01010,
        B10101,
        B01010,
        B10101,
        B01010,
        B10101,
        B01010
};

// Empty pit symbol design (5x8 matrix), 1 turns a pixel on. A space a player falls into and dies
byte emptyPit[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000
};

// Dark matter symbol design (5x8 matrix), 1 turns a pixel on. Empty space where a player can't walk
byte darkMatter[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000
};

// Random symbols you may want to use
uint8_t note[8] = {0x2,0x3,0x2,0xe,0x1e,0xc,0x0};
uint8_t clock[8] = {0x0,0xe,0x15,0x17,0x11,0xe,0x0};
uint8_t duck[8] = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};

/**
  * Runs on Arduino launch
  */
void setup() {
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  
  // Create our custom symbols
  lcd.createChar(1, note); // Use as `\x1` to display in a string
  lcd.createChar(2, clock); // Use as `\x2` to display in a string
  lcd.createChar(3, darkMatter); // Use as `\x3` to display in a string
  lcd.createChar(4, duck); // Use as `\x4` to display in a string
  lcd.createChar(5, emptyPit); // Use as `\x5` to display in a string
  lcd.createChar(6, finishFlag); // Use as `\x6` to display in a string
  lcd.createChar(7, fullblock); // Use as `\x7` to display in a string
  lcd.createChar(8, mario); // Use as `\x8` to display in a string
  
  Serial.begin(9600); // Set the data rate to 9600bps for serial data transmission

  pinMode(upPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  pinMode(leftPin, INPUT_PULLUP);
  pinMode(rightPin, INPUT_PULLUP);
  pinMode(aPin, INPUT_PULLUP);
  pinMode(speakerPin, OUTPUT);
  
  reDraw(); // Draw the game
  resetGame(); // Reset the game to begin on level 1
}

/**
  * The main Arduino loop - runs continuously
  */
void loop() {
  // Read the values from each pin, LOW means pressed
  upVAL = digitalRead(upPin);
  downVAL = digitalRead(downPin);
  leftVAL = digitalRead(leftPin);
  rightVAL = digitalRead(rightPin);
  aVAL = digitalRead(aPin);


  
  // If 'a' button is pressed, jump
  if (aVAL == LOW) {
    if (isNextMoveValid(1)) { // Only if the move is possible
      jump();
    }
  }
  
  // We use this to end a jump, falling to the floow
  if (millis() > endJumpTimePlayer) {
    if (isNextMoveValid(2)) { // Only if the move is possible
      isJumping = false;
    }
  }
  
  if (upVAL == LOW) {
     // Do Nothing
  } else if (downVAL == LOW) {
     // Do Nothing
  } else if (leftVAL == LOW) { // If left button is pressed, move left
    if (isNextMoveValid(3)) { // Only if the move is possible
      moveScreen(left);
    }
  } else if (rightVAL == LOW) { // If right button is pressed, move right
    if (isNextMoveValid(4)) { // Only if the move is possible
      moveScreen(right);
    }
  }
  
  // Used as a forced redraw
  if (millis() > nextForcedRedrawTime) {
     reDraw(); // Redraw the entire game
     nextForcedRedrawTime = millis() + forcedRedrawTime; // Set the next time the game will redraw
  }

}

/**
  * Moves screen right or left, uses left and right constants (-1 +1)
  */
void moveScreen(int dir) {
  if (millis() > nextMoveTimePlayer) { // If enough time has passed for the player to move
    screenPos += dir; // Change the screen position, +1 or -1
    reDraw(); // Redraw the entire game
    nextMoveTimePlayer = millis() + moveTimePlayer; // Set the next time the player can move
  } 
}   

/**
  * Redraws the entire game. Draws the level, then the player ontop. Also carries out the important
  * collision detection stuff.
  */
void reDraw () {
  drawLevel(); // Draw the background level
  drawPlayer(); // Lastly, draw the player ontop
  mapObjectsAroundPlayer(); // Works out the game objects all around the player. Used to work out if next move is valid
  collisionDetection();  // Finds out if the player has collided with a game object they're ontop of
}

/**
  * Draws the level. Uses `screenPos` to work out what section of level should be displayed. 0 displays the start
  */
void drawLevel() {
 
  switch (level) {
    case 1: // Draws level 1
      topDisplay = level1Top.substring(screenPos,(screenPos+16)); // Get the top substring of the current position of the level (Level 1) 16 screen characters
      botDisplay = level1Bot.substring(screenPos,(screenPos+16)); // Get the bottom substring of the current position of the level (Level 1) 16 screen characters
      break;
    case 2: // Draws level 2
      topDisplay = level2Top.substring(screenPos,(screenPos+16)); // Get the top substring of the current position of the level (Level 2) 16 screen characters
      botDisplay = level2Bot.substring(screenPos,(screenPos+16)); // Get the bottom substring of the current position of the level (Level 2) 16 screen characters
      break;
  }

  lcd.setCursor(0,2); // Set cursor to the top left of the screen
  lcd.print(topDisplay); // Display the top half of the level
      
  lcd.setCursor(0,3); // set cursor to the bottom left of the screen
  lcd.print(botDisplay); // Display the bottom ohalf of the level
}

/**
  * Draws the player in the game. Uses the saved position of the player on the screen (currently, always 2), and at the top
  * or bottom of the screen depending on whether the player is jumping.
  */
void drawPlayer() {
  if (isJumping) {
    lcd.setCursor(playerScreenPos,2); // top row
  } else {
    lcd.setCursor(playerScreenPos,3); // bottom row
  }
  
  lcd.print("\x8"); // \x8 is the player character (mario)
}

/**
  * Works out the symbols that are currently around the player. 
  * Used for collision 'collisionDetection()' and pre-collision detection `isNextMoveValid()'
  * 
  */
void mapObjectsAroundPlayer() {
  if (isJumping) { // We don't need to look ontop of the player
    objectBelow = botDisplay[playerScreenPos]; // Object below the player
    objectToLeft = topDisplay[playerScreenPos-1]; // Object to left of player
    objectToRight = topDisplay[playerScreenPos+1]; // Object to right of player
    objectInside = topDisplay[playerScreenPos]; // Object inside player
    
    objectAbove = '0'; // Can disregard as we are jumping
  } else { // We don't need to look underneath the player
    objectAbove = topDisplay[playerScreenPos]; // Object above the player
    objectToLeft = botDisplay[playerScreenPos-1]; // Object to the left of the player
    objectToRight = botDisplay[playerScreenPos+1]; // Object to the right of the player
    objectInside = botDisplay[playerScreenPos];  // Object inside the player
    
    objectBelow = '0'; // can disregard as we're on the floor
  } 
}

/**
  * Checks if next move is valid - pre-collision detection
  *
  * Returns true if possible to move to the next square
  */
boolean isNextMoveValid(int instruction) {
  switch (instruction) {
    case 1: // Up
      if ((objectAbove == '\x7') || (objectAbove == '\x3'))  { // \x3 and \x7 are full blocks and dark matter. They are impassable
        return false; // Can't pass
      } else {
        return true; // Can pass
      }
      break;
    case 2: // Down
      if ((objectBelow == '\x7') || (objectBelow == '\x3')) { // \x3 and \x7 are full blocks and dark matter. They are impassable
        return false; // Can't pass
      } else {
        return true; // Can pass
      }
      break;
    case 3: // Left
      if ((objectToLeft == '\x7') || (objectToLeft == '\x3')) { // \x3 and \x7 are full blocks and dark matter. They are impassable
        return false; // Can't pass
      } else {
        return true; // Can pass
      }
      break;
    case 4: // Right
      if ((objectToRight == '\x7') || (objectToRight == '\x3')) { // \x3 and \x7 are full blocks and dark matter. They are impassable
        return false; // Can't pass
      } else {
        return true; // Can pass
      }
      break;
    default: // If new kind of instruction is detected (probably a new button you've added), allow it by default
      return true;
  }

}

/**
  * Does all the business of collision detection between the player and map
  * If a deadly object is inside the same square as the player, displays game over and resets the game to level 1
  */
void collisionDetection() {
  switch (objectInside) {
    case '\x4': // Attacked by a duck
      gameOver();
      resetGame();
      break;
    case '\x5': // Fallen into a pit
      gameOver();
      resetGame();
      break;
    case '\x6': // Finished the level
      levelComplete();
      loadNextLevel();
      break;
  }
}

/**
  * Makes the player jump
  */
void jump() {
  if (millis() > nextJumpTimePlayer) { // If enough time has passed for the player to jump
    isJumping = true; // Set to true so we know the player is currently jumping
    nextJumpTimePlayer = millis() + (jumpTimePlayer * 1.5); // Next jump time is longer than jump. This is to avoid jetpack, hovering effect
    endJumpTimePlayer = millis() + jumpTimePlayer; // Calculate when the jump should end
    reDraw(); // Redraw the entire game to reflect the player jumping
  }
}

/**
  * Flashes `game over` on the screen, with a sad tune
  */
void gameOver() {
  lcd.setCursor(3,0);
  
  int dur = 100;
  int freq = 80;
  tone(speakerPin, freq, dur); // Plays a tone for 100 milliseconds at 80Hz
  
  delay(500);
  lcd.clear();
  lcd.print("GAME OVER"); // Display 'game over' on the screen
  
  dur = 100;
  freq = 60;
  tone(speakerPin, freq, dur); // Plays a tone for 100 milliseconds at 60Hz
  
  delay(500);
  lcd.clear();
  
  dur = 1000;
  freq = 40;
  tone(speakerPin, freq, dur); // Plays a tone for 1000 milliseconds at 40Hz
  
  delay(500);
  lcd.print("GAME OVER"); // Display 'game over' on the screen
  delay(500);
  lcd.clear();
  delay(500);
}

/**
  * Resets the game to level 1
  */
void resetGame() {
  level = 1; // Reset the game back to level 1
  screenPos = 0; // Reset the level screen position to 0 (the start)
  
  lcd.setCursor(2,0);
  lcd.print("LEVEL 1"); // Display `level 1` on the screen
  delay(500);
}

/**
  * Flashes 'level complete' on the screen. 
  */
void levelComplete() {
  lcd.setCursor(2,0);
  delay(500);
  lcd.clear();
  lcd.print("LEVEL COMPLETE!");
  delay(500);
  lcd.clear();
  delay(500);
  lcd.print("LEVEL COMPLETE!");
  delay(500);
  lcd.clear();
  delay(500);
}

/**
  * Load the next level in the game
  */
void loadNextLevel() {
  level++; // Increment the level by 1
  screenPos = 0; // Reset the level screen position to 0
  
  lcd.setCursor(2,0);
  lcd.print("LEVEL ");
  lcd.print(level, DEC); // Display 'LEVEL #' at the top of the screen
  delay(500);
}
