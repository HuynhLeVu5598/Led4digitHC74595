
#include <Led4digit74HC595.h>

Led4digit74HC595 myLedDisplay(11, 12, 13);  // Pins:(SCLK, RCLK, DIO)
int myNumber = 4826;


void setup() {
  myLedDisplay.setDecimalPoint(0);
}

void loop() {  
  myLedDisplay.loopShow();  
  myLedDisplay.setNumber(myNumber); 
}
