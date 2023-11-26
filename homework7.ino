#include "LedControl.h"
//ANIMATON LOGIC IS IMPLEMENTED WITH CHATGPT!
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
const int xPin = A0;
const int yPin = A1;
const int swPin = 2;
byte swState = LOW;
byte reading = HIGH;
byte lastReading = HIGH;
unsigned int lastDebounceTime = 0;
unsigned int debounceDelay = 100;
int playerVisible = true;
// bool bombVisible = false;
const unsigned long playerBlinkInterval = 700;
unsigned long lastPlayerBlinkTime = 0;
bool startGame = true;
int wallCount = 0;  
bool animationPlayed = false;
bool isShooting = false;
unsigned long shootingStartTime = 0;
const unsigned long shootingDuration = 1000; 
const unsigned long shootingBlinkInterval = 100;
unsigned long highscore = 0;


unsigned long gameStartTime = 0;

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

void setup() {
  Serial.begin(9600);
  lc.shutdown(0, false);
  lc.setIntensity(0, 4);  
  lc.clearDisplay(0);
  pinMode(swPin,INPUT_PULLUP);
  generateWalls();
  spawnPlayer();
}

void loop() {
  if (startGame) {
    game();
  } else {

  }
}


void checkAnimation() {
  if(animationPlayed) {
    return; 
  }

  wallCount = 0;
  int playerCount = 0;

  for (int col = 0; col < matrixSize; col++) {
    for (int row = 0; row < matrixSize; row++) {
      if (matrix[row][col] == 1) {
        wallCount++;
      }
      if (matrix[row][col] == 2) {
        playerCount++;
      }
    }
  }

  // Trigger animation only if conditions are met and it has not been played before
  if (wallCount == 0 && playerCount == 1 && !animationPlayed) {
    clearMatrix();
    updateMatrix();
    printScore();
    displaySpiralAnimation();
  }
}

void resetGame() {
  // Clear the matrix and reset all variables to initial state
  clearMatrix();
  generateWalls();
  spawnPlayer();
  wallCount = 0;
  isShooting = false;
  animationPlayed = false; 
  startGame = true; 
}



void clearMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      matrix[row][col] = 0;
    }
  }
}



void displaySpiralAnimation() {
  int startRow = 0, startCol = 0;
  int endRow = matrixSize - 1, endCol = matrixSize - 1;

  while (startRow <= endRow && startCol <= endCol) {
    
    for (int i = startCol; i <= endCol; i++) {
      setLedAndDelay(startRow, i, true);
    }
    startRow++;

    // Right column
    for (int i = startRow; i <= endRow; i++) {
      setLedAndDelay(i, endCol, true);
    }
    endCol--;

    // Bottom row
    if (startRow <= endRow) {
      for (int i = endCol; i >= startCol; i--) {
        setLedAndDelay(endRow, i, true);
      }
      endRow--;
    }

    // Left column
    if (startCol <= endCol) {
      for (int i = endRow; i >= startRow; i--) {
        setLedAndDelay(i, startCol, true);
      }
      startCol++;
    }
  }
  resetGame();
}

void setLedAndDelay(int row, int col, bool state) {
  lc.setLed(0, row, col, state);
  delay(100); 
}


void game(){

  if(!startGame){
    return;
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
  matrix[xPos][yPos] = playerVisible ? 2 : 0; 
  matrixChanged = true;
  }
    
  buttonPress();


  if (isShooting) {
    unsigned long currentTime = millis();
    if (currentTime - shootingStartTime < shootingDuration) {
      if ((currentTime - shootingStartTime) / shootingBlinkInterval % 2 == 0) {
        setBombs(5);
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

  checkAnimation();

}


void clearBombs() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      if (matrix[row][col] == 5) {
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
      if(random(100) < 65){
        matrix[row][col] = 1;
      }
    }
  }

}

void buttonPress(){
  reading = digitalRead(swPin);
  if(reading != lastReading){
    lastDebounceTime = millis();
  }
  if((millis() - lastDebounceTime) > debounceDelay){

    if(reading != swState){
      swState = reading;
    }

    if(swState == LOW){
      shoot();
    }
  }
  lastReading = reading;
}

void shoot() {
  if (!isShooting) { 
    isShooting = true;
    shootingStartTime = millis();

    setBombs(5);
  }
}

void setBombs(byte state) {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      if (matrix[row][col] == 2) {
        if (row > 1) matrix[row-2][col] = state; 
        if (row > 0) matrix[row-1][col] = state; 
        if (col > 1) matrix[row][col-2] = state; 
        if (col > 0) matrix[row][col-1] = state; 
        if (row < matrixSize-2) matrix[row+2][col] = state; 
        if (row < matrixSize-1) matrix[row+1][col] = state; 
        if (col < matrixSize-2) matrix[row][col+2] = state;
        if (col < matrixSize-1) matrix[row][col+1] = state; 
      }
    }
  }
  matrixChanged = true;
}

void printScore() {
  unsigned long timeTaken = millis() - gameStartTime;

  // Larger this value, higher the maximum potential score
  unsigned long scoreBase = 1000000; 

  // Calculate score
  unsigned long score = scoreBase / timeTaken;
  
  Serial.print("Your score is: ");
  Serial.println(score);

  gameStartTime = 0;
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
      matrix[xPos][yPos] = 2; 
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
  if(matrix[xPos][yPos] == 1){

    xPos = xLastPos;
    yPos = yLastPos;

  }else{
      if (xPos != xLastPos || yPos != yLastPos) {
    matrixChanged = true;
    matrix[xLastPos][yLastPos] = 0;
    matrix[xPos][yPos] = 2;  // Set the brightness for the LED at the new position
  }

  }
}
