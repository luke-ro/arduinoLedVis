#include "arduinoFFT.h"
#include "FastLED.h"
//#inlcude "waves.h"
#include <SPI.h>
//#include <cmath>


#define BUTTON_PIN 7
bool buttonState;
//variables for LEDs
#define NUM_LEDS 200
#define DATA_PIN 6

CRGB leds[NUM_LEDS];

int normCoef = 1;
int maxH = 50;

//variables for FFT
#define SAMPLES 16             //Must be a power of 2
#define SAMPLING_FREQUENCY 2000 //Hz, must be less than 10000 due to ADC

arduinoFFT FFT = arduinoFFT();

unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[SAMPLES];
double vImag[SAMPLES];

//functions
void runFFT();
void visualize();
double avg(double* nums, int start, int last);
double findMax(double* nums, int start, int last);
bool hasChanged();

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN,INPUT);
  ADCSRA = 0b11100101;      // set ADC to free running mode and set pre-scalar to 32 (0xe5)
  ADMUX = 0b00000000;       // use pin A0 and external voltage reference

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(100);

  sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));

  for(int i=0; i<3;i++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }
  Serial.println("begin");
}

void loop() {
  visualize_1();
  visualize_2();
  visualize_3();
  visualize_4();

}

void runFFT(){

  // ++ Sampling
  for(int i=0; i<SAMPLES; i++){
    microseconds = micros();

    while(!(ADCSRA & 0x10));        // wait for ADC to complete current conversion ie ADIF bit set
    ADCSRA = 0b11110101 ;               // clear ADIF bit so that ADC can do next operation (0xf5)
    int value = ADC - 512 ;                 // Read from ADC and subtract DC offset caused value
    vReal[i]= value/8;                      // Copy to bins after compressing
    vImag[i] = 0;

    while(micros() < (microseconds + sampling_period_us));
  }
   // -- Sampling

    /*FFT*/
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
//  for(int i=0; i<SAMPLES/2; i++){
//    vReal[i] = constrain(vReal[i],0,100);            // set max & min values for buckets
//    vReal[i] = map(vReal[i], 0, 100, 0, maxH);
//  }
}

//Profile for controlling the LED strip.
//Reacts to audio input.
void visualize_1(){
  unsigned long start = millis();
  int counter=0;

  double absoluteMax = 0;
  int bassMax=1;
  int bassTemp;
  uint8_t whiteOut = 1; //if the bass maxes out, the background will be "whited out" and will decay
  uint8_t hue=0;

  while(!pressed()){
    bassMax-=2;//decrement the bass max (causes a decay effect)
    hue++;    //cycles through all the colors
    counter++;
    if(whiteOut>10) whiteOut-=10;//decrement the "whiteOut" effect only (if condition is there so it doesn't loop infinitely)

    runFFT();
    FastLED.clear();

    //bass visualization
    bassTemp = findMax(vReal,0,SAMPLES/8); //largest number in the first quarter of vReal[]
//    bassTemp = vReal[1];//initial value to determine how many leds to light up in relation to the bass
    if(bassTemp>NUM_LEDS/4){
      bassTemp = NUM_LEDS/4;
      whiteOut = 255;
    }

    if(absoluteMax<bassTemp) absoluteMax = bassTemp;
    bassTemp = map(bassTemp,0,absoluteMax*1.1,0,NUM_LEDS/2);
    if(bassTemp>bassMax) bassMax = bassTemp;

    //white out background
    for(int i=0; i<NUM_LEDS && whiteOut>0; i++){
      leds[i] = CHSV(0,0,whiteOut);
    }

    //set color values
    for(int i=0; i<bassMax && bassMax<NUM_LEDS/2; i++){
      if(bassMax<NUM_LEDS/8*3){
        leds[i] = CHSV(hue,255,255);
        leds[NUM_LEDS-1-i] = CHSV(hue,255,255);
      }else{
        leds[i] = CHSV(hue,(-255*4/NUM_LEDS)*((bassMax)-(NUM_LEDS/2)),255);
        leds[NUM_LEDS-1-i] = CHSV(hue,(-255*4/NUM_LEDS)*((bassMax)-(NUM_LEDS/2)),255);
      }
    }
    FastLED.show();
  }
  //for testing speed
  unsigned long endTime = millis();
  double loopsPerMillis = double(counter)/(endTime-start);
  Serial.print("visulaize_1() loops per millisecond: ");
  Serial.println(loopsPerMillis,5);
}

//similar to visualize_1() except a treble visualization is added
void visualize_2(){
  unsigned long start = millis();
  int counter=0;

  int bassMax=1,trebMax=1;
  int bassTemp,trebTemp;
  uint8_t whiteOut = 1; //if the bass maxes out, the background will be "whited out" and will decay
  uint8_t hue=0;

  while(!pressed()){
    bassMax--;
    trebMax--;
    counter++;
    hue++;
    if(whiteOut>10) whiteOut-=10;

    runFFT();

    FastLED.clear();

    //bass visualization
//    bassTemp = findMax(vReal,0,SAMPLES/8); //largest number in the first quarter of vReal[]
    bassTemp = int(vReal[0]);
    if(bassTemp>NUM_LEDS/4){
      bassTemp = NUM_LEDS/4;
      whiteOut = 255;
    }
    if(bassTemp>bassMax) bassMax = bassTemp;

    //white out background
    for(int i=0; i<NUM_LEDS && whiteOut>0; i++){
      leds[i] = CHSV(0,0,whiteOut);
    }

    for(int i=0; i<bassMax*normCoef*2 && bassMax*normCoef*2<NUM_LEDS/2; i++){
      if(bassMax*normCoef*2<NUM_LEDS/8*3){
        leds[i] = CHSV(hue,255,255);
        leds[NUM_LEDS-1-i] = CHSV(hue,255,255);
      }else{
        leds[i] = CHSV(hue,(-255*4/NUM_LEDS)*((bassMax*normCoef*2)-(NUM_LEDS/2)),255);
        leds[NUM_LEDS-1-i] = CHSV(hue,(-255*4/NUM_LEDS)*((bassMax*normCoef*2)-(NUM_LEDS/2)),255);
      }
    }


        //treble visualization
        trebTemp = avg(vReal,SAMPLES/4-1,SAMPLES/4);
//        trebTemp = vReal[SAMPLES/8-1];
        if(trebTemp>NUM_LEDS/4) trebTemp = NUM_LEDS/4;
        if(trebTemp>trebMax) trebMax = trebTemp;

        for(int i=0; i<trebMax*normCoef && trebMax*normCoef<NUM_LEDS/2; i++){
          leds[NUM_LEDS/2+i] = CHSV(hue-120,255,255);
          leds[NUM_LEDS/2-i] = CHSV(hue-120,255,255);
        }

    //for(int i=0;i<NUM_LEDS; i++)leds[i] = CHSV(hue,255,255);
    FastLED.show();
  }
  //for testing speed
  unsigned long endTime = millis();
  double loopsPerMillis = double(counter)/(endTime-start);
  Serial.print("visulaize_2() loops per millisecond: ");
  Serial.println(loopsPerMillis,5);
}

//void visualize_chill(){
//  int bass;
//  int bassMax = 0;
//  uint8_t counter = 0;
//  struct wave waves[10];
//  struct wave *ptr;
//  while(!pressed()){
//    runFFT();
//    FastLED.clear();
//    bass = findMax(vReal,0,SAMPLES/8); //largest number in the first quarter of vReal[]
//    if(bass>bassMax) bassMax = bass;
//
//    //make a new/overwrite wave
//    if(bass>bassMax*0.9){
//      ptr = &waves[counter];
//      ptr->center = map(random8(),0,255,0,NUM_LEDS);
//      ptr->spread = 0;
//      ptr->amplitude = 5;
//      ptr->color = random8();
//      ptr->brightness = 255;
//      counter++;
//      counter %= 10;
//    }
//    //iterate through waves
//    for(int i=0; i<counter; i++){
//      ptr = &waves[i];
//      ptr->spread++;
//      ptr->amplitude = 5;
//      ptr->color = random8();
//      ptr->brightness-=10;
//    }
//    //implement linked list for this^^^^^^
//
//  }
//}

//breaks up the LED strip into as many sections as there are frequency buckets
void visualize_3(){
  uint8_t hue = random8();
  while(!pressed()){
    FastLED.clear();
    runFFT();
    for(int i=0; i<SAMPLES/2; i++){
      for(int k=i*NUM_LEDS/(SAMPLES/2); k<i*NUM_LEDS/(SAMPLES/2)+vReal[i] && k<(i+1)*NUM_LEDS/(SAMPLES/2); k++){
        leds[k] = CHSV(hue+i*255/(SAMPLES/2),255,255);
      }
    }
    FastLED.show();
  }
}

//shows the first quarter frequency buckets in the same fashion as visualize_3()
void visualize_4(){
  uint8_t hue = random8();
  while(!pressed()){
    FastLED.clear();
    runFFT();
    for(int i=0; i<SAMPLES/8; i++){
      for(int k=i*NUM_LEDS/(SAMPLES/8); k<i*NUM_LEDS/(SAMPLES/8)+vReal[i] && k<(i+1)*NUM_LEDS/(SAMPLES/8); k++){
        leds[k] = CHSV(hue+i*255/(SAMPLES/8),255,255);
      }
    }
    FastLED.show();
  }
}

void idle_1(){

}



//finds the average from start to last inclusive
double avg(double* nums, int start, int last){
  double sum;
  for(int i = start; i < last+1; i++){
    sum += nums[i];
  }
  return sum/(last-start+1);
}

//finds the max from start to last inclusive
double findMax(double* nums, int start, int last){
  double temp=0;
  for(int i = start; i < last+1; i++){
    if(temp < nums[i]) temp = nums[i];
  }
  return temp;
}

double sum(double* nums, int start, int last){
  double temp=0;
  for(int i = start; i < last+1; i++){
    temp += nums[i];
  }
  return temp;
}

double approxSin(double theta){

}

//returns true if the button is currntly pressed
bool pressed(){
  bool temp = digitalRead(BUTTON_PIN);
  if(temp!=0){
    delay(250);
    return 1;
  }
  return 0;
}
