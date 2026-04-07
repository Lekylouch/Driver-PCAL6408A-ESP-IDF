# ESP IO Expander — PCAL6416A (16-bit) & Advanced Cascade Setup

ESP-IDF driver for the **PCAL6416A** 16-bit I2C IO expander chip, based on the `esp_io_expander` component. 

This repository also includes an advanced example demonstrating how to use a primary PCAL6416A to control the power/reset state of a secondary **PCAL6408A** (8-bit) expander on the same I2C bus.

| Chip       | Interface | Component name                       | Datasheet |
|:----------:|:---------:|:------------------------------------:|:---------:|
| PCAL6416A  | I2C       | esp_io_expander_pcal6416a_16bit      | [NXP Datasheet](https://www.nxp.com/docs/en/data-sheet/PCAL6416A.pdf) |
| PCAL6408A  | I2C       | esp_io_expander_pcal6408a            | [NXP Datasheet](https://www.nxp.com/docs/en/data-sheet/PCAL6408A.pdf) |

---

## Features

- 16 GPIO pins organized in 2 banks (Port 0: pins 0–7, Port 1: pins 8–15) for the PCAL6416A.
- Configurable as input or output per pin.
- Set output level (HIGH / LOW) using specific bitmasks.
- Compatible with **ESP-IDF 5.x** (new `i2c_master_bus` API).
- I2C address configurable via hardware pins (A0, A1).
- **Advanced Power Gating:** Example provided for hardware-cascading multiple IO expanders.

---

## Add to project

### With ESP-IDF Component Manager

Packages from this repository are uploaded to [Espressif's component service](https://components.espressif.com/).

Add the dependencies via:

```bash
idf.py add-dependency esp_io_expander_pcal6416a_16bit==1.0.0
# Add the 8-bit version only if you are using the dual-chip advanced setup:
idf.py add-dependency esp_io_expander_pcal6408a==1.0.0
