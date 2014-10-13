/**
 * Use this program to set the time of the clock in your RTC DS1307 to the
 * current time of the machine compiling and pushing the sketch.  This is
 * necessary, usually just once, when you put together your RTC.
 */

#include <TinyWireM.h>
#include <TinyRTClib.h>

RTC_DS1307 rtc;

#ifdef __AVR_ATtiny85__
// Trinket has an led on pin 1
#define PIN 1
#else
// Arduino boards typically have an led on pin 13.
#define PIN 13
#endif


void setup()
{
  // Begin I2C
  TinyWireM.begin();
  rtc.begin();
  rtc.adjust(DateTime(__DATE__, __TIME__));

  pinMode(PIN,OUTPUT);
  for(uint8_t i=0; i<10; i++) {
      digitalWrite(PIN, HIGH);
      delay(500);
      digitalWrite(PIN, LOW);
      delay(500);
  }
}

void loop() { }

