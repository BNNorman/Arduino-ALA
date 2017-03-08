#ifndef ExtAlaLedRgb_h
#define ExtAlaLedRgb_h

#include "Ala.h"
#include "ExtNeoPixel.h"
#include "AlaApa102.h"

class AlaApa102;  // forward refernece needed

/*
*
* defines added to reduce code size when anims and palettes aren't required
* comment out the parts not required
*/

#define USES_ALA_ON
#define USES_ALA_OFF
#define USES_ALA_BLINK
#define USES_ALA_BLINKALT
#define USES_ALA_SPARKLE
#define USES_ALA_SPARKLE2
#define USES_ALA_STROBO
#define USES_ALA_CYCLECOLORS
#define USES_ALA_PIXELSHIFTRIGHT
#define USES_ALA_PIXELSHIFTLEFT
#define USES_ALA_PIXELBOUNCE
#define USES_ALA_PIXELSMOOTHSHIFTRIGHT
#define USES_ALA_PIXELSMOOTHSHIFTLEFT
#define USES_ALA_PIXELSMOOTHBOUNCE
#define USES_ALA_COMET
#define USES_ALA_COMETCOL
#define USES_ALA_MOVINGBARS
#define USES_ALA_MOVINGGRADIENT
#define USES_ALA_LARSONSCANNER
#define USES_ALA_LARSONSCANNER2
#define USES_ALA_FADEIN
#define USES_ALA_FADEOUT
#define USES_ALA_FADEINOUT
#define USES_ALA_GLOW
#define USES_ALA_PIXELSFADECOLORS
#define USES_ALA_FADECOLORS
#define USES_ALA_FADECOLORSLOOP
#define USES_ALA_FIRE
#define USES_ALA_BOUNCINGBALLS
#define USES_ALA_BUBBLES


// led strip section
// instead of using global values we store the animation info in an array
// note if Sections overlap the section with the higher array index
// will overwrite the effects of the lower index.
// when the ExtAlaLedRgb object is created a dummy section covering the entire strip is 
// created. That section can be redefined with a call to addSection(0,offset,numleds)

typedef struct Section{
  uint16_t offset;      // start of this section in the strip
  uint16_t numLeds;     // zero indicates section  not used
  int animation;        // current animation
  AlaPalette palette;   // current palette
  AlaSeq *animSeq;      // sequence list of animations
  long animSeqLen;      // number of animations. Zero indicates no animation sequence (of course)
  long speed;           // current animation speed
  long duration;        // of the current animation
  long currAnim;        // used if animSeq is set
  AlaColor maxOut;      // brightness for the section
  long nextRefreshTime; //
  long animStartTime;
  long animChangeTime;  // millis() at which to change animation to next in sequence (if there is one)
  // animation variables - used to be static but that doesn't work with sections
  float *pxPos;         // used by some animations
  float *pxSpeed;       // ditto
  long lastRefresh;     // used by bouncing balls animation
  byte *heat;           // used by fire animation
};


/**
 *  ExtAlaLedRgb can be used to drive a single or multiple RGB leds to perform animations.
 *  Available drivers are PWM pin, TLC5940, WS2811.
 */
class ExtAlaLedRgb
{

public:

	ExtAlaLedRgb(int NumSections);

	void initPWM(byte pinsRed, byte pinGreen, byte pinBlue);
	void initPWM(int numLeds, byte *pins);
	void initTLC5940(int numLeds, byte *pins);
  void initAPA102(int numLeds);
  void Checkpoint(int chk);
    /**
    * Initializes WS2812 LEDs. It be invoked in the setup() function of the main Arduino sketch.
    * 
    * The type field can be used to set the RGB order and chipset frequency. Constants are Adafruit_NeoPixel.h file.
    * It is set by default to NEO_GRB + NEO_KHZ800.
    */
	void initWS2812(int numLeds, byte pin, byte type=0x01+0x02);

	/**
	* Sets the maximum brightness level.
	*/
	void setBrightness(AlaColor maxOut);
  void setSectionBrightness(int idx,AlaColor maxOut);

	/**
	* Sets the maximum refresh rate in Hz (default value is 50 Hz).
	* May be useful to reduce flickering in some cases.
	*/
	void setRefreshRate(int refreshRate);
	
	int getRefreshRate();

  // ensures all potential sections have valid entries
  void initSections();
  
  // used to add/redefine a section of the strip
  // section[0] defaults to the entire strip but can be redefined
  void addSection(uint8_t idx,uint16_t offset,uint16_t numLeds);
  void setSectionAnimation(uint8_t idx,int animation, long speed,AlaColor color); 
  void setSectionAnimation(uint8_t idx,int animation, long speed,AlaPalette palette); 
  void setSectionAnimation(uint8_t idx,AlaSeq animSeq[]);
  bool runSectionAnimations();
  void initAnimation(int idx);

  // depracated - calls redirect to setSectionAnimation using section 0
	void setAnimation(int animation, long speed, AlaColor color);
	void setAnimation(int animation, long speed, AlaPalette palette);
	void setAnimation(AlaSeq animSeq[]);
	bool runAnimation();

 
private:

	void setAnimationFunc(int animation);
  void on(int idx);
  void off(int idx);
	void blink(int idx);
	void blinkAlt(int idx);
	void sparkle(int idx);
	void sparkle2(int idx);
	void strobo(int idx);
	void cycleColors(int idx);
	
	void pixelShiftRight(int idx);
  void pixelShiftLeft(int idx);
  void pixelBounce(int idx);
	void pixelSmoothShiftRight(int idx);
  void pixelSmoothShiftLeft(int idx);
  void comet(int idx);
  void cometCol(int idx);
	void pixelSmoothBounce(int idx);
	void larsonScanner(int idx);
	void larsonScanner2(int idx);

	void fadeIn(int idx);
  void fadeOut(int idx);
  void fadeInOut(int idx);
  void glow(int idx);
	void fadeColors(int idx);
	void pixelsFadeColors(int idx);
	void fadeColorsLoop(int idx);
	
	void movingBars(int idx);
	void movingGradient(int idx);
	
	void fire(int idx);
	void bouncingBalls(int idx);
	void bubbles(int idx);
	

	byte driver;    // type of led driver: ALA_PWM, ALA_TLC5940
	byte *pins;     // pins where the leds are attached to
	byte pin;       // WS2812 strip pin (used for debugging)

 // this is the entire strip
	AlaColor *leds; // array to store leds brightness values
	int numLeds;    // number of leds

  void (ExtAlaLedRgb::*animFunc)(int); // now passes in the section index
	
	int refreshMillis;            // global for all animations
	
	unsigned long animStartTime;    // not used
	unsigned long nextRefreshTime;  // now+anim duration, saves repeat calculation in the loop

  Adafruit_NeoPixel *neopixels;   // private to this strip (contains an array for pixels)

  int maxSections;
  Section *section; // individual strip sections

  AlaApa102 *Apa102Strip;
};


#endif
