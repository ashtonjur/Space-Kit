#include <Adafruit_NeoPixel.h>
#include <Servo.h>

const int distancePin = A1;
const int lightPin = A0;
const int potLedPin = A3;
const int potServoPin = A4;
const int buttonPin = 2;
const int ledPin = A2; 
const int numPixels = 30;

const int servo1Pin = 9;
const int servo2Pin = 10;

const float MAX_RANG = 520.0;
const float ADC_SOLUTION = 1023.0;
const float TARGET_LUX = 750.0; 

const float SMOOTHING_FACTOR = 0.05;

Adafruit_NeoPixel strip(numPixels, ledPin, NEO_GRB + NEO_KHZ800);

Servo servo1;
Servo servo2;

float distanceCm = 0.0;
int lightRaw;
float luxValue;
float filteredLux = -1.0;

bool isManualMode = false;
bool lastButtonState = HIGH;

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); 
  pinMode(buttonPin, INPUT_PULLUP);

  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
}

void loop() {
  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    isManualMode = !isManualMode;
    delay(100);
  }
  lastButtonState = currentButtonState;

  int potServoRaw = analogRead(potServoPin);
  int servoAngle = map(potServoRaw, 0, 1023, 0, 180);
  servo1.write(servoAngle);
  servo2.write(servoAngle);

  int brightness = 0;

  if (isManualMode) {
    int potLedRaw = analogRead(potLedPin);
    brightness = map(potLedRaw, 0, 1023, 0, 255);
    
    int rawDistance = analogRead(distancePin); 
    distanceCm = (rawDistance * MAX_RANG) / ADC_SOLUTION;

    Serial.print("Mode: MANUAL LED | Distance: ");
    Serial.print(distanceCm, 1);
    Serial.print(" cm | Servo: ");
    Serial.print(servoAngle);
    Serial.print("° | LED Brightness: ");
    Serial.print(brightness);
    Serial.println("/255");
  } else {
    int rawDistance = analogRead(distancePin); 
    distanceCm = (rawDistance * MAX_RANG) / ADC_SOLUTION;

    lightRaw = analogRead(lightPin);
    luxValue = (lightRaw / ADC_SOLUTION) * 6000.0;

    if (filteredLux < 0) {
      filteredLux = luxValue;
    } else {
      filteredLux = (luxValue * SMOOTHING_FACTOR) + (filteredLux * (1.0 - SMOOTHING_FACTOR));
    }

    float missingLux = TARGET_LUX - filteredLux;
    if (missingLux < 0) missingLux = 0;

    brightness = (missingLux / TARGET_LUX) * 255.0;
    if (brightness > 255) brightness = 255;
    if (brightness < 0) brightness = 0;

    Serial.print("Mode: AUTO LED | Distance: ");
    Serial.print(distanceCm, 1);
    Serial.print(" cm | ");
    Serial.print(filteredLux, 0);
    Serial.print(" Lux | Servo: ");
    Serial.print(servoAngle);
    Serial.print("° | LED Brightness: ");
    Serial.print(brightness);
    Serial.println("/255");
  }

  strip.setBrightness(brightness);
  for (int i = 0; i < numPixels; i++) {
    strip.setPixelColor(i, strip.Color(255, 147, 41));
  }
  strip.show();
  
  delay(30);
}