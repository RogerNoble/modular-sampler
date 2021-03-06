// #define ENCODER_OPTIMIZE_INTERRUPTS

#include <SdFat.h>
#include <sdios.h>
#include <TMRpcm.h>
#include <SPI.h>

#include <SoftwareSerial.h>
#include <Encoder.h>

//SDFAT
#define SD_FAT_TYPE 1
#define SPI_CLOCK SD_SCK_MHZ(50)
const uint8_t SD_CS_PIN = 4;
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)

SdFat32 sd;

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
  Serial.println(F("Initializing SD card..."));
  
  // Initialize the SD card.
  if (!sd.begin(SD_CONFIG)) {
    errorPrint();
    while (1);
  }
  
  tmrpcm.speakerPin = 9;
  tmrpcm.setVolume(4);
  
  // set up the trigger pin
  pinMode(trigger_pin, INPUT);

  knob_pos = knob.read();

  File32 root;
  if (!root.open("/")) {
    Serial.println(F("failed to open root"));
    while (1);
  }
  total_tracks = getDirectoryFilesCount(root);
  root.close();

  Serial.println(F("initialization done."));
}


void loop()
{
  bool should_play = false;
  
  // check if encoder position has updated
  ENCODER_DELTA delta = getEncoderDelta(knob.read());
  if(delta == FWD){
    // Serial.println(F("FORWARD"));
    track_number = nextTrack(track_number, total_tracks);
    should_play = true;
  } else if (delta == BACK){
    // Serial.println(F("BACK"));
    track_number = prevTrack(track_number, total_tracks);
    should_play = true;
  }
  
  //  check for button press
  int new_state = digitalRead(trigger_pin);
  if(new_state != trigger_state){
    if(new_state == HIGH){
      should_play = true;
    }
    trigger_state = new_state;
  }

  if(should_play){
    getCurrentAudioFile(track_number);
    Serial.println(current_track);
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
  return 1;
}

// Returns the previous track index.
// Will cycle back to the last/end if at the first track
int prevTrack(int current, int total) {
  if(current > 1){
    return current - 1;
  }
  return total;
}

int getDirectoryFilesCount(File32 dir) {
  File32 file;
  int count = 0;
  while (file.openNext(&dir, O_RDONLY)) {
    if (!file.isHidden()) {
      count++;
    }
    file.close();
  }
  return count;
}

void getCurrentAudioFile(int idx){
  sprintf(current_track, "drum%d.wav", idx);
}

void errorPrint() {
  if (sd.sdErrorCode()) {
    Serial.print(F("SD errorCode: "));
    Serial.println(int(sd.sdErrorCode()));
    Serial.print(F("SD errorData: "));
    Serial.println(int(sd.sdErrorData()));
  }
}
