#include "EPD_Driver.h"

#define EDP_SPI_INSTANCE    0
static const nrf_drv_spi_t epd_spi = NRF_DRV_SPI_INSTANCE(EDP_SPI_INSTANCE);
static volatile bool epd_spi_xfer_done;

static uint8_t  epd_spi_tx_buf[256];
static uint8_t  epd_spi_rx_buf[256];

const unsigned char lut_full_update[] =
{
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22, 
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88, 
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51, 
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

const unsigned char lut_partial_update[] =
{
    0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void NRF_GPIO_Init(void)
{
    nrf_gpio_cfg_input(EPD_BUSY_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_output(EPD_DC_PIN);
    nrf_gpio_cfg_output(EPD_RST_PIN);
    nrf_gpio_pin_clear(EPD_DC_PIN);
    nrf_gpio_pin_clear(EPD_RST_PIN);
}

void NRF_GPIO_unInit(void)
{
    nrf_gpio_cfg_default(EPD_BUSY_PIN);
    nrf_gpio_cfg_default(EPD_DC_PIN);
    nrf_gpio_cfg_default(EPD_RST_PIN);
}

void epd_spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context)
{
    epd_spi_xfer_done = true;
}

void NRF_SPI_Init(void)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = EPD_CS_PIN;
    spi_config.mosi_pin = EPD_SDI_PIN;
    spi_config.sck_pin  = EPD_SCK_PIN;
    nrf_drv_spi_init(&epd_spi, &spi_config, epd_spi_event_handler, NULL);
}

void NRF_SPI_unInit(void)
{
    nrf_drv_spi_uninit(&epd_spi);
}

static void EPD_SpiTransfer(unsigned char data)
{
    epd_spi_tx_buf[0] = data;
    epd_spi_xfer_done = false;
    nrf_drv_spi_transfer(&epd_spi, epd_spi_tx_buf, 1, epd_spi_rx_buf, 0);
    while(!epd_spi_xfer_done);
}

static void EPD_Reset(void)
{
    nrf_gpio_pin_write(EPD_RST_PIN, 1);
    nrf_delay_ms(200);
    nrf_gpio_pin_write(EPD_RST_PIN, 0);
    nrf_delay_ms(200);
    nrf_gpio_pin_write(EPD_RST_PIN, 1);
    nrf_delay_ms(200);
}

static void EPD_SendCommand(unsigned char command) 
{
    nrf_gpio_pin_write(EPD_DC_PIN, LOW);
    EPD_SpiTransfer(command);
}

static void EPD_SendData(unsigned char data) 
{
    nrf_gpio_pin_write(EPD_DC_PIN, HIGH);
    EPD_SpiTransfer(data);
}

static void EPD_WaitUntilIdle(void)
{
    while(nrf_gpio_pin_read(EPD_BUSY_PIN) == 1) {
        nrf_delay_ms(100);
    }
}

static void EPD_SetWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{
    EPD_SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
    EPD_SendData((Xstart >> 3) & 0xFF);
    EPD_SendData((Xend >> 3) & 0xFF);

    EPD_SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
    EPD_SendData(Ystart & 0xFF);
    EPD_SendData((Ystart >> 8) & 0xFF);
    EPD_SendData(Yend & 0xFF);
    EPD_SendData((Yend >> 8) & 0xFF);
}

static void EPD_SetCursor(uint16_t Xstart, uint16_t Ystart)
{
    EPD_SendCommand(SET_RAM_X_ADDRESS_COUNTER);
    EPD_SendData((Xstart >> 3) & 0xFF);

    EPD_SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
    EPD_SendData(Ystart & 0xFF);
    EPD_SendData((Ystart >> 8) & 0xFF);

}

static void EPD_TurnOnDisplay(void)
{
    EPD_SendCommand(DISPLAY_UPDATE_CONTROL_2);
    EPD_SendData(0xC4);
    EPD_SendCommand(MASTER_ACTIVATION);
    EPD_SendCommand(TERMINATE_FRAME_READ_WRITE);

    EPD_WaitUntilIdle();
}

uint8_t EPD_Init(const unsigned char* lut)
{
    EPD_Reset();

    EPD_SendCommand(DRIVER_OUTPUT_CONTROL);
    EPD_SendData((EPD_HEIGHT - 1) & 0xFF);
    EPD_SendData(((EPD_HEIGHT - 1) >> 8) & 0xFF);
    EPD_SendData(0x00);
    EPD_SendCommand(BOOSTER_SOFT_START_CONTROL);
    EPD_SendData(0xD7);
    EPD_SendData(0xD6);
    EPD_SendData(0x9D);
    EPD_SendCommand(WRITE_VCOM_REGISTER);
    EPD_SendData(0xA8);
    EPD_SendCommand(SET_DUMMY_LINE_PERIOD);
    EPD_SendData(0x1A);
    EPD_SendCommand(SET_GATE_TIME);
    EPD_SendData(0x08);
    EPD_SendCommand(BORDER_WAVEFORM_CONTROL);
    EPD_SendData(0x03);
    EPD_SendCommand(DATA_ENTRY_MODE_SETTING);
    EPD_SendData(0x03);

    EPD_SendCommand(WRITE_LUT_REGISTER);
    for (uint16_t i = 0; i < 30; i++) {
        EPD_SendData(lut[i]);
    }
    return 0;
}

void EPD_Clear(void)
{
    uint16_t Width, Height;
    Width = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    Height = EPD_HEIGHT;
    EPD_SetWindows(0, 0, EPD_WIDTH, EPD_HEIGHT);
    for (uint16_t j = 0; j < Height; j++) {
        EPD_SetCursor(0, j);
        EPD_SendCommand(WRITE_RAM);
        for (uint16_t i = 0; i < Width; i++) {
            EPD_SendData(0XFF);
        }
    }
    EPD_TurnOnDisplay();
}

void EPD_Display(uint8_t *Image)
{
    uint16_t Width, Height;
    Width = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    Height = EPD_HEIGHT;

    uint32_t Addr = 0;
    EPD_SetWindows(0, 0, EPD_WIDTH, EPD_HEIGHT);
    for (uint16_t j = 0; j < Height; j++) {
        EPD_SetCursor(0, j);
        EPD_SendCommand(WRITE_RAM);
        for (uint16_t i = 0; i < Width; i++) {
            Addr = i + j * Width;
            EPD_SendData(Image[Addr]);
        }
    }
    EPD_TurnOnDisplay();
}

void EPD_Sleep(void)
{
    EPD_SendCommand(DEEP_SLEEP_MODE);
    EPD_SendData(0x01);
}
