/* TO DO **************************************************************
 * make another 'WavFiles' array with another sound set
 * The array chosen by setValues() could vary based on a rotary switch!
 * consider 'nudge' button
 * add 'swing'?
 * LOW TOM sound (3) seems to pop at the end
 * Gain on BD could be higher? or a bit of saturation
 * New idea: 'wobble' button slightly randomises final delay value
 * ^- this would match up nicely with a new 'log drum' sound set
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

int activeLed = 0; //this sets the start position
int potVal = 0;
int pitchVal = 0;
int congaFiles[] = {0,1,2,3}; //these ints relate to sample numbers on the SD card; 0 is null but we could find a better way to do 'off' notes in the setValues() loops
int cowbellFiles[] = {0,0,0,4,5,6,7,8,9,10,11,12,13};
int instrumentSelect = 1; //referenced by setValues() and shuffle(). 0 = conga, 1 = cowbell. alter this to switch between em. probably need to setValues() when we do
int instPlayingNow = 1; //to prevent shuffling the two arrays together. should be able to just leave this(unless we add another array)
int backBeatPlaying = 0; //toggle the back beat
int stepValues[16];
int stepValuesPoly[16]; // To be used for polyphony... eventually
int currentStep = 0;
int switchButtonState = 0;
int beatButtonState = 0;
int shuffButtonState = 0;
int switchButtonCounter = 0;
int shuffButtonCounter = 0;
int evolveRed = 0;
int evolveGreen = 0;
int evolveBlue = 0;

Adafruit_NeoPixel ring = Adafruit_NeoPixel(16, RINGPIN);
wavTrigger wTrig;


void setValues(){
  for (int beat =  0; beat < 16; beat++) {
    if (instrumentSelect == 0) {
      stepValues[beat] = congaFiles[random(CGLENGTH)];
      stepValuesPoly[beat] = 0;
      instPlayingNow = 0;
    } else {
      stepValues[beat] = cowbellFiles[random(CBLENGTH)];
      int polySeed = random(0,8); //roll a D8 for chance to trigger a poly note
      if (polySeed < 1){
        stepValuesPoly[beat] = cowbellFiles[random(3, 13)]; //should give us just non-'null' notes. may be a more elegant way to do off notes in future!
      } else {
        stepValuesPoly[beat] = 0;
      }
      if (switchButtonCounter > 3) { //always have poly in random mode (but not always right at the start)
        stepValuesPoly[0] = cowbellFiles[random(3,13)];
      }
      instPlayingNow = 1;
    }
  }
}


void shuffle(){
  if (shuffButtonCounter < 1 || currentStep == 0) { //if the button's held, only shuffle on first step
    Serial.println("Shuffled!"); //for debug purposes
    for (int beat = 0; beat <16; beat++) {
      int shuffSeed = random(0, 5); //still looking for the best value but 5 is ok
      if (shuffSeed < 1) {
        if (instPlayingNow == 0){
          stepValues[beat] = congaFiles[random(CGLENGTH)]; //make sure the random number is the same as the Files array length
        } else {
          stepValues[beat] = cowbellFiles[random(CBLENGTH)];
          int polySeed = random(0,8);
          if (polySeed < 1) {
            stepValuesPoly[beat] = cowbellFiles[random(3, 13)];
          }
          if (polySeed > 5) { //chance to turn note off
            stepValuesPoly[beat] = 0;
          }
        }
      }
    }
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(SWITCHPIN, INPUT_PULLUP);
  pinMode(SHUFFPIN, INPUT_PULLUP);
  pinMode(BEATPIN, INPUT_PULLUP);
  randomSeed(analogRead(SEEDPIN)); //make sure this pin isn't connected to anything
  ring.begin();
  ring.setBrightness(127);
  ring.show(); // init pixels to 'off'
  setValues();
  wTrig.start();
  delay(10);
}


void loop() {

  //step management
  if (currentStep > 15) {
    currentStep = 0;
  }
  
  //button stuff
  switchButtonState = digitalRead(SWITCHPIN);
  shuffButtonState = digitalRead(SHUFFPIN);
  beatButtonState = digitalRead(BEATPIN);

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

    if (shuffButtonCounter == 1 || currentStep == 0) { //Set new step LED colour for each evolution session. this might need work for edge cases
      evolveRed = random(0,256);
      evolveBlue = random(0,256);
      evolveGreen = random(0,256);
    }
  } else {
    shuffButtonCounter = 0;
  }

  if (beatButtonState == 0) {
    backBeatPlaying = !backBeatPlaying;
  }



  //Ring stuff
  if (switchButtonCounter < 1) { //step LED in normal mode
    if (activeLed < 0){
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
    activeLed--;
  }
  else { //flash random colours if holding switch button
    if (switchButtonCounter > 1){
      if (stepValues[currentStep] > 0){
        int rand1 = random(0, 256);
        int rand2 = random(0, 256);
        int rand3 = random(0, 256);
        for (int i = 0; i < 16; i++){
          ring.setPixelColor(i, rand1, rand2, rand3);
        }
      } else {
        for (int i = 0; i < 16; i++) {
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

  //play samples
  if (backBeatPlaying == 1 && (currentStep == 0 || currentStep == 4 || currentStep == 8 || currentStep == 12)) {
    wTrig.trackPlayPoly(14); //that's the BD
  }
  wTrig.trackPlayPoly(stepValues[currentStep]); //PlayPoly is def better with cowbells. Congas may prefer PlaySolo
  wTrig.trackPlayPoly(stepValuesPoly[currentStep]); // lets see if this works
  wTrig.update();

  //Serial.println(potVal);
  //Serial.print("Step ");
  //Serial.println(currentStep);
  Serial.print("Evolve count: ");
  Serial.println(shuffButtonCounter);
  //Serial.print("Playing file ");
  //Serial.println(stepValuesPoly[currentStep]);
  //Serial.println(switchButtonState);

  currentStep++;
  delay(potVal);
}
