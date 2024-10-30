/*
 * Author: Shenzhen Xinse Technology Co., Ltd. ---- sinoseen
 * Date: 2024-09-19
 * Description: ESP32CAM captures camera images and displays them on an LCD (ST7789) screen
 */

#include "lcd_driver.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "camera_pins.h"
#include "esp_camera.h"

static const char *TAG = "camera";

void camera_show(void *arg);

static esp_err_t init_camera(void)
{
    camera_config_t camera_config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,
        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,
        .xclk_freq_hz = 20 * 1000 * 1000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_RGB565,
        .frame_size = FRAMESIZE_240X240,
        .jpeg_quality = 12,
        .fb_count = 2,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY};

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera initialization failed with error: %s", esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}

void camera_show(void *arg)
{
    while (1)
    {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // 检查像素格式
        if (fb->format != PIXFORMAT_RGB565) {
            ESP_LOGE(TAG, "Pixel format is not RGB565");
            esp_camera_fb_return(fb);
            continue;
        }

        // 使用 draw16bitRGBBitmap 绘制整个图像
        draw16bitRGBBitmap(0, 0, (uint16_t *)fb->buf, fb->width, fb->height);
        esp_camera_fb_return(fb);
    }
}


void app_main(void)
{
    lcd_init();
    init_camera();
    LCD_Clear(RED);
    xTaskCreatePinnedToCore(camera_show, "camera_show", 8192, NULL, 10, NULL, 0);
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
