#include "waves.hpp"

Waves::Waves(int numLeds){
  head = NULL;
  tail = NULL;

  CRGB* leds= New CRGB[numLeds];
}

Waves::isEmpty(){
  if (head == NULL){
    return true;
  }else{
    return false;
  }
}

Waves::addWave(){
  wave* newWave = new Wave()
  newWave->center = map(random8(),0,255,0,NUM_LEDS);
  newWave->spread = 0;
  newWave->amplitude = 5;
  newWave->color = random8();
  newWave->brightness = 255;
  newWave->next = NULL;
  newWave->previous = NULL;

  if(head==NULL){
    head = newWave;
    tail = newWave;
  }else{
    newWave->next = head;
    head->previous = newWave;
    head = newWave;
  }
  return;
}

Waves::removeLast(){
  if(tail==NULL) return;

  Wave* delPtr;
  if(tail->previous==NULL){
    delPtr = tail;
    head = NULL;
    tail = NULL;
    delete(delPtr);
    return;
  }
  delPtr = tail;
  tail->previous->next = NULL;
  tail = tail->previous;
  delete(delPtr);
  return;
}

Waves::removeAll(){
  if(isEmpty()){
    return;
  }
  Wave* delPtr;
  Wave* temp = head;
  do{
    delPtr = temp;
    temp = temp->next;
    delete delPtr;
  }while(temp->next!=NULL);
  delete temp;
  head = NULL;
  tail = NULL;
}

Waves::iterate(){
  Wave* temp = tail;
  CRGB* leds= New CRGB[numLeds];
  while(temp!=NULL){

  }
}
