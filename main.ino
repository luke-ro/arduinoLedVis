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

struct Wave{
  int center;
  int radius;
  int amplitude;
  uint8_t hue;
  uint8_t saturation;
  uint8_t brightness;
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN,INPUT);
  ADCSRA = 0b11100101;      // set ADC to free running mode and set pre-scalar to 32 (0xe5)
  ADMUX = 0b00000000;       // use pin A0 and external voltage reference

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
//  FastLED.setBrightness(100);

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
//  test();
  random_bumps();
  idle_1();
//  visualize_1();
//  visualize_2();
  visualize_3();
//  visualize_4();
//  visualize_5();
  visualize_6();

}

void test(){
  unsigned long start = millis();
  unsigned long counter=0;
  while(!pressed()){
    approxSin(counter++);
  }
  unsigned long endTime = millis();
  double milsPerLoop = (endTime-start)/double(counter);
  Serial.print("test() milliseconds per ");
  Serial.print(counter);
  Serial.print(" loops: ");
  Serial.println(milsPerLoop,10);
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
  double milsPerLoop = (endTime-start)/double(counter);
  Serial.print("visulaize_1() millisecond per loops: ");
  Serial.println(milsPerLoop,10);
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
  double milsPerLoop = (endTime-start)/double(counter);
  Serial.print("visulaize_2() millisecond per loops: ");
  Serial.println(milsPerLoop,10);
}

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

void visualize_5(){
  int len = 25;
  int bassDelay = 175;//time in milliseconds between creation of bass waves
  unsigned long lastBass=millis();
  double bassValue, bassAbsoluteMax=0;
  struct Wave bassWaves[len];
//  struct Wave midWaves[len];
//  struct Wave highWaves[len];
  int bassStart= 0, bassEnd= 0;
//  int midStart = 0, midEnd = 0;
//  int highStart= 0, highEnd= 0;
  //newWave(bassWaves,bassEnd++%len);
  while(!pressed()){
    runFFT();

    bassValue = findMax(vReal,0,SAMPLES/8); //largest number in the first quarter of vReal[]
    if(bassValue>bassAbsoluteMax) bassAbsoluteMax=bassValue;
    if(bassValue>bassAbsoluteMax*0.7 && bassDelay<millis()-lastBass){
      newWave(bassWaves,bassEnd++%len,0,10,random8(),255,255);
      lastBass = millis();
    }
    int count = bassStart%len;
    FastLED.clear();
    for(int count=bassStart%len; count%len!=bassEnd%len; count= ++count%len){

      struct Wave* thisWave = &bassWaves[count];

      for(int i=thisWave->center-thisWave->radius; i<thisWave->center+thisWave->radius; i++){

        if(i>-1 && i<NUM_LEDS+1){
//          Serial.print(127.5*(approxCos(3.14*(double(i)-thisWave->center)/thisWave->radius)+1));
//          Serial.print(" ");
          leds[i] = CHSV(thisWave->hue,thisWave->saturation, thisWave->brightness/2*(approxCos(3.14*(double(i)-thisWave->center)/thisWave->radius)+1));
        }
      }
      thisWave->center++;
      //thisWave->brightness = -255/(thisWave->radius+NUM_LEDS)*thisWave->center+255;
      if(thisWave->center-thisWave->radius > NUM_LEDS) bassStart = ++bassStart%len;
//      Serial.println();

    }
    FastLED.show();
  }
}

void visualize_6(){
  int len = 10; //size of arrays
  int bassDelay = 250;//time in milliseconds between creation of bass waves
  unsigned long lastBass=millis();
  unsigned long resetTime = millis();
  double bassValue, bassAbsoluteMax=0;
  struct Wave bassWaves[len];


//  struct Wave midWaves[len];
//  struct Wave highWaves[len];
  int bassStart= 0, bassEnd= 0;
//  int midStart = 0, midEnd = 0;
//  int highStart= 0, highEnd= 0;
  //newWave(bassWaves,bassEnd++%len);
  for(int i=0; i<len; i++) newWave(bassWaves,i,-100,0,0,0,0); //assigns a value to each wave in the array
  unsigned long counter=0;
  unsigned long startTime = millis();                                                              //to get rid of trash values
  while(!pressed()){
    runFFT();
    if(millis()-resetTime>10000){//resets the bass absolute max after 2 minutes
      resetTime=millis();
      bassAbsoluteMax /= 2;
    }
    bassValue = findMax(vReal,0,1); //largest number in the first quarter of vReal[]
//    bassValue = vReal[0];
    if(bassValue>bassAbsoluteMax) bassAbsoluteMax=bassValue;
    if(bassValue>bassAbsoluteMax*0.5 && bassDelay<millis()-lastBass){
      newWave(bassWaves,bassEnd%len,int(random(NUM_LEDS)),0,random8(),255,255);
      if(bassEnd++==bassStart-1) bassStart=++bassStart%len;
      lastBass = millis();
//      Serial.print(bassEnd);
//      Serial.print(" ");
    }
    int count = bassStart%len;
    FastLED.clear();
    for(int i=0; i<NUM_LEDS; i++) leds[i] = CHSV(0,0,1);      //creates a white background
    for(int count=bassStart%len; count%len!=bassEnd%len; count= ++count%len){

      struct Wave* thisWave = &bassWaves[count];
      int center = thisWave->center;
      int radius = thisWave->radius;
      double piDivRadius = 3.1415/thisWave->radius;
      double centerxPiDivRadius = thisWave->center*piDivRadius;
      double brightnessDiv2 = thisWave->brightness/2;
      for(int i=thisWave->center-thisWave->radius; i<thisWave->center+thisWave->radius; i++){

        if(i>-1 && i<NUM_LEDS+1){
//          Serial.print(127.5*(approxCos(3.14*(double(i)-thisWave->center)/thisWave->radius)+1));
//          Serial.print(" ");
//          leds[i] += CHSV(thisWave->hue,255, thisWave->brightness/2*(approxCos(3.14*(double(i)-thisWave->center)/thisWave->radius)+1));
          leds[i] += CHSV(thisWave->hue,255, brightnessDiv2*(approxCos(piDivRadius*i-centerxPiDivRadius))+brightnessDiv2);
        }
      }
      thisWave->radius+=2;
      thisWave->brightness-=5;
      //thisWave->brightness = -255/(thisWave->radius+NUM_LEDS)*thisWave->center+255;
      if(thisWave->brightness<1) bassStart = ++bassStart%len;
//      Serial.println();

    }
    counter++;
    FastLED.show();
  }
    //for testing speed
  unsigned long endTime = millis();
  double milsPerLoop = (endTime-startTime)/double(counter);
  Serial.print("visulaize_6() millisecond per loop: ");
  Serial.println(milsPerLoop,10);
}

void random_bumps(){
  int arrayStart= 0, arrayEnd= 0;                 //used to keep track of start and end points of array.
                                                  //New waves will be added at arrayEnd%10;
  const int len = 10;                             //size of arrays
  const int maxCyclesPerSecond = 30;
  double minTimePerLoop = 1000/maxCyclesPerSecond;//minimum time per cycle ("frames per second")
  unsigned long loopStartTime;                    //Used to keep tracl of how long a loop has lasted
  unsigned long nextWave =0;                      //will be assigned a random value. Time until next wave is created
  unsigned long lastWave = millis();

  struct Wave bassWaves[len];                     //array that stores all the waves

  for(int i=0; i<len; i++) newWave(bassWaves,i,-100,0,0,0,0); //assigns a value to each wave in the array
                                                              //to get rid of trash values
  while(!pressed()){
    loopStartTime = millis();
    if(millis()-lastWave>nextWave){     //resets the bass absolute max after 2 minutes
      newWave(bassWaves,arrayEnd++%len,int(map(random8(),0,255,0,NUM_LEDS)),0,random8(),255,255);
      lastWave=millis();
      nextWave=random(500,1500);
    }

    int count = arrayStart%len;
    FastLED.clear();
    for(int i=0; i<NUM_LEDS; i++) leds[i] = CHSV(0,0,1);      //creates a white background

    for(int count=arrayStart%len; count%len!=arrayEnd%len; count= ++count%len){
      struct Wave* thisWave = &bassWaves[count];

      for(int i=thisWave->center-thisWave->radius; i<thisWave->center+thisWave->radius; i++){
        if(i>-1 && i<NUM_LEDS+1){
          leds[i] += CHSV(thisWave->hue,thisWave->saturation, thisWave->brightness/2*(approxCos(3.14*(double(i)-thisWave->center)/thisWave->radius)+1));
        }
      }
      thisWave->radius++;
      thisWave->brightness-=3;
      if(thisWave->brightness<1){
        arrayStart = ++arrayStart%len; //iterate the start index of the array to lighten load
        thisWave->center = -1000;       //move the center of the wave far away to prevent wierd behavior
      }

    }
    while(millis()-loopStartTime<minTimePerLoop);
    FastLED.show();
  }
}

void newWave(struct Wave* arr, int index,int center,int radius,uint8_t hue,uint8_t saturation,uint8_t brightness){
  arr[index].center = center;
  arr[index].radius = radius;
  arr[index].hue = hue;
  arr[index].saturation = saturation;
  arr[index].brightness = brightness;
}

void idle_1(){
  unsigned long start = millis();
  int counter=0;

  uint8_t hue = 0;
  uint8_t phase = 0;
  while(!pressed()){
    FastLED.clear();
    counter++;
    phase++;
    hue++;
    for(int i=0; i<NUM_LEDS; i++){
      leds[i] = CHSV(uint8_t(i+hue),255,127*(-approxSin(0.05*i+.0246*phase)+1));
    }
    FastLED.show();
  }

  //for testing speed
  unsigned long endTime = millis();
  double milsPerLoop = (endTime-start)/double(counter);
  Serial.print("idle_1() millisecond per loops: ");
  Serial.println(milsPerLoop,10);
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

//uses two parabolas to approximate sine
//Can have error as high as 27%
//see https://www.desmos.com/calculator/vvd2lmtpwy
double approxSin(double theta){
  theta = fmod(theta,6.2832);
  if(theta<0){//since sine is an odd fuction, the results can be multiplied by (-1) if theta is (-)
    theta = -theta;
    if(theta<3.1416){
      return 0.405285*pow(theta-1.5708,2)-1; //-4(pi^-2)(theta-pi/2)^2+1
    }else{
      return  -0.405285*pow(theta-4.7124,2)+1; //4(pi^-2)(theta-3pi/2)^2-1
    }
  }

  if(theta<3.1416){
    return -0.405285*pow(theta-1.5708,2)+1; //-4(pi^-2)(theta-pi/2)^2+1
  }
  return  0.405285*pow(theta-4.7124,2)-1; //4(pi^-2)(theta-3pi/2)^2-1

}

//approximates cosine using parabolas.
//Can have error as high as 27%
//https://www.desmos.com/calculator/m7hfy64b4e
double approxCos(double theta){
  theta = fmod(theta,6.2832);
  if(theta<0) theta=-theta;
  if(theta<1.5708){
    return -0.405285*pow(theta,2)+1; //-4(pi^-2)(theta)^2+1
  }
  if(theta<4.7123){
    return 0.405285*pow(theta-3.1416,2)-1; //4(pi^-2)(theta-pi)^2-1
  }
  return -0.405285*pow(theta-6.2832,2)+1; //-4(pi^-2)(theta-2pi)^2+1
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
