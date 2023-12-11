#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
//ANIMATON LOGIC IS IMPLEMENTED WITH CHATGPT!

const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
const int xPin = A0;
const int yPin = A1;
const int swPin = 2;

bool introDisplayed = false;
unsigned long introStartTime = 0;
const unsigned long introDuration = 3000; // 3 seconds

const int totalMenuItems = 3; // Total number of items in the menu
int currentMenuItem = 0; // Current selected menu item
int lastMenuItem = -1; // Initialize with -1 to ensure the menu is drawn the first time



unsigned long lastJoystickMove = 0;
const unsigned int joystickDebounceTime = 300;

unsigned long lastUpdateTime = 0; // For LCD update interval
const unsigned long updateInterval = 500; // Update every 500 milliseconds

unsigned long displayScore(unsigned long timeElapsed) {
    return timeElapsed / 1000; // Converts milliseconds to seconds
}


const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte swState = HIGH;
unsigned int lastDebounceTime = 0;
unsigned int debounceDelay = 200;
int playerVisible = true;
// bool bombVisible = false;
const unsigned long playerBlinkInterval = 700;
unsigned long lastPlayerBlinkTime = 0;
bool startGame = true;
int wallCount = 0;  
bool animationPlayed = false;
bool isShooting = false;
bool firstGameRun = true; // Global variable to track the first run of the game
unsigned long shootingStartTime = 0;
const unsigned long shootingDuration = 1000; 
const unsigned long shootingBlinkInterval = 100;
unsigned long highscore = 0;

int currentLevel = 1; // Current game level
const int maxLevel = 3; // Maximum number of levels

unsigned long gameStartTime = 0;

unsigned long lastMenuEnterTime = 0;
const unsigned long menuEnterDelay = 500; // 500 milliseconds


LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

const byte matrixSize = 8;

byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;


const int minThreshold = 200;
const int maxThreshold = 600;
const byte moveInterval = 100;
unsigned long long lastMoved = 0;
bool matrixChanged = true;

byte matrix[matrixSize][matrixSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};


const byte heartPattern[8] = {
  B00000000,
  B01100110,
  B11111111,
  B11111111,
  B01111110,
  B00111100,
  B00011000,
  B00000000
};


enum GameState {
  INTRO,
  MENU,
  PLAYING,
  LEVEL_COMPLETE,
  WAIT_AFTER_LEVEL,
  ADJUST_LCD_BRIGHTNESS,
  ADJUST_MATRIX_BRIGHTNESS,
  RETURN_TO_SETTINGS,
  DISPLAY_SCORE,
  SETTINGS,
  ABOUT,
};

enum MenuSelection {
  START_GAME,
  SETTINGS_MENU,
  ABOUT_MENU
};

enum EndMenuSelection {
  NEXT_LEVEL,
  MAIN_MENU
};

enum SettingsMenuOption {
    LCD_BRIGHTNESS,
    MATRIX_BRIGHTNESS,
    SETTINGS_EXIT
};

int currentSettingsMenu = 0; // Current selected settings option
const int totalSettingsOptions = 3; // Total number of options in the settings menu
SettingsMenuOption currentSettingsOption  = LCD_BRIGHTNESS;
EndMenuSelection endMenuSelection = NEXT_LEVEL; // Current selection in the end-of-level menu
GameState currentState = INTRO;
MenuSelection currentMenuSelection = ABOUT_MENU;
int menuPosition = 0; // Position in the menu
unsigned long timeTakenForLevel[3] = {0, 0, 0}; // Array to store time taken for each level


unsigned long lastMatrixBrightnessAdjustTime = 0;
const unsigned long matrixBrightnessAdjustInterval = 100; // 100 milliseconds
const int maxMatrixBrightness = 15; // Maximum brightness level for LED matrix
int matrixBrightness = 8; // Default brightness (0-15)

unsigned long lastBrightnessAdjustTime = 0;
const unsigned long brightnessAdjustInterval = 100; // Adjust the brightness every 100 milliseconds
const int lcdBrightnessPin = 3; // PWM pin connected to LCD A pin
int playerIntensity = 0;
int wallIntensity  = 0;
int bombIntensity = 0;
int lcdBrightness = 128; // Default brightness (0-255)
const int EEPROM_LCD_BRIGHTNESS_ADDR = 0; // Address to store LCD brightness in EEPROM
const int EEPROM_MATRIX_BRIGHTNESS_ADDR = sizeof(int); // EEPROM address for matrix brightness




void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  Serial.println("aaa");
  lc.shutdown(0, false);
  lc.setIntensity(0, 4);  
  lc.clearDisplay(0);
  pinMode(swPin, INPUT_PULLUP);
  pinMode(lcdBrightnessPin, OUTPUT);
  EEPROM.get(EEPROM_LCD_BRIGHTNESS_ADDR, lcdBrightness);
  analogWrite(lcdBrightnessPin, lcdBrightness); // Set initial brightness
  EEPROM.get(EEPROM_MATRIX_BRIGHTNESS_ADDR, matrixBrightness);
  lc.setIntensity(0, matrixBrightness); // Set initial matrix brightness
  introDisplayed = false; // Ensure this is false at startup
}

void setGameIntensity(){
  if(matrixBrightness >= 14){
    playerIntensity = 13;
  }
  else if(matrixBrightness ==1){
    playerIntensity = 2;
  }
  else{
      playerIntensity = matrixBrightness;
  }
  wallIntensity = playerIntensity - 1;
  bombIntensity = playerIntensity +2;

  Serial.println("Intesitate jucator:");
  Serial.println(playerIntensity);
  Serial.println("Intensitate perete:");
  Serial.println(wallIntensity);
  Serial.println("Intensitate bomba:");
  Serial.println(bombIntensity);
}

void selectMenuItem() {
    switch (currentMenuItem) {
        case START_GAME:
            currentState = PLAYING;
            break;
        case SETTINGS_MENU:
            currentState = SETTINGS;
            break;
        case ABOUT_MENU:
            currentState = ABOUT;
            break;
    }
}

void loop() {
    switch (currentState) {
        case INTRO:
            // Intro logic
            if (!introDisplayed) {
                lcd.clear();
                lcd.print("Welcome to");
                lcd.setCursor(0, 1);
                lcd.print("Turis Game!");
                introDisplayed = true;
                introStartTime = millis();
            } else if (millis() - introStartTime > introDuration) {
                lcd.clear();
                introDisplayed = false;
                currentState = MENU;
            }
            break;
        case MENU:
            displayMenu();
            handleJoystickMenu();
            break;
        case PLAYING:
            game(); 
            break;
        case WAIT_AFTER_LEVEL:
            if (millis() - gameStartTime > 1000) { // 1 second has passed
                currentState = LEVEL_COMPLETE;
                displayEndLevelMenu();
            }
            break;
        case LEVEL_COMPLETE:
            Serial.println("imi intra unde trebe");
            if (currentLevel == maxLevel) {
              currentState = DISPLAY_SCORE;
            } else {
              handleJoystickEndLevelMenu();
            }
            break;
        case SETTINGS:
            displaySettingsMenu();
            handleJoystickSettings();
            break;
        case ADJUST_LCD_BRIGHTNESS:
            adjustLCDBrightness();
            break;
         case ADJUST_MATRIX_BRIGHTNESS:
            adjustMatrixBrightness();
            break;
        case ABOUT:
            displayAboutScreen();
            break;
        case DISPLAY_SCORE:
            if (millis() - lastUpdateTime > updateInterval) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Total Score:");
            lcd.setCursor(0, 1);
            lcd.print(calculateTotalScore()); // You need to implement this function
            buttonPress();
            lastUpdateTime = millis();
            }
            break;
    }
}

void displayMenu() {
    // Check if the menu item has changed
    if (lastMenuItem != currentMenuItem) {
        lcd.clear();

        // Display "Main Menu" on the first line
        lcd.setCursor(0, 0);
        lcd.print("Main Menu");

        // Redraw the menu item on the second line
        String menuItems[] = {"Play Game", "Settings", "About"};
        lcd.setCursor(1, 1); // Set cursor to the second line
        if (currentMenuItem < totalMenuItems) {
            lcd.print(">"); // Highlight the current selection
            lcd.print(menuItems[currentMenuItem]);
        }

        // Update the last menu item
        lastMenuItem = currentMenuItem;
    }
}

void checkEnd() {
    wallCount = 0;
    for (int col = 0; col < matrixSize; col++) {
        for (int row = 0; row < matrixSize; row++) {
            if (matrix[row][col] == wallIntensity) {
                wallCount++;
            }
        }
    }

    if (wallCount == 0 && currentState == PLAYING) {
        lcd.clear();
        currentState = WAIT_AFTER_LEVEL;
        timeTakenForLevel[currentLevel - 1] = millis() - gameStartTime; // Store time taken for the level
        gameStartTime = millis(); // Reset gameStartTime to use as timer
    }
}

void displaySettingsMenu() {
    static int lastSettingsMenu = -1; // Keep track of the last settings menu item

    if (lastSettingsMenu != currentSettingsMenu) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Settings");

        const char* settingsOptions[] = {"LCD Brightness", "Matrix Brightness", "Exit"};
        lcd.setCursor(0, 1); // Set cursor to the second line
        lcd.print(">"); // Highlight the current selection
        lcd.print(settingsOptions[currentSettingsMenu]);

        lastSettingsMenu = currentSettingsMenu;
    }
}

void displayEndLevelMenu() {
    clearMatrix();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("End Level Menu");

        // Update only the selection part
        String endMenuItems[] = {"Next Level", "Main Menu"};
        lcd.setCursor(0, 1);
        lcd.print("> ");
        lcd.print(endMenuItems[endMenuSelection]);

}

void displayAboutScreen() {
    const char* name = "Turis Vlad";
    const char* githubLink = "https://github.com/VladRo26/IntroductionToRobotics        ";
    static int githubLinkPosition = 0;
    static unsigned long lastScrollTime = 0;
    const int scrollInterval = 300;  // Time in milliseconds between scrolls

    lcd.setCursor(0, 0);
    lcd.print(name);

    if (millis() - lastScrollTime > scrollInterval) {
        lcd.clear();

        // Display the name on the first row

        // Display the scrolling GitHub link on the second row
        lcd.setCursor(0, 1);
        lcd.print(githubLink + githubLinkPosition);

        githubLinkPosition++;
        if (githubLinkPosition >= (int)strlen(githubLink) - 16) {
            githubLinkPosition = 0;  // Reset scrolling for GitHub link
        }

        lastScrollTime = millis();
    }
   buttonPress();
}

void resetGame() {
  clearMatrix();
  wallCount = 0;
  isShooting = false;
  animationPlayed = false;
  startGame = true;
  firstGameRun = true; // Reset for next game start
}

unsigned long calculateTotalScore() {
    const unsigned long baseScore = 10000; // Example base score
    unsigned long timePenalty = 0;

    for (int i = 0; i < maxLevel; i++) {
        // Adding time penalty for each level
        // Assuming each second reduces the score by a certain amount (e.g., 100 points per second)
        timePenalty += (timeTakenForLevel[i] / 1000) * 100;
    }

    // Ensure that the score does not go negative
    unsigned long totalScore = (baseScore > timePenalty) ? (baseScore - timePenalty) : 0;
    return totalScore;
}


void printLevelTimes() {
    Serial.println("Time taken for each level (in seconds):");
    for (int i = 0; i < maxLevel; i++) {
        Serial.print("Level ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(timeTakenForLevel[i] / 1000);
    }
}


void clearMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      matrix[row][col] = 0;
    }
  }
  updateMatrix();
}

void handleJoystickMenu() {
    int xValue = analogRead(xPin); // Read the X-axis of the joystick
    unsigned long currentMillis = millis();

    // Check if debounce period has elapsed
    if (currentMillis - lastJoystickMove > joystickDebounceTime) {
        if (xValue < minThreshold && currentMenuItem > 0) {
            currentMenuItem--;
            displayMenu();
            lastJoystickMove = currentMillis;
        } else if (xValue > maxThreshold && currentMenuItem < totalMenuItems - 1) {
            currentMenuItem++;
            displayMenu();
            lastJoystickMove = currentMillis;
        }
        buttonPress();
    }
}

void handleJoystickEndLevelMenu() {
    int xValue = analogRead(xPin); 
    unsigned long currentMillis = millis();

    if (currentMillis - lastJoystickMove > joystickDebounceTime) {
        if (xValue < minThreshold && endMenuSelection != NEXT_LEVEL) {
            Serial.println("imi intra aici in next level");
            endMenuSelection = NEXT_LEVEL;
            lastJoystickMove = currentMillis;
            displayEndLevelMenu(); 
        } else if (xValue > maxThreshold && endMenuSelection != MAIN_MENU) {
            Serial.println("imi intra aici in main");
            endMenuSelection = MAIN_MENU;
            lastJoystickMove = currentMillis;
            displayEndLevelMenu(); 
        }
        buttonPress();
    }

}


void handleJoystickSettings() {
    int xValue = analogRead(xPin); // Read the X-axis of the joystick
    unsigned long currentMillis = millis();

    // Check if debounce period has elapsed
    if (currentMillis - lastJoystickMove > joystickDebounceTime) {
        if (xValue < minThreshold && currentSettingsMenu > 0) {
            currentSettingsMenu--;
            displaySettingsMenu();
            lastJoystickMove = currentMillis;
        } else if (xValue > maxThreshold && currentSettingsMenu < totalSettingsOptions - 1) {
            currentSettingsMenu++;
            displaySettingsMenu();
            lastJoystickMove = currentMillis;
        }
    }
     buttonPress();
}

void game(){

   setGameIntensity();
  if (firstGameRun) {
    generateWalls();
    spawnPlayer();
    firstGameRun = false;
    gameStartTime = millis(); 
  }

  if (gameStartTime == 0) { 
    gameStartTime = millis();
  }

    if (millis() - lastMoved > moveInterval) {
    updatePositions();
    lastMoved = millis();
  }

    
  if (millis() - lastPlayerBlinkTime > playerBlinkInterval) {
  playerVisible = !playerVisible;
  lastPlayerBlinkTime = millis();
  matrix[xPos][yPos] = playerVisible ? playerIntensity : 0; 
  matrixChanged = true;
  }
    
  buttonPress();


  if (isShooting) {
    unsigned long currentTime = millis();
    if (currentTime - shootingStartTime < shootingDuration) {
      if ((currentTime - shootingStartTime) / shootingBlinkInterval % 2 == 0) {
        setBombs(bombIntensity);
      } else {
        setBombs(0); 
      }
    } else {
      setBombs(0); 
      isShooting = false;
      clearBombs(); 
    }
  }

  if (matrixChanged) {
    updateMatrix();
    matrixChanged = false;
  }

 if (millis() - lastUpdateTime > updateInterval) {
        unsigned long currentTime = millis();
        unsigned long timeElapsed = currentTime - gameStartTime;
        unsigned long score = displayScore(timeElapsed);

        // Display time and score on the LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Time: ");
        lcd.print(timeElapsed / 1000); // Display time in seconds
        lcd.setCursor(0, 1);
        lcd.print("Level: ");
        lcd.print(currentLevel);

        lastUpdateTime = millis(); // Update the last update time
    }

  checkEnd();

}


void clearBombs() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      if (matrix[row][col] == bombIntensity) {
        matrix[row][col] = 0; 
      }
    }
  }
  matrixChanged = true;
}

void updateMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row][col] > 0);
      lc.setIntensity(0, matrix[row][col]);  
    }
  }
}

void generateWalls(){
  randomSeed(analogRead(0));

  for(int row = 0; row < matrixSize; row++){
    for(int col = 0; col < matrixSize; col++){
      if(random(100) < 10){
        matrix[row][col] = wallIntensity;
      }
    }
  }

}

void buttonPress() {
    byte newSwState = digitalRead(swPin);
     
    if((newSwState != swState) && (millis() - lastDebounceTime) > debounceDelay){
      lastDebounceTime = millis();
      swState = newSwState;
      if(swState == LOW ){
         switch (currentState) {
                case PLAYING:
                    Serial.println("Button Pressed\n"); // Debug print
                    shoot();
                    break;
                case MENU:
                    Serial.println("Button Pressed in menu\n");
                    if (currentMenuItem == START_GAME) {
                      Serial.println("imi incepe sa joc");
                        currentState = PLAYING; // Change state to start the game
                    } else if (currentMenuItem == SETTINGS_MENU) {
                        currentState = SETTINGS;
                    } else if (currentMenuItem == ABOUT_MENU) {
                        currentState = ABOUT;
                    }
                    break;
                 case SETTINGS:
                    Serial.println("In SETTINGS state\n"); // Debug print
                    // Logic to handle settings selection
                    if(currentSettingsMenu == LCD_BRIGHTNESS){
                            currentState = ADJUST_LCD_BRIGHTNESS;
                    } else if(currentSettingsMenu == MATRIX_BRIGHTNESS){
                            currentState = ADJUST_MATRIX_BRIGHTNESS;
                    } else if (currentSettingsMenu == SETTINGS_EXIT){
                            Serial.println("Exiting SETTINGS\n"); // Debug print
                            currentState = MENU; // Navigate back to the main menu
                            currentMenuItem = 0; // Optionally, reset the menu item to the first one
                            currentSettingsMenu = 0;
                            lastMenuItem = -1; // Force a redraw of the main menu
                            lastMenuEnterTime = millis(); // Update the time when entering the menu
                    }
                    break;
                  case ADJUST_LCD_BRIGHTNESS:
                    // Logic for ADJUST_LCD_BRIGHTNESS state
                    EEPROM.put(EEPROM_LCD_BRIGHTNESS_ADDR, lcdBrightness); // Save the new brightness value
                    currentState = SETTINGS; // Return to settings menu
                    currentSettingsMenu = 1;
                    break;
                  case ADJUST_MATRIX_BRIGHTNESS:
                    EEPROM.put(EEPROM_MATRIX_BRIGHTNESS_ADDR, matrixBrightness); // Save the new brightness value
                    clearMatrix();
                    currentState = SETTINGS; // Return to settings menu
                    currentSettingsMenu= 0;
                    break;
                  case ABOUT:
                     Serial.println("Button Pressed in ABOUT state");
                     currentState = MENU; // Navigate back to the main menu
                     currentMenuItem = 0; // Optionally, reset the menu item to the first one
                     lastMenuItem = -1; // Force a redraw of the main menu
                     lastMenuEnterTime = millis(); // Update the time when entering the menu
                     break;
                  case LEVEL_COMPLETE:
                    if (endMenuSelection == NEXT_LEVEL) {
                        if (currentLevel < maxLevel) {
                            currentLevel++;
                            resetGame();
                            currentState = PLAYING;
                        } else {
                            currentState = MENU;
                        }
                    } else if (endMenuSelection == MAIN_MENU) {
                        resetGame();
                        currentState = MENU; // Transition to MENU state
                        lastMenuEnterTime = millis(); // Update the time when entering the menu
                        currentMenuItem = 0; // Reset the current menu item
                        lastMenuItem = -1; // Force a redraw of the menu
                    }
                    break;
                  case DISPLAY_SCORE:
                     resetGame();
                     currentState = MENU;
                     currentLevel = 1;
                     lastMenuEnterTime = millis(); // Update the time when entering the menu
                     currentMenuItem = 0; // Reset the current menu item
                     lastMenuItem = -1; // Force a redraw of the menu
                      printLevelTimes(); // Print the times for each level
                      for (int i = 0; i < 3; i++) {
                      timeTakenForLevel[i] = 0;
                    }
                  break;
            }
      }
    }

}


void shoot() {
  if (!isShooting) { 
    isShooting = true;
    shootingStartTime = millis();

    setBombs(bombIntensity);
  }
}

void adjustLCDBrightness() {
    unsigned long currentMillis = millis();
    
    // Check if the appropriate time has passed since the last adjustment
    if (currentMillis - lastBrightnessAdjustTime >= brightnessAdjustInterval) {
        int yValue = analogRead(yPin); // Reading Y-axis value of the joystick

        // Adjust brightness level based on joystick position
        if (yValue < minThreshold && lcdBrightness > 0) {
            lcdBrightness = max(0, lcdBrightness - 25); // Increase the decrement step to 25
        } else if (yValue > maxThreshold && lcdBrightness < 255) {
            lcdBrightness = min(255, lcdBrightness + 25); // Increase the increment step to 25
        }

        // Apply the brightness setting
        analogWrite(lcdBrightnessPin, lcdBrightness);

        // Calculate the brightness level for display (1 to 12)
        int brightnessLevel = map(lcdBrightness, 0, 255, 1, 12);

        // Display the current brightness level on the LCD
        lcd.clear();
        lcd.print("Brightness: ");
        lcd.setCursor(0, 1);
        lcd.print(brightnessLevel);

        // Update the last adjustment time
        lastBrightnessAdjustTime = currentMillis;
    }

    buttonPress(); // Call buttonPress to handle any button action
}


void adjustMatrixBrightness() {
    unsigned long currentMillis = millis();
  
    if (currentMillis - lastMatrixBrightnessAdjustTime >= matrixBrightnessAdjustInterval) {
        int yValue = analogRead(yPin); // Reading Y-axis value of the joystick

        // Adjust brightness level based on joystick position
        if (yValue < minThreshold && matrixBrightness > 1) {
            matrixBrightness--;
        } else if (yValue > maxThreshold && matrixBrightness < maxMatrixBrightness) {
            matrixBrightness++;
        }

        // Apply the brightness setting (adjusting for 0-based index)
        lc.setIntensity(0, matrixBrightness);

        // Display heart pattern on the matrix with the new brightness
        for (int row = 0; row < 8; row++) {
            lc.setRow(0, row, heartPattern[row]);
        }

        // Display the current matrix brightness on the LCD
        lcd.clear();
        lcd.print("Matrix Brightness: ");
        lcd.setCursor(0, 1);
        lcd.print(matrixBrightness);

        // Update the last adjustment time
        lastMatrixBrightnessAdjustTime = currentMillis;
    }

    // Call to buttonPress() to handle any button actions
    buttonPress();
}


void setBombs(byte state) {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      if (matrix[row][col] == playerIntensity) { // If player is present
        // Set bombs based on the level
        int bombRange = 5 - currentLevel; // 3 bombs in level 1, 2 in level 2, and 1 in level 3
        for (int i = 1; i < bombRange; i++) {
          if (row >= i) matrix[row - i][col] = state;
          if (col >= i) matrix[row][col - i] = state;
          if (row < matrixSize - i) matrix[row + i][col] = state;
          if (col < matrixSize - i) matrix[row][col + i] = state;
        }
      }
    }
  }
  matrixChanged = true;
}


void spawnPlayer() {
  bool playerSpawned = false;
  randomSeed(analogRead(0)); 

  while (!playerSpawned) {
    byte randomX = random(matrixSize);
    byte randomY = random(matrixSize);

    if (matrix[randomX][randomY] == 0) {
      xPos = randomX;
      yPos = randomY;
      matrix[xPos][yPos] = playerIntensity; 
      playerSpawned = true;
    }
  }
}

void updatePositions() {
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

   if (isShooting) {
    return; 
  }

  xLastPos = xPos;
  yLastPos = yPos;

  if (xValue < minThreshold) {
        if (xPos > 0) {
      --xPos;
    } else {
      xPos = matrixSize - 1;
    }
  }

  if (xValue > maxThreshold) {
      if (xPos < matrixSize - 1) {
      ++xPos;
    } else {
      xPos = 0;
    }
  }

  if (yValue < minThreshold) {
    if (yPos < matrixSize - 1) {
      ++yPos;
    } else {
      yPos = 0;
    }
  }

  if (yValue > maxThreshold) {
    if (yPos > 0) {
      --yPos;
    } else {
      yPos = matrixSize - 1;
    }
  }
  if(matrix[xPos][yPos] == wallIntensity){

    xPos = xLastPos;
    yPos = yLastPos;

  }else{
      if (xPos != xLastPos || yPos != yLastPos) {
    matrixChanged = true;
    matrix[xLastPos][yLastPos] = 0;
    matrix[xPos][yPos] = playerIntensity;  // Set the brightness for the LED at the new position
  }
  }
}
