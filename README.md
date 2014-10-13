ClockNeopixel60
===============

An Arduino (Trinket - ATTiny85 and Uno)  clock project that uses a 60 neopixel ring

This is the 60 pixel neopixel clock program written by Jason McHugh.  Feel free to drop me a line at mchughj@gmail.com for questions, comments, criticisms, concerns, etc.  :)

The backstory.

I originally bought a 16 pixel neopixel ring, a trinket, and an RTC module:

- Trinket:        https://www.adafruit.com/product/1501
- 16 pixel ring:  https://www.adafruit.com/product/1463
- RTC DS1307:     https://www.adafruit.com/product/264


I used the 16 pixel ring in another project for my son Aidan (who went camping with his 6th grade class, so I created a 2 way mirror with the ring behind the mirror).  I saw the 60 pixel ring and decided to get one (well, actually 4):

- 60 pixel ring:  https://www.adafruit.com/product/1768

Squeezing a nicely animating LED clock into the 5,310 byte maximum of the trinket wasn't easy.  Just using the TimeWireM and TinyRTClib libraries used up 3,146 bytes of space.  This left just ~ 2k of space to do something neat.  I'm sure if I were writing the assembly and really thinking hard about how to do this then it wouldn't be too challenging.  But writing 'regular' C code made it non-trivial.  

I could have moved to an Arduino and not had to worry about the space but I wanted to challenge myself a bit and fit it on the ATTiny85.  

I wrote the follow after observing clocks and thinking about what I thought the ideal animation/behavior would be.

I use this program in two separate scenarios.  

1/  The first is using an 8mhz Trinket with a physical RTC module attached to it.  This was my final intended configuration but it left very little space for debugging problems.  

In this first mode you have to have installed the TinyWireM and TinyRTClib installed. 

- https://github.com/adafruit/TinyWireM
- https://github.com/adafruit/TinyRTCLib


2/  The second scenario in which this program can be used is using a bare Arduino.  No RTC is required in this configuration - the millis implementation is used.  This mode allowed me to debug lots of problems with my original implementation so you will see lots of Serial debugging output and the ability to set a specific time.  In this second mode you have to have installed the RTClib.
  https://github.com/adafruit/RTClib


These two modes of operation are just the specific builds that I have.  There is nothing magical about them and additional configurations can be achieved by changing the code below.  

To set the two modes you have to do two things:

1/  In the Arduino programming environment select either 'Uno' or 'Adafruit Trinket 8mhz'.  Based on this setting everything else should be derived.   (Remember to set the programmer to be "USBTinyISP" if you are using the trinket!   If  you see lots of errors with "not in sync" then this can be why.)

2/  Uncomment or comment out the Wire.h, RTClib.h, TinyWireM, and TinyRTClib.  This shouldn't be necessary but apparently the Arduino programming environment uses a simple scan of all #includes in order to know what libraries might be used.  The preprocessor and directives are not used which makes this, IMO, a bug.  So the workaround is to comment and uncomment.  
