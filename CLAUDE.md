# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Bare-metal firmware for Quansheng UV-K5/K6/5R radios. Runs on a DP32G030 (ARM Cortex-M0, 48MHz) with **60KB flash** and **16KB RAM**. This fork adds SI4732 multi-mode receiver and 2MB EEPROM support.

## Build Commands

```bash
make                          # Build with default options (SI4732 enabled)
make clean && make            # Full rebuild
make ENABLE_SI4732=0          # Build with stock BK1080 instead of SI4732
arm-none-eabi-size firmware   # Check flash/RAM usage (text+data must be < 61440)
```

**Toolchain:** arm-none-eabi-gcc 10.3.1 recommended. Other versions may produce binaries too large for flash.

**Output:** `firmware.bin` (raw) and `firmware.packed.bin` (with CRC, for flashing via [uvtools](https://egzumer.github.io/uvtools)).

**Docker:** `./compile-with-docker.sh` (Linux/Mac) or `compile-with-docker.bat` (Windows).

## Flash Size Constraint

This is the most critical constraint. Flash is 60KB (61,440 bytes). `text + data` from `arm-none-eabi-size` must stay under this. Current build uses ~60,672 bytes (~768 bytes free). Every line of code matters. LTO (`ENABLE_LTO=1`, default) helps significantly.

## Architecture

**Layers (bottom to top):**
- `start.S` / `init.c` - ARM vector table, BSS/DATA init
- `bsp/dp32g030/` - Auto-generated register definitions from `hardware/*.def` files
- `driver/` - Hardware drivers: BK4819 (SPI), SI4732 (I2C), EEPROM (I2C), ST7565 LCD, GPIO, keyboard
- `board.c` - Board-level init (GPIO, ADC, peripherals)
- `radio.c` - Radio state machine: frequency control, modulation, squelch, TX/RX
- `app/` - Application logic: FM radio, scanning, spectrum analyzer, DTMF, menus
- `ui/` - LCD rendering for each app screen
- `settings.c` - EEPROM layout and config persistence
- `audio.c` - Audio path routing, beep generation
- `scheduler.c` - Simple cooperative task scheduler

**Radio chips:**
- **BK4819** - Primary VHF/UHF transceiver (SPI, `driver/bk4819.c`)
- **SI4732** - HF/FM/AM/SSB receiver replacing BK1080 (I2C addr 0x22, `driver/si473x.c`). Enabled via `ENABLE_SI4732`.
- **BK1080** - Legacy FM broadcast receiver (I2C addr 0x80, `driver/bk1080.c`). Used when SI4732 disabled.

**I2C bus** is bit-banged on GPIOA pins 10 (SCL) and 11 (SDA), shared by EEPROM and SI4732/BK1080.

## EEPROM System

The EEPROM driver (`driver/eeprom.c`) supports 8KB to 256KB chips with 32-bit addressing and I2C bank switching (`0xA0 | ((address >> 16) << 1)`). Page-aligned writes with read-before-write optimization.

EEPROM type is set in `settings.c` `SETTINGS_InitEEPROM()` — hardcoded to type 7 (M24M02/256KB) when `ENABLE_SI4732` is on, type 2 (BL24C64/8KB) otherwise.

**EEPROM layout** (addresses in `settings.c`): channels at 0x0000+, settings at 0x0E40-0x0F50, calibration at 0x1EC0-0x1F90. SSB patch (15,832 bytes) stored at end of EEPROM.

`EEPROM_WriteBuffer(address, data, size)` takes 3 args — every call site must pass the size.

## Compile-Time Feature Flags

45+ `ENABLE_*` flags in `Makefile`. Key interactions:
- `ENABLE_SI4732=1` excludes BK1080 driver; all `BK1080_*` calls must be wrapped with `#ifdef ENABLE_SI4732` / `#else`
- `ENABLE_LTO=1` and `ENABLE_OVERLAY=1` are mutually exclusive
- `ENABLE_FMRADIO` gates FM app, UI, and chip driver inclusion
- Disabling features (AIRCOPY, NOAA, VOICE, ALARM) frees flash space

## SI4732 Integration Pattern

When adding code that touches BK1080 or FM functionality:
- Wrap BK1080 calls: `#ifdef ENABLE_SI4732` → SI4732 equivalent, `#else` → BK1080 call, `#endif`
- `BK1080_Mute(true/false)` → `SI47XX_SetVolume(0)` / `SI47XX_SetVolume(63)`
- `BK1080_Init0()` → `SI47XX_PowerDown()`
- `BK1080_GetFreqLoLimit(band)` → hardcoded `640` (64.0 MHz)
- `BK1080_GetFreqHiLimit(band)` → hardcoded `1080` (108.0 MHz)
- SI4732 detection at boot: `RADIO_HasSi()` probes BK1080 register, sets `hasSI` flag
- SSB patch validation: `SETTINGS_checkSSBPatch()` checks preamble at EEPROM end

## Key Conventions

- All structs use `__attribute__((packed))` for EEPROM compatibility
- No dynamic memory allocation — all static
- `SYSTEM_DelayMs()` for millisecond delays, `SYSTICK_DelayUs()` for microsecond
- GPIO pin names defined in `bsp/dp32g030/gpio.h` (e.g., `GPIOB_PIN_BK1080`)
- Register definitions auto-generated: edit `hardware/*.def`, not `bsp/*.h`

## License

Apache License 2.0. Original work by Dual Tachyon.
