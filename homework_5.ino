const int latchPin = 11; // Connects to STCP (latch pin) on the shift register
const int clockPin = 10; // Connects to SHCP (clock pin) on the shift register
const int dataPin = 12; // Connects to DS (data pin) on the shift register
const int maxLaps = 4;
int lapTimes[maxLaps];
int lapIndex = 0;

// Define connections to the digit control pins for a 4-digit display
const int segD1 = 4;
const int segD2 = 5;
const int segD3 = 6;
const int segD4 = 7;

unsigned long startlastDebounceTime = 0;
unsigned long debounceDelay = 200;

int NrLap = 0;

const int startButtonPin = 2;
const int restartButtonPin = 8;
const int lapFlagButtonPin = 3;

byte lastrestartButtonState = HIGH;

int restartPressed = 1;

volatile bool startButtonPressed = false;

volatile bool lapButtonPressed = false;


volatile unsigned long startlastInterruptTime = 0;

volatile unsigned long laplastInterruptTime = 0;


int start = 0;
int paused = 0;
int lapping = 0;
byte lastStartButtonState = HIGH;

byte startButtonState = HIGH;
byte restartButtonState = HIGH;
byte lapFlagButtonState = HIGH;

byte lastLapFlagButtonState = HIGH;

int seconds = 0;
int tenths = 0;
int minutes = 0;

// Store the digits in an array for easy access
int displayDigits[] = {segD1, segD2, segD3, segD4};
const int displayCount = 4; // Number of digits in the display

const int encodingsNumber = 11;
// Define byte encodings for the hexadecimal characters 0-F
byte byteEncodings[encodingsNumber] = {
  //A B C D E F G DP 
  B11111100, // 0 
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
  B00000001, // DP
};

unsigned long lastIncrement = 0;
unsigned long delayCount = 100; // Delay between updates (milliseconds)

void setup() {
  // Initialize the pins connected to the shift register as outputs
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(restartButtonPin, INPUT_PULLUP);
  pinMode(lapFlagButtonPin, INPUT_PULLUP);

  // Initialize digit control pins and set them to LOW (off)
  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }

  attachInterrupt(digitalPinToInterrupt(startButtonPin),starthandleInterrupt,FALLING);
  attachInterrupt(digitalPinToInterrupt(lapFlagButtonPin),laphandleInterrupt,FALLING);


  // Begin serial communication for debugging purposes
  Serial.begin(9600);
}

void activateDisplay(int displayNumber) {
  // Turn off all digit control pins to avoid ghosting
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }
  // Turn on the current digit control pin
  digitalWrite(displayDigits[displayNumber], LOW);
}

void writeReg(int digit) {
  // Prepare to shift data by setting the latch pin low
  digitalWrite(latchPin, LOW);
  // Shift out the byte representing the current digit to the shift register
  shiftOut(dataPin, clockPin, MSBFIRST, digit);
  // Latch the data onto the output pins by setting the latch pin high
  digitalWrite(latchPin, HIGH);
}

void writeDigit(int display,int digit)
{
   activateDisplay(display);
   writeReg(byteEncodings[digit]);
   delay(0);
   writeReg(B00000000);
}

void writeTime(int minutes,int seconds, int tenths) {

  int currentSeconds = seconds;

  int firstSec = 0 ;
  int lastSec = 0;

  lastSec = currentSeconds % 10;
  currentSeconds /= 10;
  firstSec = currentSeconds;

  writeDigit(3,tenths);
  writeDigit(2,lastSec);
  writeDigit(1,firstSec);
  writeDigit(0,minutes);

    activateDisplay(2);
    writeReg(byteEncodings[10]);
    delay(0);
    writeReg(B00000000);

    activateDisplay(0);
    writeReg(byteEncodings[10]);
    delay(0);
    writeReg(B00000000);

}

void displayInitialState(){

  writeDigit(3,0);
  writeDigit(2,0);
  writeDigit(1,0);
  writeDigit(0,0);

  activateDisplay(2);
  writeReg(byteEncodings[10]);
  delay(0);
  writeReg(B00000000);

  activateDisplay(0);
  writeReg(byteEncodings[10]);
  delay(0);
  writeReg(B00000000);

}

void loop() {

  // startButtonState = digitalRead(startButtonPin);
  restartButtonState = digitalRead(restartButtonPin);
  // lapFlagButtonState = digitalRead(lapFlagButtonPin);

  if(startButtonPressed){
    if (start == 0) {
      start = 1;
      paused = 0; 
      restartPressed = 0;
      Serial.println("start");
    } else {
      start = 0;
      paused = 1;
      Serial.println("paused");
    }
    startButtonPressed = false;
  }

  if(lapButtonPressed){
     Serial.println("lap");
    if (restartPressed == 1 && NrLap >  0){
      if(lapping){
          Serial.println(lapIndex);
          lapIndex++;
          if(lapIndex == NrLap){
            lapIndex =0;
         }
      }else{
        lapping = 1;
      }
    }else if(start == 1){
      if(lapIndex == maxLaps){
        for(int i =0 ; i< maxLaps - 1 ; i++){
          lapTimes[i] = lapTimes[i+1];
        }
        lapIndex = maxLaps - 1;
      }
      lapTimes[lapIndex] = minutes * 600 + seconds * 10 + tenths;
      Serial.println(lapTimes[lapIndex]);
      lapIndex++;
    }
    lapButtonPressed = false;
  }


  if ((restartButtonState == LOW && lastrestartButtonState == HIGH) && start == 0) {
    Serial.println("restart");
    tenths = 0;
    seconds = 0;
    minutes = 0;
    start = 0;
    restartPressed = 1;
    NrLap = lapIndex;
    lapIndex = 0;
    lapping = 0;
  }

  lastrestartButtonState = restartButtonState;

  if(lapping == 1){

    int currentLapTime = lapTimes[lapIndex];
    int lapMinutes = currentLapTime / 600;
    int lapSeconds = (currentLapTime % 600) / 10;
    int lapTenths = currentLapTime % 10;
    
    writeTime(lapMinutes, lapSeconds, lapTenths);
  }else  if (start == 1) {
    if (millis() - lastIncrement > delayCount) {
      tenths++;

      if (tenths == 10) {
        tenths = 0;
        seconds++;
      }

      if (seconds == 60) {
        seconds = 0;
        minutes++;
      }

      if (minutes == 10) {
        tenths = 0;
        seconds = 0;
        minutes = 0;
      }

      lastIncrement = millis();
    }
  }
  if(!lapping){
      writeTime(minutes, seconds, tenths);
  }
}

void starthandleInterrupt(){
  static unsigned long startinterruptTime = 0;
  startinterruptTime = micros();
  
  if(startinterruptTime -  startlastInterruptTime > debounceDelay * 1000ul){
    startButtonPressed = true;
  }
  startlastInterruptTime = startinterruptTime;
}

void laphandleInterrupt(){
  static unsigned long lapinterruptTime = 0;
  lapinterruptTime = micros();
  
  if(lapinterruptTime -  laplastInterruptTime > debounceDelay * 1000ul){
    lapButtonPressed = true;
  }
  laplastInterruptTime = lapinterruptTime;
}
