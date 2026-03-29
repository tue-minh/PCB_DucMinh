#include <Arduino.h>

// // put function declarations here:
// int myFunction(int, int);

void setup() {
  pinMode(PC13, OUTPUT);
}

void loop() {
  digitalWrite(PC13, HIGH);
  delay(1000);
  digitalWrite(PC13, LOW);
  delay(1000);
}
