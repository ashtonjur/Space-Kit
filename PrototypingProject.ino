#include <Adafruit_NeoPixel.h>
#include <Servo.h>

const int distancePin = A1;
const int lightPin = A0;
const int buttonPin = 2;
const int ledPin = 3; 
const int potLedPin = 4;
const int potServoPin = 5;
const int servo1Pin = 8;
const int servo2Pin = 9;

const int numPixels = 30;



const float MAX_RANG = 520.0;
const float ADC_SOLUTION = 1023.0;
const float TARGET_LUX = 750.0; 

const float SMOOTHING_FACTOR = 0.05;
const float OPEN_THRESHOLD_CM = 50.0;

Adafruit_NeoPixel strip(numPixels, ledPin, NEO_GRB + NEO_KHZ800);

Servo servo1;
Servo servo2;

float distanceCm = 0.0;
int lightRaw;
float luxValue;
float filteredLux = -1.0;

bool isManualMode = true;
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
  int rawDistance = analogRead(distancePin); 
  distanceCm = (rawDistance * MAX_RANG) / ADC_SOLUTION;

  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    isManualMode = false;
    delay(100);
  }
  lastButtonState = currentButtonState;

  if (distanceCm > OPEN_THRESHOLD_CM && !isManualMode) {
    isManualMode = false;
    delay(200);
  }

  int potServoRaw = analogRead(potServoPin);
  int angleOffset = map(potServoRaw, 0, 1023, 0, 90);

  int s1Angle = 90 + angleOffset;
  int s2Angle = 180 - angleOffset;

  servo1.write(s1Angle);
  servo2.write(s2Angle);

  int brightness = 0;

  if (distanceCm > OPEN_THRESHOLD_CM && isManualMode) {
    int potLedRaw = analogRead(potLedPin);
    brightness = map(potLedRaw, 0, 1023, 0, 255);

    Serial.print("Mode: MANUAL LED | Distance: ");
    Serial.print(distanceCm, 1);
    Serial.print(" cm | Servo1: ");
    Serial.print(s1Angle);
    Serial.print("° | Servo2: ");
    Serial.print(s2Angle);
    Serial.print("° | LED Brightness: ");
    Serial.print(brightness);
    Serial.println("/255");
  } if (distanceCm > OPEN_THRESHOLD_CM && !isManualMode) {
    lightRaw = analogRead(lightPin);
    luxValue = (lightRaw / ADC_SOLUTION) * 6000.0;

    if (filteredLux < 0) {
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
    Serial.print(" Lux | Servo1: ");
    Serial.print(s1Angle);
    Serial.print("° | Servo2: ");
    Serial.print(s2Angle);
    Serial.print("° | LED Brightness: ");
    Serial.print(brightness);
    Serial.println("/255");
  } if (distanceCm < OPEN_THRESHOLD_CM) {
    brightness = 0;
    Serial.print("CLOSED");
  }

  strip.setBrightness(brightness);
  for (int i = 0; i < numPixels; i++) {
    strip.setPixelColor(i, strip.Color(255, 147, 41));
  }
  strip.show();
  
  delay(30);
}