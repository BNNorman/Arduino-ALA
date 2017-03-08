/*
 * AlaApa102.cpp
 * 
 * driver for ALA102 LED strips
 * 
 * emulates a NeoPixel
 * 
 * Brightness is handled by the Ala code we just use the maximum value
 * for the strip
 * 
 * 
 */

#include "AlaApa102.h"
#include "Colours.h"
#include <APA102.h>



AlaApa102::AlaApa102()
{
// empty contructor
}

AlaApa102::~AlaApa102()
{
  // only occurs at switch off so nothing to do
}


void AlaApa102::init(int _numLeds)
{
  
  numLeds=_numLeds;
  brightness=31;  // only 5 bits
  
  pixels=new rgb_color[numLeds];

  if (!pixels)
    {
      Serial.println(numLeds);
      return;
    }
  for (int p=0;p<numLeds;p++)
    {
      pixels[p].red=0;
      pixels[p].green=0;
      pixels[p].blue=0;
    }
}

void AlaApa102::setBrightness(uint8_t b)
{
  // the APA102 only uses 5 bits to simulate
  // 0->FF shift the levels down so FF becomes 1F
  brightness=b>>3;
}

/*
 * simulate a NeoPixel strip
 */
void AlaApa102::setPixelColor(uint16_t pix,uint8_t r, uint8_t g,uint8_t b)
{ 
  
  if (!pixels | pix>numLeds)  {Serial.println("exit "); return;}

  pixels[pix].red=r;
  pixels[pix].green=g;
  pixels[pix].blue=b;

}

/*
 * Show() updates the Led strip from the pixels array
 */
void AlaApa102::show()
{
  ledStrip.write(pixels,144,brightness);
}

