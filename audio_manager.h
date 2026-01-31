#pragma once
#include <Arduino.h>

extern bool soundEnabled;

void audioInit();
void audioUpdate(uint32_t now);

void audioBoot();
void audioClick();
void audioGame();
void audioWin();
void audioGameOver();

// Compatibility for settings menu
void soundApply();
