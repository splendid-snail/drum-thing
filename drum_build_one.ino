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
#define SWITCHPIN 3
#define SHUFFPIN 2

int redLed = 15; //this sets the start position
int potVal = 0;
int pitchVal = 0;
int congaFiles[] = {0,1,2,3}; //these ints relate to sample numbers on the SD card; 0 is null
int cowbellFiles[] = {0,0,4,5,6,7,8,9,10,11,12,13}; //ditto as above. Could add another null?
int instrumentSelect = 1; //referenced by setValues() and shuffle() to determine which Files array to use. 0 = conga, 1 = cowbell. Currently has to be set here.  
int playingNow = 1; //to prevent shuffling the two arrays together
int stepValues[16];
int currentStep = 0;
int switchButtonState = 0;
int shuffButtonState = 0;

Adafruit_NeoPixel ring = Adafruit_NeoPixel(16, RINGPIN);
wavTrigger wTrig;

//The only setValues function - for now
void setValues(){
  for (int beat =  0; beat < 16; beat++){
    if (instrumentSelect == 0){
      stepValues[beat] = congaFiles[random(4)]; //make sure the random number is the same as the Files array length
      playingNow = 0;  
    }
    else {
      stepValues[beat] = cowbellFiles[random(12)];
      playingNow = 1;    
    }
  }
}


//shuffles the current pattern obvs
void shuffle(){
  int shuffSeed; //redundant?
  for (int beat = 0; beat <16; beat++){
    int shuffSeed = random(0, 5); //still looking for the best value but this is ok
    if (shuffSeed < 1){      
      if (playingNow == 0){
        stepValues[beat] = congaFiles[random(4)]; //make sure the random number is the same as the Files array length  
      }
      else {
        stepValues[beat] = cowbellFiles[random(12)];    
      }    
    }
  }
}


void setup() {
  pinMode(SWITCHPIN, INPUT_PULLUP);
  pinMode(SHUFFPIN, INPUT_PULLUP);
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
  switchButtonState = digitalRead(SWITCHPIN);
  shuffButtonState = digitalRead(SHUFFPIN);

  //Holding this button causes completely random playback. could  make the ring flash red w 'button held counter' variable?
  if (switchButtonState == 0){
    setValues();
    redLed = 15;
    currentStep = 0;
  }

  if (shuffButtonState == 0){
    shuffle();
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
  potVal = map(potVal, 0, 1023, 80, 600); //tweak latter values for min/max step length

  //read pitchval and set
  pitchVal = analogRead(PITCHPIN);
  //Serial.println(pitchVal);
  pitchVal = map(pitchVal, 0, 1023, 32676, -32767);
  wTrig.samplerateOffset(pitchVal);

  //play a sample
  wTrig.trackPlaySolo(stepValues[currentStep]); // try PlaySolo or PlayPoly
  wTrig.update();

  //Serial debug stuff
  Serial.println(potVal);
  Serial.print("Step ");
  Serial.println(currentStep);
  Serial.print("Playing file ");
  Serial.println(stepValues[currentStep]);
  //Serial.println(switchButtonState);

  //increment step and delay
  currentStep++;
  delay(potVal);
}
