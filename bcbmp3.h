#ifndef BCBMP3
#define BCBMP3
#include <DFPlayerMini_Fast.h>
#include "bcbsdcard.h"
#define RXD2 16
#define TXD2 17
DFPlayerMini_Fast myDFPlayer;
void soundAlarm();
// define state
struct mp3Status {
  String playState;
  String playMode;
  int volume;
  int currentTrack;
  bool update;
  int alarmHour;
  int alarmMinute;
  bool alarmSet;
  bool soundAlarm;
  int eq;
  String filename;
};
mp3Status state = {"STOPPED", "STOPPED", 5, 0, true, -1, -1, false, false, 0, ""};

void mp3Begin() {

  randomSeed(analogRead(A0));
  myDFPlayer.begin(Serial2);
  delay(10);

  //----Set volume----
  myDFPlayer.volume(5);
  myDFPlayer.EQSelect(1);
  delay(1000);
}

void mp3Command(String command) {

  if (command == "stop") {
    myDFPlayer.stop(); // stop playing
    delay(500);
    if (state.volume == 30) {
      myDFPlayer.volume(10);
      state.volume = 10;
    }
  }

  if (command == "play") {
    int randomNum = 0;
    randomNum = random(1, myDFPlayer.numSdTracks());
    myDFPlayer.play(randomNum); // play
    delay(500);
  }

  if (command == "next") {
    state.currentTrack += 1;
    myDFPlayer.play(state.currentTrack);
    delay(500);
  }

  if (command == "prev") {
    state.currentTrack -= 1;
    myDFPlayer.play(state.currentTrack); // Random play all the mp3.
    delay(500);
  }

  if (command == "shuf") {
    myDFPlayer.randomAll(); // Random play all the mp3.
    delay(500);
  }

  if (command == "loop") {
    myDFPlayer.startRepeatPlay(); // repeat play all the mp3.
    delay(500);
  }

  if (command == "up") {
    myDFPlayer.incVolume(); // Volume Up
    delay(500);
    state.volume += 1;
  }

  if (command == "down") {
    myDFPlayer.decVolume(); // Volume Down
    delay(500);
    state.volume -= 1;
  }

  if (command.substring(0, 2) == "so") {
    String song1 = command.substring(4, 8);
    int song = atoi((char *)&song1);
    myDFPlayer.stop();
    delay(500);
    myDFPlayer.playFromMP3Folder(song);
    delay(500);
  }
  
  if (command.substring(0, 2) == "bo") {
    String song1 = command.substring(3, 6);
    int song = atoi((char *)&song1);
    myDFPlayer.stop();
    delay(500);
    myDFPlayer.playFolder(02,song);
    delay(500);
  }

  if (command.substring(0, 2) == "ur") {
    String song1 = command.substring(3, 6);
    int song = atoi((char *)&song1);
    myDFPlayer.stop();
    delay(500);
    myDFPlayer.playFolder(03,song);
    delay(500);
  }
  
  // eq = 0-5
  if (command == "eq") {
    int eq = myDFPlayer.currentEQ();
    eq = eq + 1;
    if (eq == 6) eq = 0;
    state.eq = eq;
    myDFPlayer.EQSelect(eq);
    delay(500);
  }

  if (command.substring(0, 1) == "v") {
    int lastCommaIndex = command.lastIndexOf(',');
    String sentVolume = command.substring(lastCommaIndex + 1);
    state.volume = (atoi((char *)&sentVolume));
    myDFPlayer.volume(state.volume);
  }

  if (command.substring(0, 1) == "a") {
    int lastCommaIndex = command.lastIndexOf(',');
    String sentAlarm = command.substring(lastCommaIndex + 1);
    int lastColonIndex = sentAlarm.lastIndexOf(':');
    String sentHour = sentAlarm.substring(0, lastColonIndex);
    state.alarmHour = (atoi((char *)&sentHour));
    String sentMinute = sentAlarm.substring(lastColonIndex + 1);
    state.alarmMinute = (atoi((char *)&sentMinute));
    if (state.alarmHour == -1) {
      state.alarmSet = false;
    } else {
      state.alarmSet = true;
    }
  }

  // file upload handler

  if (command.substring(0, 4) == "upld") {
    state.filename = command.substring(5);
    deleteFile(SD, "/temp.txt");
  }

  if (command.substring(0, 4) == "comp") {
    deleteFile(SD, ("/" + state.filename).c_str());
    renameFile(SD, "/temp.txt", ("/" + state.filename).c_str());
    state.filename = "";
  }

  if (command.substring(0, 4) == "file") {
    String message = command.substring(5);
    appendFile(SD, "/temp.txt", message.c_str());
  }

  if (command.substring(0, 4) != "file") {
    if (myDFPlayer.isPlaying()) {
      state.playState = "PLAYING";
    } else {
      state.playState = "STOPPED";
    };
    state.currentTrack = myDFPlayer.currentSdTrack();
  }
}

void setMP3Status() {
  if (myDFPlayer.isPlaying()) {
    state.playState = "PLAYING";
  } else {
    state.playState = "STOPPED";
  };
  delay(100);
  state.currentTrack = myDFPlayer.currentSdTrack();
  delay(100);
  state.volume = myDFPlayer.currentVolume();
  delay(100);
  state.playMode = myDFPlayer.currentMode();
  delay(100);
}

void soundAlarm() {
  state.alarmSet = false;
  state.alarmMinute = -1;
  state.alarmHour = -1;
  myDFPlayer.stop();
  delay(500);
  myDFPlayer.volume(30);
  delay(500);
  state.volume = 30;
  delay(500);
  myDFPlayer.repeatFolder(1);
  delay(500);
}
#endif
