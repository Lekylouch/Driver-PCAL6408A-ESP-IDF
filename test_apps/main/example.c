/**
 * @file main.c
 * @brief Exemple d'utilisation des drivers PCAL6416A (16-bit) et PCAL6408A (8-bit) sur ESP32.
 * * Cet exemple montre comment initialiser le bus I2C et contrôler les broches d'un expander I/O.
 * * @note ARCHITECTURE MATÉRIELLE SPÉCIFIQUE :
 * Cet exemple illustre un cas d'usage avancé de "Power Gating" ou de Reset en cascade.
 * Le composant principal (PCAL6416A à l'adresse 0x20) contrôle l'alimentation ou le reset
 * du composant secondaire (PCAL6408A à l'adresse 0x21) via sa broche IO_EXPANDER_PIN_NUM_9.
 * * IL N'EST PAS NÉCESSAIRE D'UTILISER LE PCAL6408A pour faire fonctionner le PCAL6416A. 
 * Si votre projet ne comporte qu'un seul expander, vous pouvez ignorer la partie 
 * d'initialisation du second composant.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"

#include "esp_io_expander_pcal6416a_16bit.h"
#include "esp_io_expander_pcal6408a.h"

static const char *TAG = "PCAL_EXAMPLE";

/* --- Configuration du bus I2C --- */
#define I2C_MASTER_SCL_IO       22
#define I2C_MASTER_SDA_IO       21
#define I2C_MASTER_PORT         0

/* --- Adresses I2C des composants --- */
#define PCAL6416A_I2C_ADDR      0x20    // Adresse matérielle : A0=GND, A1=GND
#define PCAL6408A_I2C_ADDR      0x21    // Adresse matérielle différente obligatoire sur le même bus

/* --- Configuration GPIO --- */
#define RST  GPIO_NUM_0

void app_main(void)
{
    // --- Étape 0 : Reset matériel global (si applicable au système) ---
    gpio_reset_pin(RST);
    gpio_set_direction(RST, GPIO_MODE_OUTPUT);
    gpio_set_level(RST, 1);

    // --- Étape 1 : Initialisation du bus I2C (API ESP-IDF v5) ---
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

    // Déclaration des handles pour les deux composants
    esp_io_expander_handle_t io_expander = NULL; // Puce principale (16-bit)
    esp_io_expander_handle_t io_exp = NULL;      // Puce secondaire (8-bit) - Optionnelle
    
    // --- Étape 2 : Création de l'instance du premier expander (PCAL6416A) ---
    ESP_ERROR_CHECK(esp_io_expander_new_i2c_pcal6416a_16bit(
        i2c_bus,
        PCAL6416A_I2C_ADDR,
        &io_expander
    ));
    ESP_LOGI(TAG, "PCAL6416A initialized");

    // --- Étape 3 : Configuration des broches du PCAL6416A ---
    // On définit un masque pour les broches que l'on va faire clignoter (14 et 15)
    uint32_t mask_16bit_toggle = IO_EXPANDER_PIN_NUM_14 | IO_EXPANDER_PIN_NUM_15;
    
    // On configure en SORTIE la broche 9 (qui contrôle l'autre puce) ET les broches 14/15
    ESP_ERROR_CHECK(esp_io_expander_set_dir(
        io_expander,
        IO_EXPANDER_PIN_NUM_9 | mask_16bit_toggle,
        IO_EXPANDER_OUTPUT
    ));

    /* * SÉQUENCE DE DÉMARRAGE EN CASCADE :
     * Ici, le PCAL6416A contrôle matériellement l'allumage ou le reset du PCAL6408A 
     * via sa broche 9. On doit donc lever cette broche avant de tenter de communiquer 
     * avec le second composant sur le bus I2C.
     */
    esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_9, 1);
    
    // Pause FreeRTOS pour laisser le temps matériel au PCAL6408A de démarrer (boot hardware)
    vTaskDelay(pdMS_TO_TICKS(1000)); 

    // --- Étape 4 : Création de l'instance du composant secondaire (PCAL6408A) ---
    // Note : Cette étape échouerait si la broche 9 du premier composant n'était pas à 1.
    ESP_ERROR_CHECK(esp_io_expander_new_i2c_pcal6408a_8bit(
        i2c_bus, 
        PCAL6408A_I2C_ADDR, 
        &io_exp
    ));
    ESP_LOGI(TAG, "PCAL6408A initialized");

    // Configuration des directions pour le 8-bit expander (broches 4, 5 et 6 en sortie)
    uint32_t mask_8bit_toggle = IO_EXPANDER_PIN_NUM_4 | IO_EXPANDER_PIN_NUM_5 | IO_EXPANDER_PIN_NUM_6;
    
    ESP_ERROR_CHECK(esp_io_expander_set_dir(
        io_exp,
        mask_8bit_toggle,
        IO_EXPANDER_OUTPUT
    ));

    // --- Étape 5 : Boucle principale (Clignotement) ---
    bool level = false;
    while (1) {
        // Applique l'état aux broches 14 et 15 du composant principal
        ESP_ERROR_CHECK(esp_io_expander_set_level(
            io_expander,
            mask_16bit_toggle,
            level
        ));

        // Applique l'état aux broches 4, 5 et 6 du composant secondaire
        ESP_ERROR_CHECK(esp_io_expander_set_level(
            io_exp, 
            mask_8bit_toggle,
            level
        ));
        
        ESP_LOGI(TAG, "Pins toggled (State: %d)", level);
        level = !level; // Alterne l'état (0 -> 1 -> 0...)
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Pause de 1 seconde
    }
}