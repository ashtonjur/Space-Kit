#include <Adafruit_NeoPixel.h>

const int distancePin = A1;   
const int lightPin = A0;     
const int potPin = A3;
const int buttonPin = 2;
const int ledPin = A2;        
const int numPixels = 30;    

const float MAX_RANG = 520.0;    
const float ADC_SOLUTION = 1023.0;
const float TARGET_LUX = 750.0; 

const float SMOOTHING_FACTOR = 0.05;

Adafruit_NeoPixel strip(numPixels, ledPin, NEO_GRB + NEO_KHZ800);

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
}

void loop() {
  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    isManualMode = !isManualMode;
    delay(100);
  }
  lastButtonState = currentButtonState;

  int brightness = 0;

  if (isManualMode) {
    int potRaw = analogRead(potPin);
    brightness = map(potRaw, 0, 1023, 0, 255);
    int rawDistance = analogRead(distancePin); 
    distanceCm = (rawDistance * MAX_RANG) / ADC_SOLUTION;

    Serial.print("Mode: MANUAL | Distance: ");
    Serial.print(distanceCm, 1);
    Serial.print(" cm | ");
    Serial.print(" | LED Brightness: ");
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

    static int autoBrightness = 0;
    const float LUX_DEADZONE = 15.0;

    if (filteredLux < (TARGET_LUX - LUX_DEADZONE)) {
      if (autoBrightness < 255) autoBrightness += 2;
    } 
    else if (filteredLux > (TARGET_LUX + LUX_DEADZONE)) {
      if (autoBrightness > 0) autoBrightness -= 2;
    }

    brightness = autoBrightness;

    Serial.print("Mode: AUTO | Distance: ");
    Serial.print(distanceCm, 1);
    Serial.print(" cm | ");
    Serial.print(filteredLux, 0);
    Serial.print(" Lux | LED Brightness: ");
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