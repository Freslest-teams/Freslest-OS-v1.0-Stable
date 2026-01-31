#include "audio_manager.h"

// Active buzzer (has only '+'), GPIO17
bool soundEnabled = true;

#define BUZZER_PIN 17

static bool buzzing = false;
static uint32_t buzzUntil = 0;

static void buzzStart(uint32_t now, uint16_t ms){
  if(!soundEnabled) return;
  digitalWrite(BUZZER_PIN, HIGH);
  buzzing = true;
  buzzUntil = now + ms;
}

void audioInit(){
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  buzzing = false;
  buzzUntil = 0;
}

void audioUpdate(uint32_t now){
  if(buzzing && (int32_t)(now - buzzUntil) >= 0){
    digitalWrite(BUZZER_PIN, LOW);
    buzzing = false;
  }
}

void audioBoot(){     buzzStart(millis(), 120); }
void audioClick(){    buzzStart(millis(), 35);  }
void audioGame(){     buzzStart(millis(), 70);  }
void audioWin(){      buzzStart(millis(), 180); }
void audioGameOver(){ buzzStart(millis(), 260); }

void soundApply(){
  // uses current soundEnabled value
}
