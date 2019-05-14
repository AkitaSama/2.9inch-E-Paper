#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_drv_spi.h"

#define EPD_WIDTH       128
#define EPD_HEIGHT      296

#define EPD_SDI_PIN 	16
#define EPD_SCK_PIN 	17
#define EPD_CS_PIN 		18
#define EPD_DC_PIN 		19
#define EPD_RST_PIN		15
#define EPD_BUSY_PIN	20

#define LOW             0
#define HIGH            1

#define COLORED      	0
#define UNCOLORED    	1

#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define TERMINATE_FRAME_READ_WRITE                  0xFF

extern const unsigned char lut_full_update[];
extern const unsigned char lut_partial_update[];

void NRF_GPIO_Init(void);
void NRF_GPIO_unInit(void);
void NRF_SPI_Init(void);
void NRF_SPI_unInit(void);

uint8_t EPD_Init(const unsigned char* lut);
void EPD_Clear(void);
void EPD_Display(uint8_t *Image);
void EPD_Sleep(void);

#endif 
