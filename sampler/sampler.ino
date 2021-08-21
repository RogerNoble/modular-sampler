#include <SoftwareSerial.h>
//https://forum.arduino.cc/t/new-and-improved-dfplayer-mini-library/492914/6
#include <DFPlayerMini_Fast.h>

SoftwareSerial mySerial(10, 11); // RX, TX
DFPlayerMini_Fast myDFPlayer;

//trigger
const int trigger_pin = 4;
int trigger_state = LOW;

// initial MP3 track
int track_number = 1;

//total number of available tracks
int total_tracks = 1;

void setup()
{
  Serial.begin(115200);
  mySerial.begin(9600);
  
  if(!myDFPlayer.begin(mySerial)){
    // failed to init - set an error LED?
  }

  // volume is controlled outside of module so set to max == 30
  myDFPlayer.volume(30);

  total_tracks = myDFPlayer.numSdTracks();
  
  // set up the trigger pin
  pinMode(trigger_pin, INPUT);
}


void loop()
{  
  //  check for button press
  int new_state = digitalRead(switch_pin);
  if(new_state != trigger_state){
    if(new_state == HIGH){
      track_number = nextTrack(track_number, total_tracks);
      Serial.print("Playing: ");
      Serial.println(track_number);
      myDFPlayer.play(track_number);
    }
    trigger_state = new_state;
  }
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
