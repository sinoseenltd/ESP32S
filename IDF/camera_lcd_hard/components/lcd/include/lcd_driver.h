#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdio.h>
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// SPI总线配置
#define SPI_MOSI GPIO_NUM_13 // MOSI引脚
#define SPI_MISO -1          // MISO引脚
#define SPI_SCLK GPIO_NUM_14 // SCLK引脚
#define LCD_DC GPIO_NUM_2    // DC控制引脚
#define LCD_RST GPIO_NUM_15
#define LCD_W 240
#define LCD_H 240
#define SPI_CS -1 

#define OLED_RST_Clr() gpio_set_level(LCD_RST, 0)
#define OLED_RST_Set() gpio_set_level(LCD_RST, 1)
#define OLED_DC_Clr() gpio_set_level(LCD_DC, 0)
#define OLED_DC_Set() gpio_set_level(LCD_DC, 1)

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40 // 棕色
#define BRRED 0XFC07 // 棕红色
#define GRAY 0X8430  // 灰色

esp_err_t spi_send_data8(spi_device_handle_t handle, uint8_t data);
esp_err_t spi_send_data16(spi_device_handle_t handle, uint16_t data);

esp_err_t lcd_init(void);
esp_err_t lcd_init(void);
void LCD_WR_REG(uint8_t dat);
void LCD_WR_DATA8(uint8_t dat);
void LCD_WR_DATA16(uint16_t dat);
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void LCD_Clear(uint16_t Color);
void draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h);

#endif 
