#define UP  0
#define DOWN  1
#define LEFT  2
#define RIGHT  3
const int pinSW = 2;
const int pinX = A0;
const int pinY = A1;
const int pinA = 12;
const int pinB = 10;
const int pinC = 9;
const int pinD = 8;
const int pinE = 7;
const int pinF = 6;
const int pinG = 5;
const int pinDP = 4;
const int segSize = 8;
unsigned long lastPress = 0;
const int joySensitivityAmount = 50;
const unsigned long blinkInterval = 500;
unsigned int previousPress = 0;
unsigned long previousMillis = 0;
byte blinkState = HIGH;
byte dpState = HIGH;
byte swState = LOW;
byte lastSwState = LOW;
const int totalMoves = 4;
int xValue = 0;
int yValue = 0;
bool joyMoved = false;
int minThreshold = 400;
int maxThreshold = 600;
volatile int buttonFlag = 0;
volatile bool buttonPressed = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;
unsigned long lastDebounceTime = 0;
int currentPosition = 0;
int segments[segSize] = {
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

byte segmentState[segSize] = {
 LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW
};

int neighbours[segSize][totalMoves] = {
//UP DOWN LEFT RIGHT
  {0, 6 , 5 , 1},  // a
  {0, 6 , 5 , 1},  // b
  {6, 3 , 4 , 7}, // c
  {6, 3 , 4 , 2},  // d
  {6, 3 , 4 , 2},  // e
  {0, 6 , 5 , 1},  // f
  {0, 3 , 6 , 6},  // g
  {7, 7 , 2  , 7}   // dp
};

void setup() {
  for (int i = 0; i < segSize; i++) {
    pinMode(segments[i], OUTPUT);
  }
  pinMode(pinSW, INPUT_PULLUP);
  Serial.begin(9600);

  currentPosition = segSize - 1;

  // attachInterrupt(digitalPinToInterrupt(pinSW),button_ISR,CHANGE);

}

void move() {
  if (xValue >= minThreshold && xValue <= maxThreshold && yValue >=minThreshold && yValue <= maxThreshold) {
    joyMoved = false;
  }

  if (xValue > maxThreshold && xValue - maxThreshold > joySensitivityAmount && joyMoved == false) {
      // 0 - means moving upwards
      currentPosition = neighbours[currentPosition][UP];
      joyMoved = true;
  }
  else if (xValue < minThreshold && minThreshold - xValue > joySensitivityAmount && joyMoved == false) {
     // 1 - means moving down
      currentPosition = neighbours[currentPosition][DOWN];
      joyMoved = true;
  }

  else if (yValue < minThreshold && minThreshold - yValue > joySensitivityAmount && joyMoved == false) {
    // 2 - means moving left
    currentPosition = neighbours[currentPosition][LEFT];
    joyMoved = true;
  }

  else if (yValue > maxThreshold && yValue - maxThreshold > joySensitivityAmount && joyMoved == false) {
   // 3 - means moving right
   currentPosition = neighbours[currentPosition][RIGHT];
   joyMoved = true;
  }
}

void toggle(int pin) {
  segmentState[pin] = !segmentState[pin];
}

void reset(){
  for (int i = 0; i < segSize; i++) {
      segmentState[i] = LOW; 
      }
    currentPosition = segSize - 1;
}

void checkPress(){
    swState = digitalRead(pinSW);
    if (swState != lastSwState) {
      if (swState == LOW) {
        toggle(currentPosition);
        lastPress = millis();
      }
      lastSwState = swState;
    }
    if((millis() - lastPress > 2000) && (swState == LOW)){

      reset();
    }
}

// void button_ISR(){
//   buttonFlag = 1;
// }


void loop() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

    move();

      for (int i = 0; i < segSize; i++) {
        digitalWrite(segments[i],segmentState[i]) ;
      }
        digitalWrite(segments[currentPosition],blinkState);

    unsigned long currentMillis = millis();


  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;
    blinkState = !blinkState;

  }
  checkPress();

}
