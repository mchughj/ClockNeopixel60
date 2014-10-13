
/**
 * This is the 60 pixel neopixel clock program written by Jason McHugh.  
 * Feel free to drop me a line at mchughj@gmail.com for questions, comments,
 * criticisms, concerns, etc.  :)
 *
 * The backstory.
 *
 * I originally bought a 16 pixel neopixel ring, a trinket, and an RTC
 * module:
 *
 *   Trinket:        https://www.adafruit.com/product/1501
 *   16 pixel ring:  https://www.adafruit.com/product/1463
 *   RTC DS1307:     https://www.adafruit.com/product/264
 *
 * 
 * I used the 16 pixel ring in another project for my son Aidan (who went
 * camping with his 6th grade class, so I created a 2 way mirror with the ring
 * behind the mirror).  I saw the 60 pixel ring and decided to get one (well,
 * actually 4):
 *
 *   60 pixel ring:  https://www.adafruit.com/product/1768
 *
 * Squeezing a nicely animating LED clock into the 5,310 byte maximum of the
 * trinket wasn't easy.  Just using the TimeWireM and TinyRTClib libraries
 * used up 3,146 bytes of space.  This left just ~ 2k of space to do something
 * neat.  I'm sure if I were writing the assembly and really thinking hard
 * about how to do this then it wouldn't be too challenging.  But writing
 * 'regular' C code made it non-trivial.  
 *
 * I could have moved to an Arduino and not had to worry about the space but I
 * wanted to challenge myself a bit and fit it on the ATTiny85.  
 *
 * I wrote the follow after observing clocks and thinking about what I thought
 * the ideal animation/behavior would be.
 *
 * I use this program in two separate scenarios.  
 *
 * 1/  The first is using an 8mhz Trinket with a physical RTC module attached to
 * it.  This was my final intended configuration but it left very little space
 * for debugging problems.  
 *
 * In this first mode you have to have installed the TinyWireM and TinyRTClib
 * installed. 
 *    https://github.com/adafruit/TinyWireM
 *    https://github.com/adafruit/TinyRTCLib
 *
 *
 * 2/  The second scenario in which this program can be used is using a bare
 * Arduino.  No RTC is required in this configuration - the millis
 * implementation is used.  This mode allowed me to debug lots of problems
 * with my original implementation so you will see lots of Serial debugging
 * output and the ability to set a specific time.  In this second mode you
 * have to have installed the RTClib.
 *   https://github.com/adafruit/RTClib
 *
 *
 * These two modes of operation are just the specific builds that I have.
 * There is nothing magical about them and additional configurations can be
 * achieved by changing the code below.  
 *
 * To set the two modes you have to do two things:
 *
 * 1/  In the Arduino programming environment select either 'Uno' or
 * 'Adafruit Trinket 8mhz'.  Based on this setting everything else should be
 * derived.   (Remember to set the programmer to be "USBTinyISP" if you are
 * using the trinket!   If  you see lots of errors with "not in sync" then
 * this can be why.)
 *
 * 2/  Uncomment or comment out the Wire.h, RTClib.h, TinyWireM, and
 * TinyRTClib.  This shouldn't be necessary but apparently the Arduino
 * programming environment uses a simple scan of all #includes in order to
 * know what libraries might be used.  The preprocessor and directives are not
 * used which makes this, IMO, a bug.  So the workaround is to comment and
 * uncomment.
 *
*/

#ifndef __AVR_ATtiny85__
// I like pretty names so mapping the __AVR*__ into something shorter and
// prettier.
#define USE_UNO
#endif

#ifdef USE_UNO

// Use the serial libraries for debugging
#include <Serial>

// RTC Library for the timing code.  
// There is an amazing part of the Arduino programming environment and this is
// that the environment will scan for #includes _without_ caring about the
// preprocessor directives surrounding it.  This these lines need to be
// commented out in addition to the #define being set.
// #include <Wire.h>
// #include <RTClib.h>
#define PIN 2

#else

// Use the Tiny libraries for RTC and I2C.
#include <TinyWireM.h>
#include <TinyRTClib.h>
#define PIN 1
#endif

#include <Adafruit_NeoPixel.h>

// Neopixel pin for the lights
#define NUMBER_PIXELS 60
#define BRIGHTNESS 128 // set max brightness

#define START_VALUE 0xfe

#ifdef USE_UNO
RTC_Millis rtc;
#else
RTC_DS1307 rtc; // Clock object uses the real time clock module
#endif

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_PIXELS, PIN, NEO_GRB + NEO_KHZ800); // strip object

byte priorHourPixel = START_VALUE;
byte priorMinuteVal = START_VALUE;
byte priorSecondVal = START_VALUE;

void setUniformRingColor( uint32_t color) {
    for(uint16_t i=0; i<NUMBER_PIXELS; i++) {
        strip.setPixelColor(i, color );
    }
}

#ifdef USE_UNO

void showOrientation() {
  Serial.println( "Going to show orientation" );
  setUniformRingColor(0);
  int numLeds = NUMBER_PIXELS;

  Serial.println( "Top (Red): 0" );
  Serial.print( "Right (Green): " );
  Serial.println( (int) numLeds/4 );
  Serial.print( "Bottom (Blue): " );
  Serial.println( (int) numLeds/2 );
  Serial.print( "Left (White): " );
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
#endif

#define USE_MANUAL_TEST_TIME 1

void setup () {
  strip.begin();
  strip.setBrightness(BRIGHTNESS); // set brightness

#ifdef USE_UNO
  Serial.begin(9600);
  Serial.println( "Running Neopixel Clock 60 with serial debugging" );
  Serial.print( "Date: " );
  Serial.print( __DATE__ );
  Serial.print( ", Time: " );
  Serial.println( __TIME__ );
  Serial.print( "Initializing light pin to " );
  Serial.println( PIN );
  showOrientation();

#ifdef USE_MANUAL_TEST_TIME
  const char *date = "Oct 12 2014";
  const char *time = "13:59:55";
#else
  const char *date = __DATE__;
  const char *time = __TIME__;;
#endif

  Serial.print( "Going to manually set the RTC time to: ");
  Serial.print( date );
  Serial.print( " " );
  Serial.println( time );
  rtc.adjust(DateTime( date, time ) );
#else

  TinyWireM.begin();
  rtc.begin();

  if (! rtc.isrunning()) {
      for( int i = 0; i < 15; i++ ) {
         strip.setPixelColor( 0, 0xff0000 );
         delay(25000);
      }         
  }
#endif
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

void showFifteenMinuteIncrements() {
    for( int i = 0; i < 5; i++ ) { 
        setUniformRingColor( 0x0000ff );
        strip.show();
        delay(300);
        setUniformRingColor( 0x0 );
        strip.show();
        delay(300);
    }
}

void loop() 
{

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
  if( hour >= 12 ) { 
    hour -= 12;
  }
  uint16_t hourVal = ((uint16_t)hour) * 60 + ( uint16_t) minuteVal;
  
#ifdef USE_UNO
  Serial.print( "currentHour: " );
  Serial.print( hour );
  Serial.print( ", hourVal: " );
  Serial.print( hourVal );
  Serial.print( ", currentMinute: " );
  Serial.print( minuteVal );
  Serial.print( ", currentSecond: " );
  Serial.println( secondVal );
#endif
  
  if( priorSecondVal == secondVal ) { 
      return;
  }   
  
  transition( hourVal, minuteVal, secondVal );
  
  priorMinuteVal = minuteVal;
  priorSecondVal = secondVal;

  // Every fifteen minutes do a little something obvious to indicate that
  // time has passed.
  if(( minuteVal == 0 || minuteVal == 15 || minuteVal == 30 || minuteVal == 45 ) && ( secondVal == 0 ) ) { 
    showFifteenMinuteIncrements();
  }
}

