#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_io_expander.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t esp_io_expander_new_i2c_pcal6408a_8bit(
  i2c_master_bus_handle_t   bus_handle,
  uint32_t                  i2c_address,
  esp_io_expander_handle_t* handle);

enum esp_io_expander_pcal_6408a_8bit_address {
    ESP_IO_EXPANDER_I2C_PCAL6408A_ADDRESS_0 = 0x20, // 0b0100000
    ESP_IO_EXPANDER_I2C_PCAL6408A_ADDRESS_1 = 0x21  // 0b0100001
};

#ifdef __cplusplus
}
#endif