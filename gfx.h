#pragma once
#include <Arduino.h>
#include <LovyanGFX.hpp>

// ===== PINS =====
static constexpr int PIN_TFT_MOSI = 11;
static constexpr int PIN_TFT_SCK  = 12;
static constexpr int PIN_TFT_CS   = 10;
static constexpr int PIN_TFT_DC   = 9;
static constexpr int PIN_TFT_RST  = 8;
static constexpr int PIN_TFT_MISO = 13;   // MISO bağlı
static constexpr int PIN_TFT_BL   = -1;   // BL 3V3 -> PWM yok

// ===== PERF =====
static constexpr uint32_t SPI_FREQ_WRITE = 40000000; // FAST (S3 + short wires)
static constexpr uint32_t FRAME_MS = 16;             // ~60fps

// global display
extern lgfx::LGFX_Device& tft;

void gfxBegin();
bool frameReady(uint32_t now);
