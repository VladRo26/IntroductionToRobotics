#include <EEPROM.h>


const int trigPin = 9;
const int echoPin = 10;
const int redPin = 11;
const int bluePin = 5;
const int greenPin = 6;
long duration = 0;
int distance = 0;
int photocellPin = 0;
int photocellValue;
byte subMenuOption = 0;
byte option = 0;
bool configuration = false;
bool inSubMenu = false;
int redValue = 0;
int greenValue = 0;
int blueValue = 0;
bool isReadingSensors = false;  
unsigned long lastSensorReadTime = 0;  

const int ADDR_MIN_ULTRASONIC_THRESHOLD = 1 + sizeof(int);
const int ADDR_SENSOR_SAMPLING_INT = ADDR_MIN_ULTRASONIC_THRESHOLD + sizeof(int);
const int ADDR_MAX_LDR_VALUE = ADDR_SENSOR_SAMPLING_INT + sizeof(int); 
const int ADDR_TOGGLE_AUTOMATIC_LED = ADDR_MAX_LDR_VALUE + sizeof(int); 
const int ADDR_RED_VALUE = ADDR_TOGGLE_AUTOMATIC_LED + sizeof(int);
const int ADDR_GREEN_VALUE = ADDR_RED_VALUE + sizeof(int);
const int ADDR_BLUE_VALUE = ADDR_GREEN_VALUE + sizeof(int);


const int DEFAULT_MIN_ULTRASONIC_THRESHOLD = 20;
const int DEFAULT_SENSOR_SAMPLING_INT = 1;
const int DEFAULT_MAX_LDR_VALUE = 300;
const int DEFAULT_TOGGLE_AUTOMATIC_LED = 1;
const int DEFAULT_RED_VALUE = 255;
const int DEFAULT_GREEN_VALUE = 255;
const int DEFAULT_BLUE_VALUE = 255;


int minUltraSonicThreshold;
int sensorSamplingInt;
int maxLdrValue;
int toggleAutomaticLed;


struct SensorData {
  int distance;
  int brightness;
};

SensorData sensorReadings[10];
int currentReadingIndex = 0;


void saveSensorReadings() {
  int startAddress = ADDR_BLUE_VALUE + sizeof(int);
  EEPROM.put(startAddress, sensorReadings);
}

void loadSensorReadings() {
  int startAddress = ADDR_BLUE_VALUE + sizeof(int);
  EEPROM.get(startAddress, sensorReadings);
}

void initializeDefaultEEPROMValues() {
    const int EEPROM_INITIALIZED_FLAG = 12345; 
    int initializedFlag;

    EEPROM.get(0, initializedFlag);

    if (initializedFlag != EEPROM_INITIALIZED_FLAG) {
        EEPROM.put(ADDR_MIN_ULTRASONIC_THRESHOLD, DEFAULT_MIN_ULTRASONIC_THRESHOLD);
        EEPROM.put(ADDR_SENSOR_SAMPLING_INT, DEFAULT_SENSOR_SAMPLING_INT);
        EEPROM.put(ADDR_MAX_LDR_VALUE, DEFAULT_MAX_LDR_VALUE);
        EEPROM.put(ADDR_TOGGLE_AUTOMATIC_LED, DEFAULT_TOGGLE_AUTOMATIC_LED);

        EEPROM.put(ADDR_RED_VALUE, DEFAULT_RED_VALUE);
        EEPROM.put(ADDR_GREEN_VALUE, DEFAULT_GREEN_VALUE);
        EEPROM.put(ADDR_BLUE_VALUE, DEFAULT_BLUE_VALUE);

        EEPROM.put(0, EEPROM_INITIALIZED_FLAG);
    }

    EEPROM.get(ADDR_MIN_ULTRASONIC_THRESHOLD, minUltraSonicThreshold);
    EEPROM.get(ADDR_SENSOR_SAMPLING_INT, sensorSamplingInt);
    EEPROM.get(ADDR_MAX_LDR_VALUE, maxLdrValue);
    EEPROM.get(ADDR_TOGGLE_AUTOMATIC_LED, toggleAutomaticLed);

    EEPROM.get(ADDR_RED_VALUE, redValue);
    EEPROM.get(ADDR_GREEN_VALUE, greenValue);
    EEPROM.get(ADDR_BLUE_VALUE, blueValue);
}

void displayLast10SensorReadings() {
    Serial.println("Last 10 Sensor Readings:");
    Serial.println("Distance (cm) | Brightness");
    Serial.println("------------------------");

    for (int i = currentReadingIndex - 1; i >= 0; i--) {
        Serial.print(sensorReadings[i].distance);
        Serial.print("           | ");
        Serial.println(sensorReadings[i].brightness);
    }

    if(currentReadingIndex < 10) {
        for (int i = 9; i >= currentReadingIndex; i--) {
            Serial.print(sensorReadings[i].distance);
            Serial.print("           | ");
            Serial.println(sensorReadings[i].brightness);
        }
    }
    Serial.println("\n");
    inSubMenu = false;
    printMenu();
}

void setup() {
  Serial.begin(9600); 
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT); 
  pinMode(redPin,OUTPUT);
  pinMode(greenPin,OUTPUT);
  pinMode(bluePin,OUTPUT);
  printMenu();
  loadSensorReadings();
  initializeDefaultEEPROMValues();

}

void printMenu() {
  Serial.println("\n\n");
  Serial.println("Choose an option by entering a value:\n");
  Serial.println("1. Sensor Settings\n");
  Serial.println("2. Reset Logger Data\n");
  Serial.println("3. System Status\n");
  Serial.println("4. RGB Led Control\n");
}

void printSubMenu(byte option) {
  switch (option) {
    case '1':
      Serial.println("1.1 Sensors Sampling Interval\n");
      Serial.println("1.2 Ultrasonic Alert Threshold\n");
      Serial.println("1.3 LDR Alert Threshold.\n");
      Serial.println("1.4 Back");
      break;
    case '2':
      Serial.println("Are you sure you want to delete all data?\n");
      Serial.println("2.1 Yes\n");
      Serial.println("2.2 No\n");
      break;
    case '3':
      Serial.println("3.1 Current Sensor Readings\n");
      Serial.println("3.2 Current Sensor Settings\n");
      Serial.println("3.3 Display Logged Data.\n");
      Serial.println("3.4 Back");
      break;
    case '4':
      Serial.println("4.1 Manual Color Control\n");
      Serial.println("4.2 LED: Toggle Automatic ON/OFF\n");
      Serial.println("4.3 Back\n");
      break;
  }
}

void selectMenu() {
  if (!inSubMenu && Serial.available() > 0) {
    option = Serial.read();
    switch (option) {
      case '1':
        inSubMenu = true;
        Serial.println("Option 1 selected: Sensor Settings\n");
        printSubMenu(option);
        break;
      case '2':
        inSubMenu = true;
        Serial.println("Option 2 selected: Reset Logger Data\n");
        printSubMenu(option);
        break;
      case '3':
        inSubMenu = true;
        Serial.println("Option 3 selected: System Status\n");
        printSubMenu(option);
        break;
      case '4':
        inSubMenu = true;
        Serial.println("Option 4 selected: RGB Led Control\n");
        printSubMenu(option);
        break;
      default:
        Serial.println("Invalid option. Please select a valid option.\n");
        break;
    }
  } else if (inSubMenu && Serial.available() > 0) {
    subMenuOption = Serial.read();
    switch (option) {
      case '1':
        switch (subMenuOption) {
          case '1':
            samplingInterval();
            break;
          case '2':
            minTresholdUS();
            break;
          case '3':
            ldrAlertSensor();
            break;
          case '4':
            inSubMenu = false;
            printMenu();
            break;
          default:
            Serial.println("Invalid submenu option");
            break;
        }
        break;
      case '2':
        switch (subMenuOption) {
          case '1':
            clearEEPROM();
            inSubMenu = false;
            printMenu();
            break;
          case '2':
            inSubMenu = false;
            printMenu();
            break;
          default:
            Serial.println("Invalid submenu option");
            break;
        }
        break;
      case '3':
        switch (subMenuOption) {
          case '1':
          isReadingSensors = true;
          Serial.println("Printing sensor readings. Press 'x' to stop.");
          break;
          case '2':
            Serial.println("Value for sensors US/Ldr:");
            Serial.println(minUltraSonicThreshold);
            Serial.println(maxLdrValue);
            Serial.println("Value for sampling:");
            Serial.println(sensorSamplingInt);
            Serial.print("Toggle value:");
            Serial.println(toggleAutomaticLed);
            inSubMenu = false;
            printMenu();
            break;
          case '3':
            displayLast10SensorReadings();
            break;
          case '4':
            inSubMenu = false;
            printMenu();
            break;
          default:
            Serial.println("Invalid submenu option");
            break;
        }
        break;
      case '4':
        switch (subMenuOption) {
          case '1':
          Serial.println("Red:");
          while(Serial.available() <= 0);
            if (Serial.available() > 0) {
              redValue = Serial.parseInt();
              Serial.print("New value set to: ");
              Serial.println(redValue);
              EEPROM.put(ADDR_RED_VALUE, redValue);

            }

            Serial.println("Green:");
            while(Serial.available() <= 0);
            if (Serial.available() > 0) {
              greenValue = Serial.parseInt();
              Serial.print("New value set to: ");
              Serial.println(greenValue);
              EEPROM.put(ADDR_GREEN_VALUE, greenValue);
            }
            Serial.println("Blue:");
            while(Serial.available() <= 0);
            if (Serial.available() > 0) {
              blueValue = Serial.parseInt();
              Serial.print("New value set to: ");
              Serial.println(blueValue);
              EEPROM.put(ADDR_BLUE_VALUE, blueValue);
            inSubMenu = false;
            printMenu();
            }
            break;
          case '2':
          Serial.println("Toggle automatic 1.ON/2.OFF");
          while(Serial.available() <= 0);
          if (Serial.available() > 0) {
            toggleAutomaticLed = Serial.parseInt();
            EEPROM.put(ADDR_TOGGLE_AUTOMATIC_LED,toggleAutomaticLed);
          }
          ledState();
          inSubMenu = false;
          printMenu();
            break;
          case '3':
            inSubMenu = false;
            printMenu();
            break;
          default:
            Serial.println("Invalid submenu option");
            break;
        }
        break;
      default:
        Serial.println("Invalid submenu option.");
        break;
    }
  }
}

void minTresholdUS() {
  Serial.println("Enter the new value:");
  while (Serial.available() <= 0);
  if (Serial.available() > 0) {
    minUltraSonicThreshold = Serial.parseInt();
    EEPROM.put(ADDR_MIN_ULTRASONIC_THRESHOLD, minUltraSonicThreshold);
    Serial.print("New value set to: ");
    Serial.println(minUltraSonicThreshold);
    inSubMenu = false;
    printMenu();
  }
}

void samplingInterval() {
  Serial.println("Enter the new value:");
  while (Serial.available() <= 0);
  if (Serial.available() > 0) {
    sensorSamplingInt = Serial.parseInt();
    EEPROM.put(ADDR_SENSOR_SAMPLING_INT, sensorSamplingInt);
    Serial.print("New sampling interval set to: ");
    Serial.println(sensorSamplingInt); 
    inSubMenu = false;
    printMenu();
  }
}


void ldrAlertSensor() {
  Serial.println("Enter the new value:");
  while (Serial.available() <= 0);
  if (Serial.available() > 0) {
    maxLdrValue = Serial.parseInt();
    EEPROM.put(ADDR_MAX_LDR_VALUE, maxLdrValue);
    Serial.print("New LDR value set to: ");
    Serial.println(maxLdrValue); 
    inSubMenu = false;
    printMenu();
  }
}


void clearEEPROM() {
  int eepromSize = EEPROM.length();

  for (int i = 0; i < eepromSize; ++i) {
    EEPROM.update(i, 0);
  }

  EEPROM.put(ADDR_MIN_ULTRASONIC_THRESHOLD, DEFAULT_MIN_ULTRASONIC_THRESHOLD);
  EEPROM.put(ADDR_SENSOR_SAMPLING_INT, DEFAULT_SENSOR_SAMPLING_INT);
  EEPROM.put(ADDR_MAX_LDR_VALUE, DEFAULT_MAX_LDR_VALUE);
  EEPROM.put(ADDR_TOGGLE_AUTOMATIC_LED, DEFAULT_TOGGLE_AUTOMATIC_LED);

  EEPROM.get(ADDR_MIN_ULTRASONIC_THRESHOLD, minUltraSonicThreshold);
  EEPROM.get(ADDR_SENSOR_SAMPLING_INT, sensorSamplingInt);
  EEPROM.get(ADDR_MAX_LDR_VALUE, maxLdrValue);
  EEPROM.get(ADDR_TOGGLE_AUTOMATIC_LED, toggleAutomaticLed);

  Serial.println("The data has been reseted");
}

void distanceSensor(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
}

void colorLed(int red, int green, int blue){
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
}

void ledState(){
  EEPROM.get(ADDR_TOGGLE_AUTOMATIC_LED,toggleAutomaticLed);
   if (toggleAutomaticLed == 1){
    automaticLed();
   }else 
  if(toggleAutomaticLed == 2){
    colorLed(redValue,greenValue,blueValue);
  }
}

void automaticLed(){
  EEPROM.get(ADDR_MIN_ULTRASONIC_THRESHOLD, minUltraSonicThreshold);
  EEPROM.get(ADDR_MAX_LDR_VALUE, maxLdrValue);
  if(distance <= minUltraSonicThreshold || photocellValue>=maxLdrValue ){
    colorLed(255,0,0);
  }else{
    colorLed(0,255,0);
  }
}

void brightnessSensor(){
  photocellValue = analogRead(photocellPin);
}

void printSensorReadings() {
  brightnessSensor();
  distanceSensor();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, Brightness: ");
  Serial.println(photocellValue);
}
void loop() {
  brightnessSensor();
  distanceSensor();

  sensorReadings[currentReadingIndex].distance = distance;
  sensorReadings[currentReadingIndex].brightness = photocellValue;
  currentReadingIndex = (currentReadingIndex + 1) % 10; 

  saveSensorReadings();

  ledState();


  if (isReadingSensors) {
    if (millis() - lastSensorReadTime >=  sensorSamplingInt* 1000) {
      brightnessSensor();
      distanceSensor();

      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.print(" cm, Brightness: ");
      Serial.println(photocellValue);

      lastSensorReadTime = millis();  
    }

    if (Serial.available() > 0 && Serial.read() == 'x') {
      isReadingSensors = false;
      inSubMenu = false;
      printMenu();
    }
  } else {
    selectMenu();
  }
}
