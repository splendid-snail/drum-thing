/* TO DO **************************************************************
 * make another 'WavFiles' array with another sound set
 * The array chosen by setValues() could vary based on a rotary switch!
 * consider 'nudge' button
 * add 'swing'?
 * LOW TOM sound (3) seems to pop at the end
 * Gain on BD could be higher? or a bit of saturation
 */

#include <Adafruit_NeoPixel.h>
#include <wavTrigger.h>
#include <AltSoftSerial.h> //serial TX pin is 9. Remember to connect WT to ground

#define SPEEDPIN A0
#define PITCHPIN A1
#define BEATPIN A2
#define SEEDPIN A5
#define SHUFFPIN 2
#define SWITCHPIN 3
#define RINGPIN 5
//lengths of the various file arrays. keep these up to date or it breaks
#define CGLENGTH 4
#define CBLENGTH 13

int redLed = 0; //this sets the start position
int potVal = 0;
int pitchVal = 0;
int congaFiles[] = {0,1,2,3}; //these ints relate to sample numbers on the SD card; 0 is null
int cowbellFiles[] = {0,0,0,4,5,6,7,8,9,10,11,12,13};
int instrumentSelect = 1; //referenced by setValues() and shuffle(). 0 = conga, 1 = cowbell. alter this to switch between em
int playingNow = 1; //to prevent shuffling the two arrays together. should be able to just leave this(unless we add another array)
int backBeat = 0; //toggle the back beat
int stepValues[16];
int currentStep = 0;
int switchButtonState = 0;
int beatButtonState = 0;
int shuffButtonState = 0;
int switchButtonCounter = 0;

Adafruit_NeoPixel ring = Adafruit_NeoPixel(16, RINGPIN);
wavTrigger wTrig;

//The only setValues function - for now
void setValues(){
  for (int beat =  0; beat < 16; beat++){
    if (instrumentSelect == 0){
      stepValues[beat] = congaFiles[random(CGLENGTH)]; //make sure the random number is the same as the Files array length
      playingNow = 0;
    }
    else {
      stepValues[beat] = cowbellFiles[random(CBLENGTH)];
      playingNow = 1;
    }
  }
}


//shuffles the current pattern obvs
void shuffle(){
  for (int beat = 0; beat <16; beat++){
    int shuffSeed = random(0, 5); //still looking for the best value but this is ok
    if (shuffSeed < 1){
      if (playingNow == 0){
        stepValues[beat] = congaFiles[random(CGLENGTH)]; //make sure the random number is the same as the Files array length
      }
      else{
        stepValues[beat] = cowbellFiles[random(CBLENGTH)];
      }
    }
  }
}


void setup() {
  pinMode(SWITCHPIN, INPUT_PULLUP);
  pinMode(SHUFFPIN, INPUT_PULLUP);
  pinMode(BEATPIN, INPUT_PULLUP);
  Serial.begin(9600);
  randomSeed(analogRead(SEEDPIN)); //make sure this pin isn't connected to anything
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
  beatButtonState = digitalRead(BEATPIN);

  //Holding this button causes completely random playback
  if (switchButtonState == 0){
    setValues();
    redLed = 0;
    currentStep = 0;
	switchButtonCounter++;
  } else{
  	switchButtonCounter = 0;
  }

  if (shuffButtonState == 0){
    shuffle();
  }

  if (beatButtonState == 0){
    backBeat = !backBeat;
  }

  //step management
  if (currentStep > 15){
    currentStep = 0;
  }

  //RING STUFF
  if (switchButtonCounter < 1 ){ //step red LED in normal mode
	  if (redLed < 0){
	    redLed = 15;
	  }
    for (int i = 0; i < 16; i++) {
     ring.setPixelColor(i, 0, 0, 255);
    }
    ring.setPixelColor(redLed, 255, 0, 0);
	  redLed--;
  }
  else{ //flash random colours if holding switch button
	  if (switchButtonCounter > 1){
      if (stepValues[currentStep] > 0){
        int rand1 = random(0, 256);
        int rand2 = random(0, 256);
        int rand3 = random(0, 256);
        for (int i = 0; i < 16; i++){
          ring.setPixelColor(i, rand1, rand2, rand3);
        }
	    }
      else{
        for (int i = 0; i < 16; i++){
          ring.setPixelColor(i, 0, 0, 0);
        }
      }
    }
  }
  ring.show();

  //read pots
  potVal = analogRead(SPEEDPIN);
  potVal = map(potVal, 0, 1023, 80, 600); //tweak latter values for min/max step length

  pitchVal = analogRead(PITCHPIN);
  pitchVal = map(pitchVal, 0, 1023, 32676, -32767);
  wTrig.samplerateOffset(pitchVal);

  //play a sample
  if (backBeat == 1 && (currentStep == 0 || currentStep == 4 || currentStep == 8 || currentStep == 12)){
		wTrig.trackPlayPoly(14);
	}
  wTrig.trackPlayPoly(stepValues[currentStep]); //PlayPoly is def better with cowbells. Congas may prefer PlaySolo
  wTrig.update();

  //Serial debug stuff
  //Serial.println(potVal);
  //Serial.print("Step ");
  //Serial.println(currentStep);
  //Serial.print("Hold count: ");
  //Serial.println(switchButtonCounter);
  //Serial.print("Playing file ");
  //Serial.println(stepValues[currentStep]);
  //Serial.println(switchButtonState);

  //increment step and delay
  currentStep++;
  delay(potVal);
}
