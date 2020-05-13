#include "FastLED.h"

struct Wave{
  int center;
  int spread;
  int amplitude;
  uint8_t color;
  uint8_t brightness;
  Wave* next;
  Wave* previous;
};

class Waves{
  private:
    CRGB leds[NUM_LEDS];
    Wave* head;
    Wave* tail;
  public:
    Waves(int numLeds);
    ~Waves();
    bool isEmpty();
    void addWave();
    void removeLast();
    void removeAll();
    void setColors();
}
