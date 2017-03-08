
#include "Ala.h"
#include "ExtAlaLedRgb.h"

#include "ExtNeoPixel.h"
#include "ExtTlc5940.h"

#include "AlaApa102.h"
#include "Colours.h"

#define ALA_APA102 4  // should be in Ala.h

/* changes by Brian Norman January 2016
 *  
 *  Listen Up - you are probably going to need to use a 2560.
 *  The demo sketch drives two LED matrices of 64 and 40 Leds. It compiles to 26,784 bytes - which will just fir in a 32k Flash
 *  but the Global variables use 1,916 bytes which is just under the 2K limit of most Arduino.
 *  
 *  If you drive more LED strips the chances are you will definately need a 2560.
 *  
 *  I have renamed the AlaLedRgb.cpp and AlaLedRgb.h by prepending Ext (extended). These files should go into
 *  your Sketch folder
 *  
 *  LASTLY: I have only tested this code using WS2912 LED strips. It ought to work with the other supported strip types
 *  perhaps someone could check that out for me. (Thanks in advance)
 *  
 *  1 Now drives two or more WS2812 LED strips
 *  
 *  previously the last animation appeared on both strips. (See Demo sketch)
 *  
 *  2 Strips can be split into sections running separate animations
 *  
 *  You can define a number of section for a strip. This helps reduce wiring.
 *  
 *  Sections have index numbers 0..MAXSECTIONS as defined in ExtAlaledRgb.h
 *  
 *  3 You can specify seperate brightness levels for each section
 *  
 *  See demo sketch.
 *  
 *  4 You can remove unused animations by commenting out the USES_ definitions in ExtAlaledRgb.h
 *  
 *  This will reduce the Flash memory footprint - but not by a massive amount.
 *  
 */

ExtAlaLedRgb::ExtAlaLedRgb(int MaxSections)
{
	// set default values for animation (not many)
	refreshMillis = 1000/60;
  maxSections=MaxSections;
  initSections();
}


void ExtAlaLedRgb::initPWM(byte pinRed, byte pinGreen, byte pinBlue)
{
	byte *pins_ = (byte *)malloc(3);
    pins_[0] = pinRed;
    pins_[1] = pinGreen;
    pins_[2] = pinBlue;

    initSections();
    
    initPWM(1, pins_);
}

void ExtAlaLedRgb::initPWM(int numLeds, byte *pins)
{
	driver = ALA_PWM;
	this->numLeds = numLeds;
	this->pins = pins;

  initSections();
  
	for (int x=0; x<3*numLeds ; x++)
	{
		pinMode(pins[x], OUTPUT);
	}

	// allocate and clear leds array
	leds = (AlaColor *)malloc(3*numLeds);
	memset(leds, 0, 3*numLeds);
}

void ExtAlaLedRgb::initTLC5940(int numLeds, byte *pins)
{
	driver = ALA_TLC5940;
	this->numLeds = numLeds;
	this->pins = pins;

  initSections();

	// allocate and clear leds array
	leds = (AlaColor *)malloc(3*numLeds);
	memset(leds, 0, 3*numLeds);

	Tlc.init(0);
}

void ExtAlaLedRgb::initWS2812(int numLeds, byte pin, byte type)
{
	driver = ALA_WS2812;
	this->numLeds = numLeds;
  this->pin = pin; // used in debug messages
  
  initSections();
  
	// allocate and clear leds array
	leds = new AlaColor[numLeds]; //(AlaColor *)malloc(3*numLeds);
	//memset(leds, 0, 3*numLeds);
  for (int p=0;p<numLeds;p++) leds[p]=RGB_Black;
  
  neopixels=new Adafruit_NeoPixel(numLeds, pin, type);
	neopixels->begin();
}

void ExtAlaLedRgb::Checkpoint(int chk)
{
  Apa102Strip->Checkpoint(chk);
}

void ExtAlaLedRgb::initAPA102(int numLeds)
{
  driver=ALA_APA102;
  this->numLeds=numLeds;
  initSections();

  // allocate and clear leds array
  leds = new AlaColor[numLeds]; //(AlaColor *)malloc(3*numLeds);
  //memset(leds, 0, 3*numLeds);
  for(int i=0;i<numLeds;i++) leds[i]=RGB_Black;
  
  Apa102Strip=new AlaApa102();
  Apa102Strip->init(numLeds);
}

void ExtAlaLedRgb::setSectionBrightness(int idx,AlaColor maxOut)
{
  if ((idx>maxSections) or (idx<0)) return;
	section[idx].maxOut = maxOut;
}

void ExtAlaLedRgb::setBrightness(AlaColor maxOut)
{
  setSectionBrightness(0,maxOut);
}

void ExtAlaLedRgb::setRefreshRate(int refreshRate)
{
	this->refreshMillis = 1000/refreshRate;
}

int ExtAlaLedRgb::getRefreshRate()
{
	static long t;

	long el = millis() - t;
	t = millis();
	
	return 1000/el;
}


// initSections is used to ensure the section array
// contains valid entries the first is section
// specifying the whole strip - this can be reconfigured using addSection(0,offset,numLeds)

void ExtAlaLedRgb::initSections()
{
  if (!section)
    {
      section=new Section[maxSections];
    
      for (uint8_t i=0;i<maxSections;i++)
      {
        section[i].offset=0;
        if (i==0)
          {
            section[i].numLeds=numLeds;
          }
        else
          {
            section[i].numLeds=0;
          }
        section[i].animation=0;
        section[i].duration=0;
        section[i].maxOut=0xFFFFFF; // full on by default
        section[i].pxPos = NULL;
        section[i].pxSpeed = NULL;
        section[i].heat = NULL;
      }
  }
}


// addSection will overwrite existing sections that were setup by initSections()

void ExtAlaLedRgb::addSection(uint8_t idx,uint16_t offset,uint16_t numLeds)
{
  if ((idx>maxSections) or (idx<0)) return;
  // make sure the section fits the strip otherwise neopixels->setPixelColor
  // will crash the millis() clock for the Arduino
  
  if ((offset+numLeds)> this->numLeds) { numLeds=this->numLeds-offset;}

  section[idx].offset=offset;
  section[idx].numLeds=numLeds;
  section[idx].animation=ALA_OFF;
  section[idx].speed=0;
  section[idx].animSeqLen=0;
  section[idx].duration=0;
  
}

void ExtAlaLedRgb::setSectionAnimation(uint8_t idx,int animation, long speed,AlaColor color)
{
  if ((idx>maxSections) or (idx<0)) return;
  
  section[idx].animation=animation;
  section[idx].speed=speed;
  // need to create a palette
  section[idx].palette.numColors = 1;
  section[idx].palette.colors = (AlaColor*)malloc(3);
  section[idx].palette.colors[0] = color;
  section[idx].animSeqLen=0;

  initAnimation(idx);
  
}
void ExtAlaLedRgb::setSectionAnimation(uint8_t idx,int animation, long speed,AlaPalette palette)
{

  if ((idx>maxSections) or (idx<0)) return;
  
  section[idx].animation=animation;
  section[idx].speed=speed;
  section[idx].palette=palette;
  section[idx].animSeqLen=0;

  initAnimation(idx);
}

void ExtAlaLedRgb::setSectionAnimation(uint8_t idx,AlaSeq animSeq[])
{
  if ((idx>maxSections) or (idx<0)) return;

  section[idx].animSeq=animSeq;
  section[idx].currAnim=0;      // begin at the beginning
  section[idx].duration=animSeq[0].duration;
  section[idx].animation=animSeq[0].animation;
  section[idx].speed=animSeq[0].speed;
  section[idx].animChangeTime=millis()+animSeq[0].duration;
 
  // workout animSeqLen
  for(section[idx].animSeqLen=0; animSeq[section[idx].animSeqLen].animation!=ALA_ENDSEQ; section[idx].animSeqLen++)
    {
    // nothing to do here
    }

  initAnimation(idx);
}

void ExtAlaLedRgb::initAnimation(int idx)
{
  // delete any previously allocated array
  if (section[idx].pxPos!=NULL)
  { delete[] section[idx].pxPos; section[idx].pxPos=NULL; }
  if (section[idx].pxSpeed!=NULL)
  { delete[] section[idx].pxSpeed; section[idx].pxSpeed=NULL; }
  
  section[idx].animStartTime = millis();
  
  // this is a global refresh  
  section[idx].nextRefreshTime=section[idx].animStartTime+refreshMillis; // recalculated in runSectionAnimation loop
}

void ExtAlaLedRgb::setAnimation(int animation, long speed, AlaColor color)
{
  // default to idx 0
  setSectionAnimation(0,animation,speed,color);
  initAnimation(0);
}

void ExtAlaLedRgb::setAnimation(int animation, long speed, AlaPalette palette)
{
    // default to idx 0
  setSectionAnimation(0,animation,speed,palette);
  initAnimation(0);
}

void ExtAlaLedRgb::setAnimation(AlaSeq animSeq[])
{
  // default to idx 0
  setSectionAnimation(0,animSeq);
}


/*
 * runSectionAnimations
 * 
 * Steps through all sections which have numLeds>0
 * and sets up the animation for that section
 * 
 */
bool ExtAlaLedRgb::runSectionAnimations()
{
  bool animationDone=false;

  for (uint8_t idx=0;idx<maxSections;idx++)
    {
    unsigned long now=millis();
   
    if ((section[idx].numLeds==0) | (now < section[idx].nextRefreshTime)) 
      {
      //Serial.print("Skipping section ");
      //Serial.print(idx);
      //Serial.println(" animation.");
      //if (section[idx].numLeds==0) Serial.println("numLeds=0");
      //else Serial.println("not reached next resfresh time");
      continue;
      }

    section[idx].nextRefreshTime=now+refreshMillis;
 
    // setup the current section animation
    // no changes are needed if there's no animation sequence
    if (section[idx].animSeqLen!=0)
        { 
          // has current Animation finished?
          if (now>section[idx].animChangeTime)
              {
              section[idx].currAnim=(section[idx].currAnim+1)%section[idx].animSeqLen;
              section[idx].animChangeTime=now+section[idx].animSeq[section[idx].currAnim].duration;
              section[idx].lastRefresh=now; // used by some animations
              section[idx].animStartTime=now; // also used by some animations
              }
    
          // set the current section animation
          section[idx].animation=section[idx].animSeq[section[idx].currAnim].animation;
          section[idx].palette=section[idx].animSeq[section[idx].currAnim].palette;
          section[idx].speed=section[idx].animSeq[section[idx].currAnim].speed;
          //
        }

      // set the actual animation
  
      setAnimationFunc(section[idx].animation);

      // do the animation

      //if (animFunc==NULL) Serial.println("NULL animation function");
      
      if (animFunc != NULL)
      {
      animationDone=true;
            
      (this->*animFunc)(idx); // call the animation function for this section

        // refresh the LED strips
        
        if(driver==ALA_PWM)
          {
            for(int i=0; i<section[idx].numLeds; i++)
            {
              analogWrite(pins[3*i], leds[i].r*section[idx].maxOut.r/255);
              analogWrite(pins[3*i+1], leds[i].g*section[idx].maxOut.g/255);
              analogWrite(pins[3*i+2], leds[i].b*section[idx].maxOut.b/255);
            }
          }
        else if(driver==ALA_TLC5940)
          {
          for(int i=0; i<section[idx].numLeds; i++)
            {
              Tlc.set(pins[3*i], (leds[i].r*section[idx].maxOut.r/255)*16);
              Tlc.set(pins[3*i+1], (leds[i].g*section[idx].maxOut.g/255)*16);
              Tlc.set(pins[3*i+2], (leds[i].b*section[idx].maxOut.b/255)*16);
            }
          Tlc.update();
          }
        else if(driver==ALA_WS2812)
          {
            // probably not needed if we could write direct to neopixels in the animations
            // unfortunately one animation uses the raw led values for progressive
            // colour changes and since we write scaled values to NeoPixels we are stuck
            // with this method
            for(int i=0; i<section[idx].numLeds; i++)
             {
              uint32_t pix=i+section[idx].offset;

              uint8_t r=leds[pix].r*section[idx].maxOut.r/255;
              uint8_t g=leds[pix].g*section[idx].maxOut.g/255;
              uint8_t b=leds[pix].b*section[idx].maxOut.b/255;

              neopixels->setPixelColor(pix,r,g,b);
             }
            
          }
       else if (driver==ALA_APA102)
       {
           for(int i=0; i<section[idx].numLeds; i++)
             {
              uint32_t pix=i+section[idx].offset;

              //Serial.print("Section idx=");
              //Serial.print(idx);
              //Serial.print(" offset=");
              //Serial.println(section[idx].offset);
              

              uint8_t r=leds[pix].r*section[idx].maxOut.r/255;
              uint8_t g=leds[pix].g*section[idx].maxOut.g/255;
              uint8_t b=leds[pix].b*section[idx].maxOut.b/255;
              Apa102Strip->setPixelColor(pix,r,g,b);
             }
       }
       
      } // animFunc!=NULL
 
  } // end of sections loop

  // WS2812 only. we don't want to call neopixels->show() after every section has been updated only at the end
  // I'm not familiar with other LED strips so they need someone else to check
  if(driver==ALA_WS2812)
    {
    neopixels->show();
    }
    
  if(driver==ALA_APA102)
    {
         Apa102Strip->show();
    }

    // the return value indicates if ANY animation ran
    // in case the caller wants to know
    return animationDone;
}

bool ExtAlaLedRgb::runAnimation()
{
  return runSectionAnimations();
}

///////////////////////////////////////////////////////////////////////////////

void ExtAlaLedRgb::setAnimationFunc(int animation)
{

    switch(animation) 
	{
		#ifdef USES_ALA_ON
			case ALA_ON:                animFunc = &ExtAlaLedRgb::on;                    break;
		#endif
		#ifdef USES_ALA_OFF
			case ALA_OFF:				        animFunc = &ExtAlaLedRgb::off;                   break;
		#endif
		#ifdef USES_ALA_BLINK
			case ALA_BLINK:				      animFunc = &ExtAlaLedRgb::blink;                  break;
		#endif
		#ifdef USES_ALA_BLINKALT
			case ALA_BLINKALT:          animFunc = &ExtAlaLedRgb::blinkAlt;               break;
		#endif
		#ifdef USES_ALA_SPARKLE
			case ALA_SPARKLE:           animFunc = &ExtAlaLedRgb::sparkle;                 break;
		#endif
		#ifdef USES_ALA_SPARKLE2
			case ALA_SPARKLE2:          animFunc = &ExtAlaLedRgb::sparkle2;                 break;
		#endif
		#ifdef USES_ALA_STROBO
			case ALA_STROBO:            animFunc = &ExtAlaLedRgb::strobo;                   break;
		#endif
		#ifdef USES_ALA_CYCLECOLORS
			case ALA_CYCLECOLORS:       animFunc = &ExtAlaLedRgb::cycleColors;               break;
		#endif
		#ifdef USES_ALA_PIXELSHIFTRIGHT 
			case ALA_PIXELSHIFTRIGHT:   animFunc = &ExtAlaLedRgb::pixelShiftRight;           break;
		#endif
		#ifdef USES_ALA_PIXELSHIFTLEFT
			case ALA_PIXELSHIFTLEFT:    animFunc = &ExtAlaLedRgb::pixelShiftLeft;        break;
		#endif
		#ifdef USES_ALA_PIXELBOUNCE
			case ALA_PIXELBOUNCE:       animFunc = &ExtAlaLedRgb::pixelBounce;           break;
		#endif
		#ifdef USES_ALA_PIXELSMOOTHSHIFTRIGHT
			case ALA_PIXELSMOOTHSHIFTRIGHT: 
			                            animFunc = &ExtAlaLedRgb::pixelSmoothShiftRight; break;
		#endif
		#ifdef USES_ALA_PIXELSMOOTHSHIFTLEFT
			case ALA_PIXELSMOOTHSHIFTLEFT:  
			                            animFunc = &ExtAlaLedRgb::pixelSmoothShiftLeft;  break;
		#endif
		#ifdef USES_ALA_PIXELSMOOTHBOUNCE
			case ALA_PIXELSMOOTHBOUNCE: animFunc = &ExtAlaLedRgb::pixelSmoothBounce;     break;
		#endif
		#ifdef USES_ALA_COMET
			case ALA_COMET:             animFunc = &ExtAlaLedRgb::comet;                 break;
		#endif
		#ifdef USES_ALA_COMETCOL
			case ALA_COMETCOL:          animFunc = &ExtAlaLedRgb::cometCol;              break;
		#endif
		#ifdef USES_ALA_MOVINGBARS
			case ALA_MOVINGBARS:        animFunc = &ExtAlaLedRgb::movingBars;            break;
		#endif
		#ifdef USES_ALA_MOVINGGRADIENT
			case ALA_MOVINGGRADIENT:    animFunc = &ExtAlaLedRgb::movingGradient;        break;
		#endif
		#ifdef USES_ALA_LARSONSCANNER
			case ALA_LARSONSCANNER:     animFunc = &ExtAlaLedRgb::larsonScanner;         break;
		#endif
		#ifdef USES_ALA_LARSONSCANNER2
			case ALA_LARSONSCANNER2:    animFunc = &ExtAlaLedRgb::larsonScanner2;        break;
		#endif
		#ifdef USES_ALA_FADEIN
			case ALA_FADEIN:            animFunc = &ExtAlaLedRgb::fadeIn;                break;
		#endif
		#ifdef USES_ALA_FADEOUT
			case ALA_FADEOUT:           animFunc = &ExtAlaLedRgb::fadeOut;               break;
		#endif
		#ifdef USES_ALA_FADEINOUT
			case ALA_FADEINOUT:         animFunc = &ExtAlaLedRgb::fadeInOut;             break;
		#endif
		#ifdef USES_ALA_GLOW
			case ALA_GLOW:              animFunc = &ExtAlaLedRgb::glow;                  break;
		#endif
		#ifdef USES_ALA_PIXELSFADECOLORS
			case ALA_PIXELSFADECOLORS:  animFunc = &ExtAlaLedRgb::pixelsFadeColors;      break;
		#endif
		#ifdef USES_ALA_FADECOLORS
			case ALA_FADECOLORS:        animFunc = &ExtAlaLedRgb::fadeColors;            break;
		#endif
		#ifdef USES_ALA_FADECOLORSLOOP
			case ALA_FADECOLORSLOOP:    animFunc = &ExtAlaLedRgb::fadeColorsLoop;        break;
		#endif
		#ifdef USES_ALA_FIRE
			case ALA_FIRE:              animFunc = &ExtAlaLedRgb::fire;                  break;
		#endif
		#ifdef USES_ALA_BOUNCINGBALLS
			case ALA_BOUNCINGBALLS:     animFunc = &ExtAlaLedRgb::bouncingBalls;         break;
		#endif
		#ifdef USES_ALA_BUBBLES
			case ALA_BUBBLES:           animFunc = &ExtAlaLedRgb::bubbles;               break;
		#endif
    #ifdef DEBUG
		  default:                    animFunc = &ExtAlaLedRgb::on;
	  #else
      default:                    animFunc = &ExtAlaLedRgb::off;
	  #endif
	}
}


#ifdef USES_ALA_ON
void ExtAlaLedRgb::on(int idx)
{
  AlaPalette pal=section[idx].palette;
  AlaColor  c=pal.colors[0];
   
	for(int i=0; i<section[idx].numLeds; i++)
	{
		leds[i+section[idx].offset] = c;
	}
}
#endif

// always available - see default
void ExtAlaLedRgb::off(int idx)
{
	for(int i=0; i<section[idx].numLeds; i++)
	{
		leds[i+section[idx].offset] = 0x000000;
	}
}


#ifdef USES_ALA_BLINK
void ExtAlaLedRgb::blink(int idx)
{
	int t = getStep(animStartTime, section[idx].speed, 2);
	int k = (t+1)%2;
  AlaPalette pal=section[idx].palette;
    
	for(int x=0; x<section[idx].numLeds; x++)
	{
    leds[x+section[idx].offset] = pal.colors[0].scale(k);
	}
}
#endif

#ifdef USES_ALA_BLINKALT
void ExtAlaLedRgb::blinkAlt(int idx)
{
	int t = getStep(section[idx].animStartTime, section[idx].speed, 2);

  AlaPalette pal=section[idx].palette;
  
	for(int x=0; x<section[idx].numLeds; x++)
	{
		int k = (t+x)%2;
    leds[x+section[idx].offset] = pal.colors[0].scale(k);
	}
}
#endif

#ifdef USES_ALA_SPARKLE
void ExtAlaLedRgb::sparkle(int idx)
{

  AlaPalette pal=section[idx].palette;
    
  int p = section[idx].speed/100;
	for(int x=0; x<section[idx].numLeds; x++)
    {
		leds[x+section[idx].offset] = pal.colors[random(pal.numColors)].scale(random(p)==0);
    }
}
#endif

#ifdef USES_ALA_SPARKLE2

void ExtAlaLedRgb::sparkle2(int idx)
{
  int p = section[idx].speed/10;
  AlaPalette pal=section[idx].palette;
      
	for(int x=0; x<section[idx].numLeds; x++)
    {
		if(random(p)==0)
			leds[x+section[idx].offset] = pal.colors[random(pal.numColors)];
		else
			leds[x+section[idx].offset] = leds[x+section[idx].offset].scale(0.88);
    }
}

#endif

#ifdef USES_ALA_STROBO
void ExtAlaLedRgb::strobo(int idx)
{
	int t = getStep(section[idx].animStartTime, section[idx].speed, ALA_STROBODC);
	AlaPalette pal=section[idx].palette;
  AlaColor c = pal.colors[0].scale(t==0);
	
	for(int x=0; x<section[idx].numLeds; x++)
    {
		leds[x+section[idx].offset] = c;
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////
// Shifting effects
////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USES_ALA_PIXELSHIFTRIGHT
void ExtAlaLedRgb::pixelShiftRight(int idx)
{
  AlaPalette pal=section[idx].palette;
	int t = getStep(section[idx].animStartTime, section[idx].speed, section[idx].numLeds);
	float tx = getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors);
	AlaColor c = pal.getPalColor(tx);
	
	for(int x=0; x<section[idx].numLeds; x++)
	{
		int k = (x==t ? 1:0);
		leds[x+section[idx].offset] = c.scale(k);
	}
}
#endif

#ifdef USES_ALA_PIXELSHIFTLEFT
void ExtAlaLedRgb::pixelShiftLeft(int idx)
{
  AlaPalette pal=section[idx].palette;
	int t = getStep(section[idx].animStartTime, section[idx].speed, section[idx].numLeds);
	float tx = getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors);
	AlaColor c = pal.getPalColor(tx);
	
	for(int x=0; x<section[idx].numLeds; x++)
	{
		int k = ((x==(section[idx].numLeds-1-t) ? 1:0));
		leds[x+section[idx].offset] = c.scale(k);
	}
}
#endif

#ifdef USES_ALA_PIXELBOUNCE

// Bounce back and forth
void ExtAlaLedRgb::pixelBounce(int idx)
{
  AlaPalette pal=section[idx].palette;
	int t = getStep(section[idx].animStartTime, section[idx].speed, 2*section[idx].numLeds-2);
	float tx = getStepFloat(animStartTime, section[idx].speed, pal.numColors);
	AlaColor c = pal.getPalColor(tx);

	for(int x=0; x<section[idx].numLeds; x++)
	{
		int k = x==(-abs(t-section[idx].numLeds+1)+section[idx].numLeds-1) ? 1:0;
		leds[x+section[idx].offset] = c.scale(k);
	}
}
#endif

#ifdef USES_ALA_PIXELSMOOTHSHIFTRIGHT

void ExtAlaLedRgb::pixelSmoothShiftRight(int idx)
{
  AlaPalette pal=section[idx].palette;
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, section[idx].numLeds+1);
	float tx = getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors);

	AlaColor c = pal.getPalColor(tx);
	
	for(int x=0; x<section[idx].numLeds; x++)
	{
		float k = max(0, (-abs(t-1-x)+1));
		leds[x+section[idx].offset] = c.scale(k);
	}
}

#endif

#ifdef USES_ALA_PIXELSMOOTHSHIFTLEFT
void ExtAlaLedRgb::pixelSmoothShiftLeft(int idx)
{
  AlaPalette pal=section[idx].palette;
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, section[idx].numLeds+1);
	float tx = getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors);
	AlaColor c = pal.getPalColor(tx);
	
	for(int x=0; x<section[idx].numLeds; x++)
	{
		float k = max(0, (-abs(section[idx].numLeds-t-x)+1));
		leds[x+section[idx].offset] = c.scale(k);
	}
}
#endif

#ifdef USES_ALA_COMET
void ExtAlaLedRgb::comet(int idx)
{
  AlaPalette pal=section[idx].palette;
	float l = section[idx].numLeds/2;  // length of the tail
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, 2*section[idx].numLeds-l);
	float tx = getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors);
  
	AlaColor c = pal.getPalColor(tx);
	
	for(int x=0; x<section[idx].numLeds; x++)
	{
		float k = constrain( (((x-t)/l+1.2f))*(((x-t)<0)? 1:0), 0, 1);
		leds[x+section[idx].offset] = c.scale(k);
	}
}
#endif

#ifdef USES_ALA_COMETCOL
void ExtAlaLedRgb::cometCol(int idx)
{
	float l = section[idx].numLeds/2;  // length of the tail
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, 2*section[idx].numLeds-l);

  AlaPalette pal=section[idx].palette;
	for(int x=0; x<section[idx].numLeds; x++)
	{
		float tx = mapfloat(max(t-x, 0), 0, section[idx].numLeds/1.7, 0, pal.numColors-1);
	  AlaColor c = pal.getPalColor(tx);
		float k = constrain( (((x-t)/l+1.2f))*(((x-t)<0)? 1:0), 0, 1);
		leds[x+section[idx].offset] = c.scale(k);
	}
}
#endif

#ifdef USES_ALA_PIXELSMOOTHBOUNCE
void ExtAlaLedRgb::pixelSmoothBounce(int idx)
{
	// see larsonScanner
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, 2*section[idx].numLeds-2);
  AlaPalette pal=section[idx].palette;
	AlaColor c = pal.getPalColor(getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors));

	for(int x=0; x<section[idx].numLeds; x++)
	{
		float k = constrain((-abs(abs(t-section[idx].numLeds+1)-x)+1), 0, 1);
		leds[x+section[idx].offset] = c.scale(k);
	}
}
#endif

#ifdef USES_ALA_LARSONSCANNER
void ExtAlaLedRgb::larsonScanner(int idx)
{
	float l = section[idx].numLeds/4;
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, 2*section[idx].numLeds-2);
  AlaPalette pal=section[idx].palette;
	AlaColor c = pal.getPalColor(getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors));

	for(int x=0; x<section[idx].numLeds; x++)
	{
		float k = constrain((-abs(abs(t-section[idx].numLeds+1)-x)+l), 0, 1);
		leds[x+section[idx].offset] = c.scale(k);
	}
}
#endif

#ifdef USES_ALA_LARSONSCANNER2
void ExtAlaLedRgb::larsonScanner2(int idx)
{
	float l = section[idx].numLeds/4;  // 2>7, 3-11, 4-14
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, 2*section[idx].numLeds+(l*4-1));
  AlaPalette pal=section[idx].palette;
	AlaColor c = pal.getPalColor(getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors));

	for(int x=0; x<section[idx].numLeds; x++)
	{
		float k = constrain((-abs(abs(t-section[idx].numLeds-2*l)-x-l)+l), 0, 1);
		leds[x+section[idx].offset] = c.scale(k);
	}
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////
// Fading effects
////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USES_ALA_FADEIN
void ExtAlaLedRgb::fadeIn(int idx)
{
	float s = getStepFloat(section[idx].animStartTime, section[idx].speed, 1);
  AlaPalette pal=section[idx].palette;
  AlaColor c = pal.colors[0].scale(s);
	
	for(int x=0; x<section[idx].numLeds; x++)
	{
		leds[x+section[idx].offset] = c;
	}
}
#endif


#ifdef USES_ALA_FADEOUT
void ExtAlaLedRgb::fadeOut(int idx)
{

	float s = getStepFloat(section[idx].animStartTime, section[idx].speed, 1);
  AlaPalette pal=section[idx].palette;
  AlaColor c = pal.colors[0].scale(1-s);
	
	for(int x=0; x<section[idx].numLeds; x++)
	{
		leds[x+section[idx].offset] = c;
	}
}
#endif

#ifdef USES_ALA_FADEINOUT
void ExtAlaLedRgb::fadeInOut(int idx)
{
	float s = getStepFloat(section[idx].animStartTime, section[idx].speed, 2) - 1;
  AlaPalette pal=section[idx].palette;
  AlaColor c = pal.colors[0].scale(abs(1-abs(s)));;

	for(int x=0; x<section[idx].numLeds; x++)
	{
		leds[x+section[idx].offset] = c;
	}
}
#endif

#ifdef USES_ALA_GLOW
void ExtAlaLedRgb::glow(int idx)
{
	float s = getStepFloat(section[idx].animStartTime, section[idx].speed, TWO_PI);
	float k = (-cos(s)+1)/2;

  AlaPalette pal=section[idx].palette;
  AlaColor c = pal.colors[0].scale(k);
   
	for(int x=0; x<section[idx].numLeds; x++)
	{
		leds[x+section[idx].offset] = c;
	}
}
#endif

#ifdef USES_ALA_FADECOLORS
void ExtAlaLedRgb::fadeColors(int idx)
{
  AlaPalette pal=section[idx].palette;
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors-1);

	AlaColor c = pal.getPalColor(t);
	for(int x=0; x<section[idx].numLeds; x++)
	{
		leds[x+section[idx].offset] = c;
	}
	
}
#endif

#ifdef USES_ALA_PIXELSFADECOLORS
void ExtAlaLedRgb::pixelsFadeColors(int idx)
{
  AlaPalette pal=section[idx].palette;
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors);
  
	for(int x=0; x<section[idx].numLeds; x++)
	{
		AlaColor c = pal.getPalColor(t+7*x);
		leds[x+section[idx].offset] = c;
	}
}
#endif

#ifdef USES_ALA_FADECOLORSLOOP

void ExtAlaLedRgb::fadeColorsLoop(int idx)
{
  AlaPalette pal=section[idx].palette;
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, pal.numColors);
  AlaColor c = pal.getPalColor(t);
	for(int x=0; x<section[idx].numLeds; x++)
	{
		leds[x+section[idx].offset] = c;
	}
}
#endif

#ifdef USES_ALA_CYCLECOLORS
void ExtAlaLedRgb::cycleColors(int idx)
{
  AlaPalette pal=section[idx].palette;
	int t = getStep(section[idx].animStartTime, section[idx].speed, pal.numColors);

  
	for(int x=0; x<section[idx].numLeds; x++)
	{
		leds[x+section[idx].offset] = pal.colors[t];
	}
}
#endif

#ifdef USES_ALA_MOVINGBARS
void ExtAlaLedRgb::movingBars(int idx)
{
	int t = getStep(section[idx].animStartTime, section[idx].speed, section[idx].numLeds);

  AlaPalette pal=section[idx].palette;
  
	for(int x=0; x<section[idx].numLeds; x++)
	{
		leds[x+section[idx].offset] = pal.colors[(((t+x)*pal.numColors)/section[idx].numLeds)%pal.numColors];
	}
}
#endif

#ifdef USES_ALA_MOVINGGRADIENT
void ExtAlaLedRgb::movingGradient(int idx)
{
	float t = getStepFloat(section[idx].animStartTime, section[idx].speed, section[idx].numLeds);

  AlaPalette pal=section[idx].palette;
  
	for(int x=0; x<section[idx].numLeds; x++)
	{
		leds[x+section[idx].offset] = pal.getPalColor((float)((x+t)*pal.numColors)/section[idx].numLeds); // interpolates colours
	}
}
#endif

#ifdef USES_ALA_FIRE
void ExtAlaLedRgb::fire(int idx)
{
    AlaPalette pal=section[idx].palette;
        
    // COOLING: How much does the air cool as it rises?
    // Less cooling = taller flames.  More cooling = shorter flames.
    // Default 550
    #define COOLING  550

    // SPARKING: What chance (out of 255) is there that a new spark will be lit?
    // Higher chance = more roaring fire.  Lower chance = more flickery fire.
    // Default 120, suggested range 50-200.
    #define SPARKING 120

    // Array of temperature readings at each simulation cell
    if (section[idx].heat==NULL)
        section[idx].heat = new byte[section[idx].numLeds];

    // Step 1.  Cool down every cell a little
    for( int i = 0; i < section[idx].numLeds; i++)
	{
      section[idx].heat[i] = max((int)section[idx].heat[i] - random(0, (COOLING / section[idx].numLeds) + 2), 0);
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= numLeds - 1; k >= 2; k--)
	{
      section[idx].heat[k] = (section[idx].heat[k - 1] + section[idx].heat[k - 2] + section[idx].heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random(255) < SPARKING )
	{
      int y = random(7);
      section[idx].heat[y] = min((int)section[idx].heat[y] + random(160,255), 255);
    }

    // Step 4.  Map from heat cells to LED colors
    for(int j=0; j<section[idx].numLeds; j++)
	{
      float colorindex = (float)(section[idx].heat[j] * (pal.numColors-1) ) / 255;

      leds[j+section[idx].offset] = pal.getPalColor(colorindex);  // getPalColor interpolates between two colors
    }
}
#endif

#ifdef USES_ALA_BOUNCINGBALLS
void ExtAlaLedRgb::bouncingBalls(int idx)
{
	//static long lastRefresh;
  AlaPalette pal=section[idx].palette;

	if (section[idx].pxPos==NULL)
	{
		// allocate new arrays
		section[idx].pxPos = new float[pal.numColors];
		section[idx].pxSpeed = new float[pal.numColors];

		for (int i=0; i<pal.numColors; i++)
		{
			section[idx].pxPos[i] = ((float)random(255))/255;
			section[idx].pxSpeed[i] = 0;
		}
		section[idx].lastRefresh = millis();
		
		return; // skip the first cycle
	}
	
	float speedReduction = (float)(millis() - section[idx].lastRefresh)/5000;
	section[idx].lastRefresh = millis();
 
	for (int i=0; i<pal.numColors; i++)
	{
		if(section[idx].pxSpeed[i]>-0.04 and section[idx].pxSpeed[i]<0 and section[idx].pxPos[i]>0 and section[idx].pxPos[i]<0.1)
			section[idx].pxSpeed[i]=(0.09)-((float)random(10)/1000);
		
		section[idx].pxPos[i] = section[idx].pxPos[i] + section[idx].pxSpeed[i];
		if(section[idx].pxPos[i]>=1)
		{
			section[idx].pxPos[i]=1;
		}
		if(section[idx].pxPos[i]<0)
		{
			section[idx].pxPos[i]=-section[idx].pxPos[i];
			section[idx].pxSpeed[i]=-0.91*section[idx].pxSpeed[i];
		}
		
		section[idx].pxSpeed[i] = section[idx].pxSpeed[i]-speedReduction;
	}

	for (int x=0; x<section[idx].numLeds ; x++)
	{
		leds[x] = 0;
	}
     
	for (int i=0; i<pal.numColors; i++)
	{
		int p = mapfloat(section[idx].pxPos[i], 0, 1, 0, section[idx].numLeds-1);
		leds[p+section[idx].offset] = leds[p+section[idx].offset].sum(pal.colors[i]);
	}

}
#endif

#ifdef USES_ALA_BUBBLES
void ExtAlaLedRgb::bubbles(int idx)
{
	//static long lastRefresh; moved to Section

  AlaPalette pal=section[idx].palette;
  
	if (section[idx].pxPos==NULL)
	{
		// allocate new arrays
		section[idx].pxPos = new float[pal.numColors];
		section[idx].pxSpeed = new float[pal.numColors];

		for (int i=0; i<pal.numColors; i++)
		{
			section[idx].pxPos[i] = ((float)random(255))/255;
			section[idx].pxSpeed[i] = 0;
		}
		section[idx].lastRefresh = millis();
		return; // skip the first cycle
	}
	
	float speedDelta = (float)(millis() - section[idx].lastRefresh)/80000;
	section[idx].lastRefresh = millis();
 
	for (int i=0; i<pal.numColors; i++)
	{
		if(section[idx].pxPos[i]>=1)
		{
			section[idx].pxPos[i]=0;
			section[idx].pxSpeed[i]=0;
		}
		if(random(20)==0 and section[idx].pxPos[i]==0)
		{
			section[idx].pxPos[i]=0.0001;
			section[idx].pxSpeed[i]=0.0001;
		}
		if(section[idx].pxPos[i]>0)
		{
			section[idx].pxPos[i] = section[idx].pxPos[i] + section[idx].pxSpeed[i];
			section[idx].pxSpeed[i] = section[idx].pxSpeed[i] + speedDelta;
		}		
	}

	for (int x=0; x<section[idx].numLeds ; x++)
	{
		leds[x+section[idx].offset] = 0;
	}


	for (int i=0; i<pal.numColors; i++)
	{
		if (section[idx].pxPos[i]>0)
		{
			int p = mapfloat(section[idx].pxPos[i], 0, 1, 0, section[idx].numLeds-1);
      
			AlaColor c = pal.colors[i].scale(1-(float)random(10)/30); // add a little flickering
			leds[p+section[idx].offset] = c;
		}
	}

}
#endif
