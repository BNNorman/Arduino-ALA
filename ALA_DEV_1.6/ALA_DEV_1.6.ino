/********************************************************************************************
 * Extended ALA DEMO sketch
 * 
 * ExtAlaLedRgb modified to include APA102 leds
 * 
 * Uses a modified version of the arduino ALA library to enable the sketch to drive nultiple 
 * WS2812 LED strips (well, I've tested with 2 matrix arrays of 40 and 64 leds). Previously
 * if you created two AlaLedRgb strips the animations for the second strip also ran on the 
 * first strip. (Well, that's what happened to me)
 * 
 * You can now subdivide a strip into sections and run different animations on each section.
 * 
 * You can control the max brightness of each section.
 * 
 * The above need ExtAlaLedRGB.cpp and .h in the same folder as your sketch.
 *
 * I have also included a header file defining all the standard web colours in colours.h. Put 
 * it into the sketch folder then you can specify colours like RGB_Azure. I have created a PDF
 * document which lists and shows all the colours. In your code just add RGB_ to the start of the name
 * 
 * WARNING: This demo sketch uses most of the 32K Flash avaiable in most Arduinos. It will exceed the 
 * available 2k SRAM of all except the big boys like the 2560. You have been warned!
 * 
 */
#define ALA_APA102  4     // driver
#define MAXSECTIONS 4

#define USES_APA_LED
#define APA102_CLOCK_PIN    10
#define APA102_DATA_PIN 11

#include "ExtfastGPIO.h"
#define APA102_USE_FAST_GPIO

#include "ExtAlaLedRgb.h"
#include "colours.h"
#include <MemoryFree.h>

// number of LEDS in each strip
#define STRIP1_LEDS  144
#define REFRESHRATE  100

ExtAlaLedRgb Strip1(MAXSECTIONS);

/*
 * Define the colour palettes to use. 
 * 
 * NOTE some animations, by default, only utilise the first entry in the palette. 
 * For example ALA_ON, ALA_BLINK,ALA_BLINKALT,ALA_STROBO,ALA_FADEIN, ALA_FADEOUT,
 * ALA_FADEINOUT and ALA_GLOW.
 *   
 */

static AlaColor STRIP1_COLORS[] =
{
    RGB_Magenta,RGB_LightBlue,RGB_Yellow,RGB_Green,RGB_Violet,RGB_Red
};

static AlaPalette STRIP1_PAL = { 6, STRIP1_COLORS };


static AlaColor FIRE_COLORS[] =
{
    RGB_Magenta,RGB_Red,RGB_Yellow,RGB_Orange,RGB_Violet
};

static AlaPalette FIRE_PAL = { 6, FIRE_COLORS };


// animations
// params are: animation,speed,duration,palette


static AlaSeq STRIP1_SEQ0[] =
{
  { ALA_SPARKLE,500, 5000, STRIP1_PAL},
    { ALA_COMET,500, 5000, STRIP1_PAL },
   { ALA_BLINK,500, 5000, STRIP1_PAL },
     {ALA_FIRE,500,5000,STRIP1_PAL},
   { ALA_LARSONSCANNER,500, 5000, STRIP1_PAL },
   { ALA_ENDSEQ }
};


static AlaSeq STRIP1_SEQ1[] =
{
  { ALA_LARSONSCANNER,500, 5000, STRIP1_PAL },
  { ALA_SPARKLE,500, 5000, STRIP1_PAL },
     { ALA_BLINK,500, 5000, STRIP1_PAL },
  { ALA_COMET,500, 5000, STRIP1_PAL },
  {ALA_FIRE,500,5000,STRIP1_PAL},
  { ALA_ENDSEQ }
};


void setup()
{
  Serial.begin(9600); // for debugging

  Serial.println("\nRESTART\n");
  
  // initialize the strip
  Strip1.initAPA102(STRIP1_LEDS);
  Strip1.setRefreshRate(REFRESHRATE);
  // using an 144 Led strip for testing
  // split it into 2 sections
  Strip1.addSection(0,0,72);    // first bank of leds
  Strip1.addSection(1,72,72);   // second bank of leds
  Strip1.setSectionAnimation(0,STRIP1_SEQ0);
  Strip1.setSectionAnimation(1,STRIP1_SEQ1);
  Strip1.setSectionBrightness(0,0x202020);  // values are RGB multipliers
  Strip1.setSectionBrightness(1,0x404040);
}


void loop()
{
  // run the animations
  Strip1.runAnimation();
  Serial.print("memory free=");
  Serial.println(freeMemory());
}

