
/**
 * This is the 60 pixel neopixel clock program written by Jason McHugh.
 * 
 * This is a copy of my original program that I've forked and extended.  
 * In December 2014 my mother came to visit and remarked, several times!, 
 * how much she liked my clock.  I decided, with the prompting of a very
 * good friend, to make one for her.
 *
 * In making this version I decided to be smart this time in two 
 * significant ways.
 *
 * 1/  In the original version I challenged myself by putting the program
 * on a trinket - which has very limited memory space.  After putting on the
 * TinyWireM and TinyRTClib libraries I had only 3,146 bytes of programmer
 * space.  This time I'm using an Arduino Pro Mini with a full 8k of space.
 * This is huge and really makes the entire project easier.
 *
 * 2/  Day light savings time wasn't handled in my prior project so I'm adding
 * two buttons to support moving the hour and the minutes.  Minutes shouldn't 
 * be necessary but in case they want to synchronize with other clocks in the
 * house.
 * 
 * Project contents:
 *
 *   60 pixel ring:    https://www.adafruit.com/product/1768
 *   Arduino Pro Mini: https://www.sparkfun.com/products/11113
 *   RTC DS1307:       https://www.adafruit.com/product/264
 *   Pushbuttons:      https://www.adafruit.com/products/1009
 * 
 * End results: 
 * 
 *   1/  Interrupt service routines for buttons
 *   2/  Nicely animated clocks
 *   3/  Super happy Mom
 *
 * If you are interested in seeing a build of this project that works under
 * different settings then look around for the original Neopixel 60 clock
 * project.
 *
 * This project requires:
 *
 *   https://github.com/adafruit/RTClib
 *   https://github.com/adafruit/Adafruit_NeoPixel
 *
*/

// Use the serial libraries for debugging
#include <Serial>
#include <Wire.h>
#include <RTClib.h>
#define PIN 5

#include <Adafruit_NeoPixel.h>

// Uncomment out the following line to set the time.  Remember
// to reflash the same program with this commented out so that subsequent
// restarts don't reset the time.
// #define SET_TIME

// Uncomment out the following, but only when SET_TIME is defined,
// in order to test specific time values.
// #define USE_MANUAL_TEST_TIME

// Neopixel pin for the lights
#define NUMBER_PIXELS 60
#define BRIGHTNESS 128 // set max brightness

#define START_VALUE 0xfe

uint8_t hourTriggered = 0;
uint8_t minuteTriggered = 0;
uint32_t triggerTime;
uint8_t hourOffset = 0;
uint8_t minuteOffset = 0;

RTC_DS1307 rtc; // Clock object uses the real time clock module

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_PIXELS, PIN, NEO_GRB + NEO_KHZ800); // strip object

byte priorHourPixel = START_VALUE;
byte priorMinuteVal = START_VALUE;
byte priorSecondVal = START_VALUE;
boolean shownFifteen = false;

void setUniformRingColor( uint32_t color) {
    for(uint16_t i=0; i<NUMBER_PIXELS; i++) {
        strip.setPixelColor(i, color );
    }
}

void showOrientation() {
  Serial.println( "Going to show orientation" );
  setUniformRingColor(0);
  int numLeds = NUMBER_PIXELS;

  Serial.println( F("Top (Red): 0") );
  Serial.print( F("Right (Green): " ));
  Serial.println( (int) numLeds/4 );
  Serial.print( F("Bottom (Blue): " ));
  Serial.println( (int) numLeds/2 );
  Serial.print( F("Left (White): " ) );
  Serial.println( (int) numLeds/4*3 );
  
  for( int i = 0; i < 10; i++ ) {
      strip.setPixelColor(0, 0xff0000);
      strip.setPixelColor(numLeds/4, 0x00ff00);
      strip.setPixelColor(numLeds/2, 0x0000ff);
      strip.setPixelColor(numLeds/4*3, 0xffffff);
      strip.show();
      delay(150);
      setUniformRingColor(0);
      strip.show();
      delay(150);
  }
}

void setup () {
  strip.begin();
  strip.setBrightness(BRIGHTNESS); // set brightness
  
  Wire.begin();
  rtc.begin();

  Serial.begin(9600);
  Serial.println( F("Running Neopixel Clock 60 with serial debugging" ));
  Serial.print( F("Date: " ));
  Serial.print( __DATE__ );
  Serial.print( F(", Time: ") );
  Serial.println( __TIME__ );
  Serial.print( F("Initializing light pin to " ));
  Serial.println( PIN );
  showOrientation();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
   
#ifdef SET_TIME 
#ifdef USE_MANUAL_TEST_TIME
  const char *date = "Oct 12 2014";
  const char *time = "13:59:52";
#else
  const char *date = __DATE__;
  const char *time = __TIME__;;
#endif
 
  Serial.print( F("Going to manually set the RTC time to: "));
  Serial.print( date );
  Serial.print( F(" " ));
  Serial.println( time );
  rtc.adjust(DateTime( date, time ) );
#endif

  while(! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
  
  // Enable interrupt 0 (pin 2) which is connected to a button
  // that controls the hour increment.
  attachInterrupt(0, incrementHour, RISING);

  // Enable interrupt 1 (pin 3) which is connected to a button
  // that controls the minute increment.
  attachInterrupt(1, incrementMinute, RISING);
}


/**
 * This routine will add a red, green, and blue color value to the current
 * color value for a pixel.  This creates a blend of the two colors. 
 *
 * The specific index for the pixel to be set can have a value -NUMBER_PIXELS
 * to NUMBER_PIXELS and it will be normalized into the range of 0 ->
 * NUMBER_PIXELS - 1.
 */
void addColorToPixel( int pixel, int red, int green, int blue ) { 
   if( pixel == NUMBER_PIXELS ) { 
     pixel = 0;
   } else if ( pixel < 0 ) { 
     pixel = NUMBER_PIXELS + pixel;
   }
   uint32_t currentPixelColor = strip.getPixelColor(pixel);
   
   int currentRed = (currentPixelColor & 0xFF0000) >> 16;
   int currentGreen = (currentPixelColor & 0x00FF00) >> 8;
   int currentBlue = currentPixelColor & 0x0000FF;
   
   strip.setPixelColor(pixel, currentRed + red, currentGreen + green, currentBlue + blue);
}

/**
 * Adds a color to a pixel, via addColorToPixel, and then does the same for
 * the surrounding pixels but at a reduced intensity.
 */
void addColorToPixelAndShadeAround( int pixel, uint8_t red, uint8_t green, uint8_t blue ) { 
    addColorToPixel( pixel, red, green, blue);
    red = red >> 4;
    green = green >> 4;
    blue = blue >> 4;
    
    addColorToPixel(pixel+1,red,green,blue);
    addColorToPixel(pixel-1,red,green,blue);
}

/**
 * Draw a single frame of a transition.  
 *
 * This method takes in the pixel that it will be animating, what the current
 * step of the animation is, the maximum number of steps, and finally the
 * color for the central pixel.  
 *
 * The animation frame will 'bleed' the color from the initial three pixels
 * into four pixels - the original three and the next pixel.
 */
void showTransitionStep( int pixel, int curTransitionStep, int maxTransitionSteps, int red, int green, int blue ) { 

   // The pixel that we are leaving
   addColorToPixel( pixel - 2, 
       red >> (4+curTransitionStep), 
       green >> (4+curTransitionStep), 
       blue >> (4+curTransitionStep) );

   // The pixel that used to be the brightest but now will be shaded dimly and
   // which preceeds the actual pixel for the time.
   addColorToPixel( pixel - 1, red >> curTransitionStep, green >> curTransitionStep, blue >> curTransitionStep );

   // The pixel that will now be the brightest and center.
   addColorToPixel( pixel, red >> (4-curTransitionStep), green >> (4-curTransitionStep), blue >> (4-curTransitionStep) );

   // Finally the pixel that follows the brightest which will become shaded
   // and bright as the transition nears the max steps.
   addColorToPixel( pixel+1, 
       red >> ( 4 + maxTransitionSteps - curTransitionStep), 
       green >> ( 4 + maxTransitionSteps - curTransitionStep), 
       blue >> ( 4 + maxTransitionSteps - curTransitionStep) );
}


/**
 * Show the transition from one time to the next.
 */
void transition( int hourVal, byte minuteVal, byte secondVal ) { 
  uint8_t amount = 0xff;
  int time = 200;
  int maximumTransitions = 4;
  int hourPixel = map( hourVal, 0, 719, 0, 59 );

  for( int i = 0; i <= maximumTransitions; i++ ) { 
       setUniformRingColor(0);
       
       if( priorHourPixel != hourPixel ) { 
         showTransitionStep( hourPixel, i, maximumTransitions, 0xff, 0, 0 );
       } else {
         addColorToPixelAndShadeAround( hourPixel, 0xff, 0, 0 );
       }
       
       if( priorMinuteVal != minuteVal ) { 
         showTransitionStep( minuteVal, i, maximumTransitions, 0, 0xff, 0 );
       } else {
         addColorToPixelAndShadeAround( minuteVal, 0, 0xff, 0 );
       }
       showTransitionStep( secondVal, i, maximumTransitions, 0, 0, 0xff);
       
       strip.show();
       delay(time);
  }
  priorHourPixel = hourPixel;
}

uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void showColorWheel() {
  uint16_t i, j;

  // Turn down the brightness to be safe.  There are 60 pixels and at full brightness and with
  // all white then each would be about 60 milliamps.  This is 3.6 amps total.  I'm using a 2
  // amp power supply.  I've set the default BRIGHTNESS to be a safe value even with all white
  // but here I'm going to decrease it just a bit to be extra safe.
  strip.setBrightness(BRIGHTNESS*0.85);
  for(j=256*4; j>0; j--) { 
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(3);
  }
  strip.setBrightness(BRIGHTNESS);
}

void showFifteenMinuteIncrements(byte minuteVal) {
  uint32_t color;
  if( minuteVal == 0 ) { 
    
    showColorWheel();
    return;
  } else if( minuteVal == 15 ) { 
    color = 0x00ff00;
  } else if( minuteVal == 30 ) { 
    color = 0xff0000;
  } else if( minuteVal == 45 ) { 
    color = 0x0000ff;
  } else {
    color = 0xff00ff;
  }
  for( int i = 0; i < 5; i++ ) { 
        setUniformRingColor( color );
        strip.show();
        delay(300);
        setUniformRingColor( 0x0 );
        strip.show();
        delay(300);
    }
}

void loop() 
{
  uint32_t currentTime = millis();
  
  // Look to see if the interrupt routines registered a button press.  
  // If so then acknowledge the trigger and, if it is the first pass
  // through this main loop since it has been triggered, run the routine
  // to process the button press.
  if( hourTriggered == 1 ) { 
    hourTriggered = 0;
    Serial.println( "Hour triggered" );
    if (triggerTime == 0) {
      Serial.println( "Incrementing hour offset" );
      triggerTime = currentTime;
      incrementHourOffset();
    }
  }
  if( minuteTriggered == 1 ) { 
    minuteTriggered = 0;
    Serial.println( "Minute triggered" );
    if (triggerTime == 0) {
      Serial.println( "Incrementing minute" );
      triggerTime = currentTime;
      incrementMinuteOffset();
    }
  }
  
  // Handle a uint32_t wrapping around every couple of days or so.
  if( triggerTime + 750 < triggerTime ) { 
    triggerTime = 0;
  }  
  
  // After some amount of times has passed allow another trigger event to occur.
  if( triggerTime != 0 && currentTime > triggerTime + 750 ) { 
    triggerTime = 0;
  }
  
  // Get the current time
  DateTime n = rtc.now(); 

  byte secondVal = n.second();  
  byte minuteVal = n.minute();  
 
  // Since hour granularity is too coarse - we have 60 pixels and there are
  // only 12 hour values - we translate the raw hour value into an hourVal
  // which is the number of minutes that have elapsed within the 12 hour
  // period.  This gives us a range from 0 to 719 which can be mapped to the
  // ring's 60 pixels nicely.
  byte hour = n.hour();
  hour += hourOffset;
  while( hour >= 12 ) { 
    hour -= 12;
  }
  
  minuteVal += minuteOffset;
  if( minuteVal > 60 ) { 
    minuteVal -= 60;
  }
  
  uint16_t hourVal = ((uint16_t)hour) * 60 + ( uint16_t) minuteVal;
  
  Serial.print( "currentHour: " );
  Serial.print( hour );
  Serial.print( ", hourVal: " );
  Serial.print( hourVal );
  Serial.print( ", currentMinute: " );
  Serial.print( minuteVal );
  Serial.print( ", currentSecond: " );
  Serial.println( secondVal );
  
  if( priorSecondVal == secondVal ) { 
      return;
  }   
  
  transition( hourVal, minuteVal, secondVal );
  
  priorMinuteVal = minuteVal;
  priorSecondVal = secondVal;

  // Every fifteen minutes do a little something obvious to indicate that
  // time has passed.
  if(( minuteVal == 0 || minuteVal == 15 || minuteVal == 30 || minuteVal == 45 ) ) {
    if( shownFifteen == false ) { 
       showFifteenMinuteIncrements( minuteVal );
       shownFifteen = true;
    }
  } else { 
    shownFifteen = false;
  }
}

// ISR for hour button
void incrementHour() {
  hourTriggered = 1;
}

void incrementHourOffset() {
    hourOffset++;
    if( hourOffset == 24 ) { 
       hourOffset = 0;
    }
}


// ISR for minute button
void incrementMinute() {
  minuteTriggered = 1;
}

void incrementMinuteOffset() {
    minuteOffset++;
    if( minuteOffset == 60 ) { 
       minuteOffset = 0;
    }
}


