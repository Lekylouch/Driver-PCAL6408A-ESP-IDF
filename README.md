# ESP IO Expander — PCAL6416A (16-bit)

ESP-IDF driver for the **PCAL6416A** 16-bit I2C IO expander chip, based on the `esp_io_expander` component.

| Chip       | Interface | Component name                       | Datasheet |
|:----------:|:---------:|:------------------------------------:|:---------:|
| PCAL6416A  | I2C       | esp_io_expander_pcal6416a_16bit      | [NXP Datasheet](https://www.nxp.com/docs/en/data-sheet/PCAL6416A.pdf) |

---

## Features

- 16 GPIO pins organized in 2 banks (Port 0: pins 0–7, Port 1: pins 8–15)
- Configurable as input or output per pin
- Set output level (HIGH / LOW) per pin or group of pins
- Compatible with **ESP-IDF 5.x** (new `i2c_master_bus` API)
- I2C address configurable via hardware pins (A0, A1)

---

## Add to project

### With ESP-IDF Component Manager

Packages from this repository are uploaded to [Espressif's component service](https://components.espressif.com/).

Add the dependency via:

```bash
idf.py add-dependency esp_io_expander_pcal6416a_16bit==1.0.0
```

Or create/edit `idf_component.yml` in your component folder:

```yaml
dependencies:
  esp_io_expander_pcal6416a_16bit: "1.0.0"
```

More details in [Espressif's documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-component-manager.html).

---

## Wiring

| PCAL6416A Pin | ESP32 Pin (example) |
|:-------------:|:-------------------:|
| SDA           | GPIO 21             |
| SCL           | GPIO 22             |
| VCC           | 3.3V                |
| GND           | GND                 |
| A0, A1        | GND (address 0x20)  |

> **I2C addresses:** The PCAL6416A address depends on A0/A1 pins:
> - A0=GND, A1=GND → `0x20` (`ESP_IO_EXPANDER_I2C_PCAL6416A_ADDRESS_0`)
> - A0=VCC, A1=GND → `0x21`
> - A0=GND, A1=VCC → `0x22`
> - A0=VCC, A1=VCC → `0x23`

---

## Example usage (ESP-IDF 5.x)

This example uses the **new ESP-IDF v5 I2C master driver** (`i2c_master_bus`).

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_io_expander_pcal6416a_16bit.h"

static const char *TAG = "PCAL6416A_EXAMPLE";

#define I2C_MASTER_SCL_IO       22
#define I2C_MASTER_SDA_IO       21
#define I2C_MASTER_PORT         0
#define IO_EXPANDER_I2C_ADDR    0x20    // A0=GND, A1=GND

void app_main(void)
{
    // 1) Initialize I2C bus (ESP-IDF v5 API)
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c_bus));

    // 2) Create IO expander instance
    esp_io_expander_handle_t io_expander = NULL;
    ESP_ERROR_CHECK(esp_io_expander_new_i2c_pcal6416a_16bit(
        i2c_bus,
        IO_EXPANDER_I2C_ADDR,
        &io_expander
    ));
    ESP_LOGI(TAG, "IO expander initialized");

    // 3) Set pins 14 and 15 as outputs
    ESP_ERROR_CHECK(esp_io_expander_set_dir(
        io_expander,
        IO_EXPANDER_PIN_NUM_14 | IO_EXPANDER_PIN_NUM_15,
        IO_EXPANDER_OUTPUT
    ));

    // 4) Toggle pins every second
    bool level = false;
    while (1) {
        ESP_ERROR_CHECK(esp_io_expander_set_level(
            io_expander,
            IO_EXPANDER_PIN_NUM_14 | IO_EXPANDER_PIN_NUM_15,
            level
        ));
        ESP_LOGI(TAG, "Pin level = %d", level);
        level = !level;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

### Other useful API calls

Set pin direction as input:

```c
esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_0, IO_EXPANDER_INPUT);
```

Read pin level:

```c
uint32_t level_mask = 0;
esp_io_expander_get_level(io_expander, IO_EXPANDER_PIN_NUM_0, &level_mask);
```

Print all pins status to log:

```c
esp_io_expander_print_state(io_expander);
```

---

## Common issues

| Error | Cause | Fix |
|:------|:------|:----|
| `LoadProhibited` panic | Handle is NULL after failed init | Always check return value of `esp_io_expander_new_i2c_pcal6416a_16bit` |
| `this port has not been initialized` | Using legacy I2C API mixed with new API | Use only `i2c_new_master_bus` (ESP-IDF v5+) |
| Wrong I2C address | A0/A1 pins not wired correctly | Verify hardware wiring and use correct address constant |

---

---

## Using with PlatformIO (ESP-IDF framework)

If you are using **PlatformIO** with the **ESP-IDF framework** (not Arduino), the ESP-IDF Component Manager is not directly available via `platformio.ini`.

Instead, you need to create an `idf_component.yml` file inside your `src/` folder.

### Steps

1. In your PlatformIO project, create the file `src/idf_component.yml`.
2. Add the following content:

```yaml
dependencies:

  esp_io_expander_PCAL6416A_16bit:
    git: "https://github.com/dominicpoeschko/esp_io_expander_PCAL6416A_16bit.git"
```

3. Build your project (`platformio run`). PlatformIO will automatically download the component into `managed_components/` via the ESP-IDF Component Manager.

> **Note:** Make sure your `platformio.ini` uses `framework = espidf` and not `framework = arduino`.

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = espidf
```

> **Note:** The example code uses the new ESP-IDF v5 I2C master API (`driver/i2c_master.h`). Make sure you are using **ESP-IDF 5.x** or later.

## License

Apache-2.0 — see [LICENSE](./LICENSE) for details.
