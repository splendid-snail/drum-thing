/* TO DO **************************************************************
 * make another 'WavFiles' array with another sound set
 * The array chosen by setValues() could vary based on a rotary switch!
 * Button could enable 4-to-the-floor BD <<DO THIS *
 * add 'swing'?
 * LOW TOM sound (3) seems to pop at the end
 */

#include <Adafruit_NeoPixel.h>
#include <wavTrigger.h>
#include <AltSoftSerial.h> //serial TX pin is 9. Remember to connect WT to ground

#define RINGPIN 5
#define SPEEDPIN A0
#define PITCHPIN A1
#define SWITCHPIN 2

int redLed = 15; //this sets the start position
int potVal = 0;
int pitchVal = 0;
int wavFiles[] = {0,1,2,3}; //these ints relate to sample numbers on the SD card; 0 is null
int stepValues[16];
int currentStep = 0;
int switchButtonState = 0;

Adafruit_NeoPixel ring = Adafruit_NeoPixel(16, RINGPIN);
wavTrigger wTrig;

//The only setValues function - for now
void setValues(){
  for(int beat =  0; beat < 16; beat++){
    stepValues[beat] = wavFiles[random(4)]; //make sure the random number is the same as the Files array length
  }
}


//shuffles the current pattern obvs
void shuffle(){
  int shufSeed;
  for (int beat = 0; beat <16; beat++){
    int shufSeed = random(0, 5); //still looking for the best value but this is good
    if (shufSeed < 1){
      stepValues[beat] = wavFiles[random(4)]; //just like the setValues function
    }       
  }
}


void setup() {
  pinMode(SWITCHPIN, INPUT_PULLUP);
  Serial.begin(9600);
  randomSeed(analogRead(5)); //make sure this pin isn't connected to anything
  ring.begin();
  ring.setBrightness(127);
  ring.show(); // init pixels to 'off'
  wTrig.start(); 
  delay(10);
  setValues();
}



void loop() {
  //Switch button (shuffle too for now, comment out function as required)
  //note that dedicated shuffle button won't reset the step/LED
  switchButtonState = digitalRead(SWITCHPIN);
  if (switchButtonState == 0){
    //setValues();
    shuffle();
    //redLed = 15;
    //currentStep = 0;
  }

  //step management
  if (currentStep > 15){
    currentStep = 0;
  }

  //Set all pixels to blue
  for (int i = 0; i < 16; i++) {
     ring.setPixelColor(i, 0, 0, 255);
  }

  //red led bits
  if (redLed < 0){
    redLed = 15;
  }

  ring.setPixelColor(redLed, 255, 0, 0);

  redLed--;
  ring.show();

  //read potval, 0-1023
  potVal = analogRead(SPEEDPIN);
  //Serial.println(potVal);
  //map potval
  potVal = map(potVal, 0, 1023, 80, 800); //tweak latter values for min/max step length was 30/1000

  //read pitchval and set
  pitchVal = analogRead(PITCHPIN);
  //Serial.println(pitchVal);
  pitchVal = map(pitchVal, 0, 1023, 32676, -32767);
  wTrig.samplerateOffset(pitchVal);

  //play a sample
  wTrig.trackPlaySolo(stepValues[currentStep]); // try PlaySolo or PlayPoly
  wTrig.update();  //possibly not necessary as we're not 'reporting' - see library documentation

  //Serial debug stuff
  //Serial.println(potVal);
  Serial.print("Step ");
  Serial.println(currentStep);
  Serial.print("Playing file ");
  Serial.println(stepValues[currentStep]);
  //Serial.println(switchButtonState);

  //increment step and delay
  currentStep++;
  delay(potVal);
}
