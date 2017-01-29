#include <stdbool.h>
#include <stdint.h>

#include "led.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "nrf_soc.h"
#include "nrf.h"
#include "CoughDetect_v3.h"
#include "simple_timer.h"
#include "app_timer.h"
#include "simple_ble.h"
#include "simple_adv.h"
#include "spi_master2.h"
#include "fm25vn10.h"
#include "nrf_drv_gpiote.h"
#include "nrf_soc.h"
#include "nrf_gpio.h"
#include "rv3049.h"

#define UMICH_COMPANY_IDENTIFIER 0x02E0
#define DEVICE_NAME "CoughDet"
#define PHYSWEB URL "goo.gl/fill_in_something"
#define DATA_SIZE 10
#define DATA_LOG_SIZE 6000

static simple_ble_service_t cough_service = {
    .uuid128 = {{0x9c, 0x16, 0x35, 0x86, 0x30, 0x2b, 0x40, 0xb9, 0x89, 0x1f, 0x8a, 0x4c, 0x02, 0x19, 0x2c, 0xc5}}
};

static simple_ble_config_t ble_config = {
    .platform_id       = 0xe0,              // used as 4th octect in device BLE address
    .device_id         = DEVICE_ID_DEFAULT,
    .adv_name          = DEVICE_NAME,
    .adv_interval      = MSEC_TO_UNITS(500, UNIT_0_625_MS),
    .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS),
    .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS)
};


static SPIConfig_t fram_spi =  {
                                .Config.Fields.BitOrder = SPI_BITORDER_MSB_LSB,
                                .Config.Fields.Mode     = SPI_MODE3,
                                .Frequency              = SPI_FREQ_2MBPS,
                                .Pin_SCK                = FRAM_SCK_2,
                                .Pin_MOSI               = FRAM_MOSI_2,
                                .Pin_MISO               = FRAM_MISO,
                                .Pin_CSN                = DUMMY_CS
                              };

static SPIConfig_t nuc_rtc_spi = {
                                .Config.Fields.BitOrder = SPI_BITORDER_MSB_LSB,
                                .Config.Fields.Mode     = SPI_MODE1,
                                .Frequency              = SPI_FREQ_500KBPS,
                                .Pin_SCK                = NUC_SPI_SCK_PIN,
                                .Pin_MOSI               = NUC_SPI_MOSI_PIN,
                                .Pin_MISO               = NUC_SPI_MISO_PIN,
                                .Pin_CSN                = DUMMY_CS
                              };

static SPIConfig_t adc_spi = {
                                .Config.Fields.BitOrder = SPI_BITORDER_MSB_LSB,
                                .Config.Fields.Mode     = SPI_MODE3,
                                .Frequency              = SPI_FREQ_2MBPS,
                                .Pin_SCK                = ADC_SCK,
                                .Pin_MOSI               = DUMMY_CS,
                                .Pin_MISO               = ADC_MISO,
                                .Pin_CSN                = ADC_CS
};
nrf_drv_gpiote_in_config_t comp1_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);

void timer_handler(void *p_context) {
    //led_toggle(LED0);
}

void comparator_handler(void *p_context) {

}

int main(void) {
    simple_ble_init(&ble_config);
    configure_platform_pins();
    simple_adv_only_name();
    //simple_timer_start(5000, timer_handler);
    fram_init(SPI1, &fram_spi, FRAM_CS);
    fram_sleep();
    spi_master_disable(SPI1);

    comp1_config.hi_accuracy = false;
    comp1_config.is_watcher = false;
    comp1_config.pull = NRF_GPIO_PIN_NOPULL;

    if(!nrf_drv_gpiote_is_init()) {nrf_drv_gpiote_init();}
    nrf_drv_gpiote_in_init(COMP1, &comp1_config, comparator_handler);

    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);

    rv3049_init(SPI0, &nuc_rtc_spi, NUC_RV3049_CS);
    spi_master_disable(SPI0);
    //led_on(LED0);

    while (1) {
        sd_app_evt_wait();
    }
}
