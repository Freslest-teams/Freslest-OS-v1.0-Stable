#include "gfx.h"

class LGFX_Impl : public lgfx::LGFX_Device {
  lgfx::Bus_SPI _bus;
  lgfx::Panel_ILI9488 _panel;

public:
  LGFX_Impl() {
    auto b = _bus.config();
    b.spi_host   = SPI2_HOST;
    b.spi_mode   = 0;
    b.freq_write = SPI_FREQ_WRITE;
    b.freq_read  = 20000000;
    b.use_lock   = true;
    b.dma_channel = SPI_DMA_CH_AUTO;
    b.pin_sclk   = PIN_TFT_SCK;
    b.pin_mosi   = PIN_TFT_MOSI;
    b.pin_miso   = PIN_TFT_MISO;
    b.pin_dc     = PIN_TFT_DC;
    _bus.config(b);
    _panel.setBus(&_bus);

    auto p = _panel.config();
    p.pin_cs       = PIN_TFT_CS;
    p.pin_rst      = PIN_TFT_RST;
    p.pin_busy     = -1;
    p.panel_width  = 320;
    p.panel_height = 480;
    p.offset_x     = 0;
    p.offset_y     = 0;
    p.readable     = false;
    p.rgb_order    = false;
    p.dlen_16bit   = false;
    p.bus_shared   = false;
    _panel.config(p);

    setPanel(&_panel);
  }
};

static LGFX_Impl _tft;
lgfx::LGFX_Device& tft = _tft;

static uint32_t _lastFrame = 0;

void gfxBegin() {
  tft.init();
  tft.setRotation(1);
  tft.setColorDepth(16);
  tft.setSwapBytes(true);
}

bool frameReady(uint32_t now) {
  if (now - _lastFrame < FRAME_MS) return false;
  _lastFrame = now;
  return true;
}
