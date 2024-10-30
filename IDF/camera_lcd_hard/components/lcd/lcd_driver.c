#include "lcd_driver.h"

spi_device_handle_t handle;

void LCD_WR_REG(uint8_t dat)
{
    OLED_DC_Clr();
    spi_send_data8(handle, dat);
}

void LCD_WR_DATA8(uint8_t dat)
{
    OLED_DC_Set();
    spi_send_data8(handle, dat);
}

void LCD_WR_DATA16(uint16_t dat)
{
    OLED_DC_Set();
    spi_send_data8(handle, dat >> 8);
    spi_send_data8(handle, dat);
}

esp_err_t lcd_init(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MISO,
        .sclk_io_num = SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_IOMUX_PINS, // SPICOMMON_BUSFLAG_GPIO_PINS
        .max_transfer_sz = 240 * 40 * sizeof(uint16_t)};

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 40 * 1000 * 1000, // SPI频率SPI_MASTER_FREQ_80M
        .mode = 3,                          // SPI模式
        .spics_io_num = SPI_CS,             // CS引脚
        .queue_size = 3,                    // 发送队列大小
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &handle));

    gpio_config_t io_conf =
        {
            .pin_bit_mask = (1ULL << LCD_RST) | (1ULL << LCD_DC),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = 0};
    gpio_config(&io_conf);

    OLED_RST_Clr();
    vTaskDelay(pdMS_TO_TICKS(20));
    OLED_RST_Set();
    vTaskDelay(pdMS_TO_TICKS(20));

    LCD_WR_REG(0x36);
    LCD_WR_DATA8(0x00);

    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x33);

    LCD_WR_REG(0xB7);
    LCD_WR_DATA8(0x35);

    LCD_WR_REG(0xBB);
    LCD_WR_DATA8(0x19);

    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0x2C);

    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x01);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x12);

    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x20);

    LCD_WR_REG(0xC6);
    LCD_WR_DATA8(0x00);

    LCD_WR_REG(0xD0);
    LCD_WR_DATA8(0xA4);
    LCD_WR_DATA8(0xA1);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x2B);
    LCD_WR_DATA8(0x3F);
    LCD_WR_DATA8(0x54);
    LCD_WR_DATA8(0x4C);
    LCD_WR_DATA8(0x18);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x23);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x2C);
    LCD_WR_DATA8(0x3F);
    LCD_WR_DATA8(0x44);
    LCD_WR_DATA8(0x51);
    LCD_WR_DATA8(0x2F);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x20);
    LCD_WR_DATA8(0x23);

    LCD_WR_REG(0x21);
    LCD_WR_REG(0x11);
    LCD_WR_REG(0x29);
    return ESP_OK;
}

void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_WR_REG(0x2a);
    LCD_WR_DATA16(x1);
    LCD_WR_DATA16(x2);
    LCD_WR_REG(0x2b);
    LCD_WR_DATA16(y1);
    LCD_WR_DATA16(y2);
    LCD_WR_REG(0x2c);
}

void LCD_Clear(uint16_t Color)
{
    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
    OLED_DC_Set();

    const size_t max_pixels_per_transfer = 8192;
    size_t total_pixels = LCD_W * LCD_H;
    size_t pixels_left = total_pixels;

    uint16_t *color_buffer = heap_caps_malloc(max_pixels_per_transfer * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (color_buffer == NULL)
        return;

    for (size_t i = 0; i < max_pixels_per_transfer; i++)
    {
        color_buffer[i] = Color;
    }

    while (pixels_left > 0)
    {
        size_t pixels_to_send = (pixels_left > max_pixels_per_transfer) ? max_pixels_per_transfer : pixels_left;
        size_t len = pixels_to_send * 2;

        spi_transaction_t t = {
            .length = len * 8,
            .tx_buffer = color_buffer,
            .user = (void *)1,
        };

        ESP_ERROR_CHECK(spi_device_transmit(handle, &t));

        pixels_left -= pixels_to_send;
    }

    free(color_buffer);
}

esp_err_t spi_send_data8(spi_device_handle_t handle, uint8_t data)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.tx_buffer = &data;
    t.length = 8;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &t));
    return ESP_OK;
}

esp_err_t spi_send_data16(spi_device_handle_t handle, uint16_t data)
{
    spi_transaction_t t;
    uint16_t buf = data;
    memset(&t, 0, sizeof(t));
    
    t.tx_buffer = &buf; 
    t.length = 8 * sizeof(data); 

    ESP_ERROR_CHECK(spi_device_transmit(handle, &t));
    return ESP_OK;
}

void draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h)
{
    LCD_Address_Set(x, y, x + w - 1, y + h - 1);
    OLED_DC_Set();

    const size_t max_pixels_per_transfer = 8192;
    size_t pixels_left = w * h;
    uint16_t *current_pixel = bitmap;

    while (pixels_left > 0)
    {
        size_t pixels_to_send = (pixels_left > max_pixels_per_transfer) ? max_pixels_per_transfer : pixels_left;
        size_t len = pixels_to_send * 2; 

        spi_transaction_t t = {
            .length = len * 8,
            .tx_buffer = current_pixel,
            .user = (void *)1,
        };

        ESP_ERROR_CHECK(spi_device_transmit(handle, &t));

        current_pixel += pixels_to_send;
        pixels_left -= pixels_to_send;
    }
}
