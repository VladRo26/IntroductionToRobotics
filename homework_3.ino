const int floorButtonPins[] = {4, 3, 2};
const int floorLedPins[] = {8, 9, 10};
const int buzzerPin = 12;
const int operationalLedPin = 11;

byte floorButtonStates[] = {HIGH, HIGH, HIGH};
byte floorLedStates[] = {HIGH, HIGH, HIGH};

unsigned long floorTransitionTime = 0;
unsigned long operationLedBlinkTime = 0;
unsigned long floorStayTime = 2000;
unsigned long floorBlinkInterval = 200;

unsigned int elevatorState = 0;
//0 - elevator it s not moving
//1 - elevator it s moving
//2 - elevator arrived at the desired floor
//3 - elevator it's closing/opening the door
unsigned int desiredFloor = 1;
unsigned int currentFloor_c = 0;
unsigned int currentFloor = 1;

unsigned long soundStartTime = 0;
unsigned long soundDuration1 = 1000; 
unsigned long soundDuration2 = 500; 

byte readings[] = {LOW,LOW,LOW};
byte lastReadings[] = {LOW,LOW,LOW};

unsigned int debounceDelay = 100;

unsigned int lastDebounceTimes[] = {0,0,0};

int movementTone = 1100;
int arrivedTone = 1200;
int doorTone = 1300;


void debounce(int buttonPin, byte &buttonState, byte &reading, byte &lastReading, unsigned int &lastDebounceTime, unsigned int debounceDelay) {
  reading = digitalRead(buttonPin);

  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
    }
  }
  lastReading = reading;
}

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(floorButtonPins[i], INPUT_PULLUP);
    pinMode(floorLedPins[i], OUTPUT);
  }
  pinMode(operationalLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  for (int i = 0; i < 3; i++) {
    debounce(floorButtonPins[i],floorButtonStates[i],readings[i],lastReadings[i],lastDebounceTimes[i],debounceDelay);
  }

  if (elevatorState == 0) {
    noTone(buzzerPin);
    digitalWrite(operationalLedPin, HIGH);
    for (int i = 0; i < 3; i++) {
      if (floorButtonStates[i] == LOW) {
        desiredFloor = i + 1;
      }
    }

    if (desiredFloor != currentFloor) {
      elevatorState = 1;
      floorTransitionTime = millis();
      operationLedBlinkTime = millis();
      soundStartTime = millis();
    } else {
      for (int i = 0; i < 3; i++) {
        if (currentFloor == i + 1) {
          digitalWrite(floorLedPins[i], HIGH);
        } else {
          digitalWrite(floorLedPins[i], LOW);
        }
      }
    }
  }

  if (elevatorState == 1) {
    if (millis() - operationLedBlinkTime >= floorBlinkInterval) {
      digitalWrite(operationalLedPin, !digitalRead(operationalLedPin));
      operationLedBlinkTime = millis();
    }
    if (millis() - soundStartTime <= soundDuration2){
        tone(buzzerPin, doorTone);
    }else
    {
        tone(buzzerPin, movementTone);
    }
    if (desiredFloor < currentFloor && elevatorState != 0) {
      currentFloor_c = currentFloor;
      if (currentFloor_c >= desiredFloor) {
        digitalWrite(floorLedPins[currentFloor_c - 1], HIGH);
        if (millis() - floorTransitionTime >= floorStayTime) {
          digitalWrite(floorLedPins[currentFloor_c - 1], LOW);
          currentFloor -= 1;
          floorTransitionTime = millis();
          if (currentFloor == desiredFloor) {
            elevatorState = 2; 
            soundStartTime = millis(); 
          }
        }
      }
    } else if (desiredFloor > currentFloor && elevatorState != 0) {
      currentFloor_c = currentFloor;
      if (currentFloor_c < desiredFloor) {
        digitalWrite(floorLedPins[currentFloor_c - 1], HIGH);
        if (millis() - floorTransitionTime >= floorStayTime) {
          digitalWrite(floorLedPins[currentFloor_c - 1], LOW);
          currentFloor += 1;
          floorTransitionTime = millis();
          if (currentFloor == desiredFloor) {
            elevatorState = 2; 
            soundStartTime = millis();
          }
        }
      }
    }
  }

if (elevatorState == 2) {
    if (millis() - soundStartTime <= soundDuration1) {
      tone(buzzerPin, arrivedTone);
    } else {
      noTone(buzzerPin);
      elevatorState = 3;
      soundStartTime = millis(); 
    }
  }

  if (elevatorState == 3) {
    if (millis() - soundStartTime <= soundDuration2) {
      tone(buzzerPin, doorTone); 
    } else {
      noTone(buzzerPin);
      elevatorState = 0; 
    }
  }
}
