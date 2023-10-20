const int pin_input_1 = A0;
const int pin_input_2 = A1;
const int pin_input_3 = A2;


int input_value_1 = 0;
int input_value_2 = 0;
int input_value_3 = 0;


const int rgb_Pin_1 = 11;
const int rgb_Pin_2 = 10;
const int rgb_Pin_3 = 9;


int rgb_value_1 = 0;
int rgb_value_2 = 0;
int rgb_value_3 = 0;


const int minInputValue = 0;
const int maxInputValue = 1023;
const int minMappedValue = 0;
const int maxMappedValue = 255;




void Control_RGB(const int pin_input_,int input_value_, int rgb_value_,const int rgb_Pin_)
{
  input_value_  = analogRead(pin_input_);
  rgb_value_ = map(input_value_,minInputValue,maxInputValue,minMappedValue,maxMappedValue);
  analogWrite(rgb_Pin_,rgb_value_);

}


void setup() {
  // put your setup code here, to run once:
  pinMode(rgb_Pin_1,OUTPUT);
  pinMode(rgb_Pin_2,OUTPUT);
  pinMode(rgb_Pin_3,OUTPUT);



}

void loop() {
  // put your main code here, to run repeatedly:
  // input_value_1  = analogRead(pin_input_1);
  // Serial.println(pin_input_1);
  // rgb_value_1 = map(input_value_1,0,1023,0,255);
  // analogWrite(rgb_Pin_1,rgb_value_1);

  //  input_value_2  = analogRead(pin_input_2);
  // Serial.println(pin_input_2);
  // rgb_value_2 = map(input_value_2,0,1023,0,255);
  // analogWrite(rgb_Pin_2,rgb_value_2);

  // input_value_3  = analogRead(pin_input_3);
  // Serial.println(pin_input_3);
  // rgb_value_3 = map(input_value_3,0,1023,0,255);
  // analogWrite(rgb_Pin_3,rgb_value_3);
  Control_RGB(pin_input_1,input_value_1,rgb_value_1,rgb_Pin_1);
  Control_RGB(pin_input_2,input_value_2,rgb_value_2,rgb_Pin_2);
  Control_RGB(pin_input_3,input_value_3,rgb_value_3,rgb_Pin_3);


}
