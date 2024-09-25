#include "esp_adc_cal.h"
#include "driver/adc.h"
#include "esp_log.h"
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_sleep.h"

#define DEFAULT_VREF    4200  // Reference voltage in mV (adjust based on your board calibration)
#define ADC_WIDTH       ADC_WIDTH_BIT_13  // 12-bit resolution
#define ADC_ATTEN       ADC_ATTEN_DB_11   // 11dB attenuation (for higher voltage range)
#define ADC_CHANNEL     ADC2_CHANNEL_3    // GPIO14 is ADC1 Channel 6 on ESP32-S2

// Function to calculate the battery percentage
float calculate_battery_percentage(float battery_voltage) {
    float percentage = ((battery_voltage - 3.3) / (4.2 - 3.3)) * 100;
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    return percentage;
}

// Function to read the battery voltage from the ADC
float get_battery_voltage() {
    // Set up ADC configuration
    // adc2_config_width(ADC_WIDTH);
    adc2_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
    
    // Read raw ADC value
    int raw_out=0;
    int adc_reading = adc2_get_raw(ADC_CHANNEL,ADC_WIDTH,&raw_out);
    printf("Raw Batt: %d\n",raw_out);
    // Characterize ADC for the voltage conversion
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, &adc_chars);
    
    // Convert ADC reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
    
    // Convert to volts (and account for voltage divider)
    float battery_voltage = (float)voltage / 1000.0 * 2; // Multiply by 2 for voltage divider
    
    return battery_voltage;
}

void app_main(void) {
    // Initialize the ADC and log the battery voltage and percentage
    while (1) {
        float battery_voltage = get_battery_voltage();
        float battery_percentage = calculate_battery_percentage(battery_voltage);

        // Log the battery voltage and percentage
        ESP_LOGI("Battery", "Battery Voltage: %.2fV", battery_voltage);
        ESP_LOGI("Battery", "Battery Percentage: %.2f%%", battery_percentage);
        
        // Delay for a second before the next reading
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
