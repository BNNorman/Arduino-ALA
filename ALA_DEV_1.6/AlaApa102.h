#ifndef ALA_APA102
#define ALA_APA102
#include <APA102.h>


#ifndef APA102_DATA_PIN
#define APA102_DATA_PIN 11
#warning "APA102_DATA_PIN constant not defined - using Pin11 (AlaApa102.h)"
#endif
#ifndef APA102_CLOCK_PIN
#define APA102_CLOCK_PIN 10
#warning "APA102_DATA_PIN constant not defined - using Pin11 (AlaApa102.h)"
#endif

class AlaApa102
{
  public:
  
  APA102 <APA102_DATA_PIN,APA102_CLOCK_PIN> ledStrip;

  void Checkpoint(int chk);
  
  AlaApa102();
  ~AlaApa102();
  void init(int numLeds);
  
  void show();
  void setBrightness(uint8_t b);
  void setPixelColor(uint16_t pix,uint8_t r, uint8_t g,uint8_t b);
  
private:

  int numLeds;
  uint8_t brightness; // actually only top 5 bits
  rgb_color *pixels;
  
};

#endif
