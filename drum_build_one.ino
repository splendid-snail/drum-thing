/* TO DO **************************************************************
 * consider 'nudge' button
 * add 'swing'?
 * LOW TOM sound (3) seems to pop at the end
 * Gain on BD could be higher? PLUS maraca?
 * New idea: 'wobble' button slightly randomises final delay value
 * ^- this would match up nicely with a new 'log drum' sound set
 * New idea for gush mode: hold the same poly note for 16 counts then switch?
 */

#include <Adafruit_NeoPixel.h>
#include <wavTrigger.h>
#include <AltSoftSerial.h> //serial TX pin is 9. Remember to connect WT to ground

#define SPEEDPIN A0
#define PITCHPIN A4
#define BEATPIN A2 // why is this analog??
#define SEEDPIN A5
#define SHUFFPIN 2
#define SWITCHPIN 3
#define GBUTPIN 13
#define RINGPIN 5
//selector pins
#define CGPIN 10
#define CBPIN 11
#define LGPIN 12
//lengths of the various file arrays. keep these up to date or it breaks
#define CGLENGTH 4
#define CBLENGTH 13
#define GLENGTH 16
#define LGLENGTH 9

int activeLed = 0; //this sets the start position
int potVal;
int pitchVal;
int congaFiles[] = {0,1,2,3}; //these ints relate to sample numbers on the SD card; 0 is null but we could find a better way to do 'off' notes in the setValues() loops
int cowbellFiles[] = {0,0,0,4,5,6,7,8,9,10,11,12,13};
int greggFiles[] = {0,0,0,15,16,17,18,19,20,21,22,23,24,25,26,27}; //maybe needs more null
int logFiles[] = {0,0,28,29,30,31,32,33,34};
int instrumentSelected; //referenced by setValues() and shuffle(). 0 = conga, 1 = cowbell, 2 = greg).
int instPlayingNow; //to prevent shuffling the arrays together
int backBeatPlaying = 0; //toggle the back beat
int stepValues[16];
int stepValuesPoly[16];
int stepValuesGregg[16];
int currentStep = 0;
int switchButtonState = 0;
int beatButtonState = 0;
int shuffButtonState = 0;
int greggButtonState = 0;
int greggPlaying = 0;
int switchButtonCounter = 0;
int shuffButtonCounter = 0;
int evolveRed;
int evolveGreen;
int evolveBlue;

Adafruit_NeoPixel ring = Adafruit_NeoPixel(16, RINGPIN);
wavTrigger wTrig;


void setValues(){
  for (int beat =  0; beat < 16; beat++) {
    if (instrumentSelected == 0) {
      stepValues[beat] = congaFiles[random(CGLENGTH)];
      stepValuesPoly[beat] = 0;
      instPlayingNow = 0;
    } else if (instrumentSelected == 1){
      stepValues[beat] = cowbellFiles[random(CBLENGTH)];
      int polySeed = random(0,8); //roll a D8 for chance to trigger a poly note
      if (polySeed < 1) {
        stepValuesPoly[beat] = cowbellFiles[random(3, CBLENGTH)]; //should give us just non-'null' notes. may be a more elegant way to do off notes in future!
      } else {
        stepValuesPoly[beat] = 0;
      }
      if (switchButtonCounter > 3) { //always have poly in random mode (but not always right at the start)
        stepValuesPoly[0] = cowbellFiles[random(3,CBLENGTH)];
      }
      instPlayingNow = 1;
    } else if (instrumentSelected == 2) {
      stepValues[beat] = logFiles[random(LGLENGTH)];
      stepValuesPoly[beat] = 0;
      instPlayingNow = 2;
    }
  }
}

void setGregg(){
	for (int beat = 0; beat < 16; beat++) {
		stepValuesGregg[beat] = greggFiles[random(GLENGTH)];
	}
}


void shuffle(){
  if (shuffButtonCounter < 1 || currentStep == 0) { //if the button's held, only shuffle on first step
    Serial.println("Shuffled!"); //for debug purposes
    for (int beat = 0; beat <16; beat++) {
      int shuffSeed = random(0, 5); //still looking for the best value but 5 is ok
      if (shuffSeed < 1) {
        if (instPlayingNow == 0) {
          stepValues[beat] = congaFiles[random(CGLENGTH)]; //make sure the random number is the same as the Files array length
        } else if (instPlayingNow == 1) {
          stepValues[beat] = cowbellFiles[random(CBLENGTH)];
          int polySeed = random(0,8);
          if (polySeed < 1) {
            stepValuesPoly[beat] = cowbellFiles[random(3, 13)];
          }
          if (polySeed > 5) { //chance to turn note off
            stepValuesPoly[beat] = 0;
          }
        } else if (instPlayingNow == 2) {
          stepValues[beat] = logFiles[random(LGLENGTH)];
        }
      }
    }
  }
}


void setInstrument(){
  if (digitalRead(CGPIN) == 0) {
    instrumentSelected = 0;
  }
  if (digitalRead(CBPIN) == 0) {
    instrumentSelected = 1;
  }
  if (digitalRead(LGPIN) == 0) {
    instrumentSelected = 2;
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(SWITCHPIN, INPUT_PULLUP);
  pinMode(SHUFFPIN, INPUT_PULLUP);
  pinMode(BEATPIN, INPUT_PULLUP);
  pinMode(CGPIN, INPUT_PULLUP);
  pinMode(CBPIN, INPUT_PULLUP);
	pinMode(LGPIN, INPUT_PULLUP);
	pinMode(GBUTPIN, INPUT_PULLUP);
  randomSeed(analogRead(SEEDPIN)); //make sure this pin isn't connected to anything
  ring.begin();
  ring.setBrightness(127);
  ring.show(); // init pixels to 'off'
  setInstrument();
  setValues();
	setGregg(); //here for now
  wTrig.start();
  delay(10);
}


void loop() {
//select instrument
  setInstrument();

//step management
  if (currentStep > 15) {
    currentStep = 0;
  }

//button stuff
  switchButtonState = digitalRead(SWITCHPIN);
  shuffButtonState = digitalRead(SHUFFPIN);
  beatButtonState = digitalRead(BEATPIN);
	greggButtonState = digitalRead(GBUTPIN);

  if (switchButtonState == 0) {
    setValues();
    activeLed = 0;
    currentStep = 0;
    switchButtonCounter++;
  } else {
    switchButtonCounter = 0;
  }

  if (shuffButtonState == 0) {
    if (shuffButtonCounter == 0 || currentStep == 0) { //only shuffle on first press OR first step - 'evolve mode' if held
      shuffle();
    }

    shuffButtonCounter++;

    if (shuffButtonCounter == 1 || currentStep == 0) { //Set new stepping LED colour for each evolution sequence
      evolveRed = random(0,256);
      evolveBlue = random(0,256);
      evolveGreen = random(0,256);
    }
  } else {
    shuffButtonCounter = 0;
  }

  if (beatButtonState == 0) {
    backBeatPlaying = 0;
  } else {
    backBeatPlaying = 1;
  }

	if (greggButtonState == 0) {
		greggPlaying = !greggPlaying;
		setGregg();
	}

//LED ring stuff
  if (switchButtonCounter < 2) { //step LED in normal mode
    if (activeLed < 0) {
      activeLed = 15;
    }
    for (int i = 0; i < 16; i++) {
     ring.setPixelColor(i, 0, 0, 255);
    }
    if (shuffButtonCounter > 0) {
      ring.setPixelColor(activeLed, evolveRed, evolveBlue, evolveGreen);
    } else {
      ring.setPixelColor(activeLed, 255, 0, 0);
    }
  }
  else { //flash random colours if holding switch button
    if (switchButtonCounter > 1) { //is this redundant?
      if (stepValues[currentStep] > 0) {
        int rand1 = random(0, 256);
        int rand2 = random(0, 256);
        int rand3 = random(0, 256);
        for (int i = 0; i < 16; i++) {
          ring.setPixelColor(i, rand1, rand2, rand3);
        }
      } else {
        for (int i = 0; i < 16; i++) {
          ring.setPixelColor(i, 0, 0, 0);
        }
      }
    }
  }

  activeLed--;
  ring.show();

//read pots
  potVal = analogRead(SPEEDPIN);
  potVal = map(potVal, 0, 1023, 80, 600); //tweak latter values for min/max step length in ms

  pitchVal = analogRead(PITCHPIN);
  pitchVal = map(pitchVal, 0, 1023, 32676, -32767);
  wTrig.samplerateOffset(pitchVal);

//play samples
  if (backBeatPlaying == 1 && (currentStep == 0 || currentStep == 4 || currentStep == 8 || currentStep == 12)) {
    wTrig.trackPlayPoly(14); //that's the BD
  }

  if (instPlayingNow == 0 || instPlayingNow == 2) { //determine solo or poly playback
    wTrig.trackPlaySolo(stepValues[currentStep]);
  } else { // poly
    wTrig.trackPlayPoly(stepValues[currentStep]);
  }

	if (greggPlaying == 1) {
		wTrig.trackPlayPoly(stepValuesGregg[currentStep]);
	}

  wTrig.trackPlayPoly(stepValuesPoly[currentStep]);
  wTrig.update();

//VARIOUS DEBUG THINGS
  //Serial.print("Step ");
  //Serial.println(currentStep);
  Serial.print("CG pin ");
  Serial.println(digitalRead(CGPIN));
  Serial.print("CB pin ");
  Serial.println(digitalRead(CBPIN));
	Serial.print("LG pin ");
	Serial.println(digitalRead(LGPIN));
	Serial.print("Gregg button pin: ");
	Serial.println(greggButtonState);
	Serial.print("Gregg playing: ");
	Serial.println(greggPlaying);
  //Serial.print("LED: ");
  //Serial.println(activeLed);
  //Serial.print("Playing file ");
  //Serial.println(stepValuesPoly[currentStep]);
  //Serial.println(potVal);
  //Serial.println(switchButtonState);
  Serial.println();

  currentStep++;
  delay(potVal);
}
