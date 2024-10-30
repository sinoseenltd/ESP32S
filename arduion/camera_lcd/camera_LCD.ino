/*
 * Author: Shenzhen Xinse Technology Co., Ltd. ---- sinoseen
 * Date: 2024-09-12
 * Description: ESP32CAM captures camera images and displays them on an LCD (ST7789) screen
 */

#include "esp_camera.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

Arduino_DataBus *bus = new Arduino_ESP32SPI(2 /* DC */, -1 /* CS */, 13 /* SCK */, 15 /* MOSI */, -1 /* MISO */, HSPI /* spi_num */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 14 /* RST */, 0 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */);

camera_fb_t *fb = NULL; // Capture buffer

void camera_init(void);
void camera_show(void);

void setup()
{
  Serial.begin(115200);
  gfx->begin();

  Serial.setDebugOutput(true);
  Serial.println();

  gfx->fillScreen(BLACK);

  camera_init();
}

void loop()
{
  camera_show(); // Display the image on the LCD
}

void camera_init(void)
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_240X240;
  config.pixel_format = PIXFORMAT_RGB565;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 2;
  // Camera initialization
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1); // Vertical flip
  s->set_hmirror(s, 1); // Horizontal flip
  // s->set_brightness(s, 1); // Slightly increase brightness
}

void camera_show(void)
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }

  // Check pixel format
  if (fb->format != PIXFORMAT_RGB565)
  {
    Serial.println("Pixel format is not RGB565");
    esp_camera_fb_return(fb);
    return;
  }

  // Byte swapping, use as needed based on board memory endianness
  uint16_t *rgb_buffer = (uint16_t *)fb->buf;
  for (int i = 0; i < fb->width * fb->height; i++)
  {
    rgb_buffer[i] = (rgb_buffer[i] << 8) | (rgb_buffer[i] >> 8);
  }

  // Use draw16bitRGBBitmap to draw the entire image at once
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)fb->buf, fb->width, fb->height);

  esp_camera_fb_return(fb);
}