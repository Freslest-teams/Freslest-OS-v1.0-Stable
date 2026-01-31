# Freslest-OS-v1.0-Stable

A **lightweight, stable, and tested** mini operating system for ESP32-S3.  
Designed to work with an ILI9488 SPI display, 6 buttons, and a buzzer.  
**Runs entirely on 3.3V.**

---

## 🔧 Hardware Requirements

- ESP32-S3  
- 3.5" ILI9488 SPI TFT  
- 6 push buttons  
- 1 buzzer  
- 1 × **10K resistor (for buzzer only)**  
- 3.3V power supply  

---

## 🖥️ TFT Pin Configuration (From Source Code)

```cpp
static constexpr int PIN_TFT_MOSI = 11;
static constexpr int PIN_TFT_SCK  = 12;
static constexpr int PIN_TFT_CS   = 10;
static constexpr int PIN_TFT_DC   = 9;
static constexpr int PIN_TFT_RST  = 8;
static constexpr int PIN_TFT_MISO = 13;
static constexpr int PIN_TFT_BL   = -1;   // Backlight connected to 3.3V
```

### TFT Wiring Table
```
VCC  → 3.3V
GND  → GND
CS   → GPIO 10
DC   → GPIO 9
RST  → GPIO 8
MOSI → GPIO 11
SCK  → GPIO 12
MISO → GPIO 13
BL   → 3.3V
```

📌 **Backlight is always ON (no PWM control).**

---

## 🎮 Button Pins

```
UP     → GPIO 4
DOWN   → GPIO 5
LEFT   → GPIO 6
RIGHT  → GPIO 7
OK     → GPIO 15
BACK   → GPIO 16
```

✔ No resistors required  
✔ Direct connection  

---

## 🔊 Buzzer

```
+ → GPIO 17 (with 10K resistor)
- → GND
```

📌 **10K resistor is required only for the buzzer.**

---

## ✅ Features

- ESP32-S3 optimized  
- ILI9488 SPI display support  
- Game menu system  
- Sound support  
- Flash reset  
- Stable performance  
- 3.3V operation  

---

## 🧪 Tested Hardware

✔ ESP32-S3 N16R8  
✔ ILI9488 SPI Display  
✔ Buttons working correctly  
✔ Buzzer tested  
✔ Long-run stable operation  

---

## ℹ️ Notes

This project has been fully tested.  
If you encounter any issues, please report them in **GitHub Issues**.

---

## 📦 Release v1.0

**First stable release**

✔ Stable core  
✔ Display + buttons + sound  
✔ Clean and lightweight  
✔ Ready to use 
