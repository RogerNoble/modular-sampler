#define ENCODER_OPTIMIZE_INTERRUPTS

#include <SPI.h>
#include <SD.h>
#define SD_ChipSelectPin 4
#include <TMRpcm.h>

#include <SoftwareSerial.h>
#include <Encoder.h>

TMRpcm tmrpcm;

// initial track
int track_number = 1;
//track file name
char current_track[10];

//total number of available tracks
int total_tracks = 1;

// ROTARY ENCODER
//rotary encoder init
Encoder knob(5, 6);

//initial position of encoder
int knob_pos = -999;

// rotary encoder states
enum ENCODER_DELTA {
  NONE,
  FWD,
  BACK
};

// debounce for encoder - it's very noisy!
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers

// TRIGGER
const int trigger_pin = 3;
int trigger_state = LOW;


void setup()
{
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_ChipSelectPin)) {
    Serial.println("initialization failed!");
    while (1);
  }
  
  tmrpcm.speakerPin = 9;
  tmrpcm.setVolume(4);
  
  // set up the trigger pin
  pinMode(trigger_pin, INPUT);

  knob_pos = knob.read();

  File root = SD.open("/");
  total_tracks = getDirectoryFilesCount(root);
  root.close();

  Serial.println("initialization done.");
}


void loop()
{
  bool should_play = false;
  
  // check if encoder position has updated
  ENCODER_DELTA delta = getEncoderDelta(knob.read());
  if(delta == FWD){
    Serial.println("FORWARD");
    track_number = nextTrack(track_number, total_tracks);
    should_play = true;
  } else if (delta == BACK){
    Serial.println("BACK");
    track_number = prevTrack(track_number, total_tracks);
    should_play = true;
  }
  
  //  check for button press
  int new_state = digitalRead(trigger_pin);
  if(new_state != trigger_state){
    Serial.println(new_state);
    if(new_state == HIGH){
      should_play = true;
    }
    trigger_state = new_state;
  }

  if(should_play){
    getCurrentAudioFile(track_number);
    tmrpcm.play(current_track);
  }
}

// gets the state of the rotary encoder
ENCODER_DELTA getEncoderDelta(int new_pos) {
  if((millis() - lastDebounceTime) > debounceDelay) {
    if(new_pos != knob_pos){
      // clockwise == forward
      ENCODER_DELTA delta = new_pos > knob_pos ? BACK : FWD;
      knob_pos = new_pos;
      lastDebounceTime = millis();
      return delta;
    }
  }
  knob_pos = new_pos;
  return NONE;
}

// Returns the next track index.
// Will cycle back to 0 if at the last track
int nextTrack(int current, int total) {
  if(current < total){
    return current + 1;
  }
  return 0;
}

// Returns the previous track index.
// Will cycle back to the last/end if at the first track
int prevTrack(int current, int total) {
  if(current > 0){
    return current - 1;
  }
  return total;
}

int getDirectoryFilesCount(File dir) {
  int count = 0;
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }

    if (!entry.isDirectory()) {
      count++;
    }
    entry.close();
  }
  return count;
}

void getCurrentAudioFile(int idx){
  sprintf(current_track, "drum%d.wav", idx);
}
