#include <stdio.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_heap_caps.h"
#include "nvs_flash.h"

#include "lvgl.h"
#include "demos/lv_demos.h"
#include <esp_lvgl_port.h>
#include "lvgl_ui.h"
#include "bsp_spi.h"
#include "bsp_display.h"
#include "bsp_touch.h"
#include "bsp_i2c.h"
#include "bsp_wifi.h"
#include "bsp_sdcard.h"
#include "bsp_qmi8658.h"
#include "bsp_codec.h"
// #include "bsp_battery.h"
#include "bsp_power_manager.h"

#include "iot_button.h"
#include "button_gpio.h"

#define EXAMPLE_DISPLAY_ROTATION 0
#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 240

#define EXAMPLE_LCD_DRAW_BUFF_HEIGHT (50)
#define EXAMPLE_LCD_DRAW_BUFF_DOUBLE (1)

#define RECORD_MAX_SECONDS 4
#define RECORD_SAMPLE_RATE 16000
#define RECORD_BYTES_PER_SAMPLE 2
#define RECORD_CHANNELS 1
#define RECORD_CHUNK_SIZE 1024
#define RECORD_BUFFER_SIZE (RECORD_MAX_SECONDS * RECORD_SAMPLE_RATE * RECORD_BYTES_PER_SAMPLE * RECORD_CHANNELS)

static const char *TAG = "factory";

static button_handle_t boot_btn = NULL;
static button_handle_t pwr_btn = NULL;
static button_handle_t plus_btn = NULL;

i2c_master_bus_handle_t bus_handle;
esp_lcd_panel_io_handle_t io_handle = NULL;
esp_lcd_panel_handle_t panel_handle = NULL;
esp_lcd_touch_handle_t touch_handle = NULL;
lv_disp_drv_t disp_drv;
lv_indev_t *indev_touchpad;

lv_display_t *lvgl_disp = NULL;
lv_indev_t *lvgl_touch_indev = NULL;

SemaphoreHandle_t codec_recording_BinarySemaphore;
SemaphoreHandle_t codec_stop_BinarySemaphore;
static volatile bool codec_stop_requested = false;

void lv_port_init(void);

extern lv_obj_t *tileview;
static uint32_t col_id = 0;
extern SemaphoreHandle_t wifi_scanf_semaphore;
static void button_event_cb(void *arg, void *data)
{
    button_event_t event = iot_button_get_event((button_handle_t)arg);
    ESP_LOGI(TAG, "BOOT KEY %s", iot_button_get_event_str(event));
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    static bool long_press_flag = false;

    if (event == BUTTON_SINGLE_CLICK)
    {
        col_id = (col_id + 3) % 4;
        if (lvgl_port_lock(0))
        {
            lv_obj_set_tile_id(tileview, col_id, 0, LV_ANIM_ON);
            lvgl_port_unlock();
        }
    }
    else if (event == BUTTON_LONG_PRESS_START)
    {
        long_press_flag = true;
        codec_stop_requested = false;
        xSemaphoreGiveFromISR(codec_recording_BinarySemaphore, &xHigherPriorityTaskWoken);
    }
    else if (event == BUTTON_PRESS_END)
    {
        if (long_press_flag)
        {
            long_press_flag = false;
            codec_stop_requested = true;
            xSemaphoreGiveFromISR(codec_stop_BinarySemaphore, &xHigherPriorityTaskWoken);
        }
    }
}

static void plus_button_event_cb(void *arg, void *data)
{
    button_event_t event = iot_button_get_event((button_handle_t)arg);
    ESP_LOGI(TAG, "PLUS KEY %s", iot_button_get_event_str(event));
    if (event == BUTTON_LONG_PRESS_START)
    {
    }
    else if (event == BUTTON_SINGLE_CLICK)
    {
        col_id = (col_id + 1) % 4;
        if (lvgl_port_lock(0))
        {
            lv_obj_set_tile_id(tileview, col_id, 0, LV_ANIM_ON);
            lvgl_port_unlock();
        }
    }
}

static void button_init(void)
{
    button_config_t btn_cfg = {};
    button_gpio_config_t btn_gpio_cfg = {};
    btn_gpio_cfg.gpio_num = GPIO_NUM_9;
    btn_gpio_cfg.active_level = 0;
    // static button_handle_t btn = NULL;
    ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &boot_btn));

    iot_button_register_cb(boot_btn, BUTTON_LONG_PRESS_START, NULL, button_event_cb, NULL);
    // iot_button_register_cb(boot_btn, BUTTON_LONG_PRESS_HOLD, NULL, button_event_cb, NULL);
    // iot_button_register_cb(boot_btn, BUTTON_LONG_PRESS_UP, NULL, button_event_cb, NULL);
    iot_button_register_cb(boot_btn, BUTTON_PRESS_END, NULL, button_event_cb, NULL);

    btn_gpio_cfg.gpio_num = GPIO_NUM_18;
    ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &plus_btn));

    iot_button_register_cb(boot_btn, BUTTON_SINGLE_CLICK, NULL, button_event_cb, NULL);
    iot_button_register_cb(plus_btn, BUTTON_SINGLE_CLICK, NULL, plus_button_event_cb, NULL);
}

void es8311_test_task(void *arg)
{
    uint8_t *data = heap_caps_malloc(RECORD_BUFFER_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (data == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate %d bytes for audio buffer", RECORD_BUFFER_SIZE);
        vTaskDelete(NULL);
    }

    while (1)
    {
        if (xSemaphoreTake(codec_recording_BinarySemaphore, portMAX_DELAY) == pdTRUE)
        {
            while (xSemaphoreTake(codec_stop_BinarySemaphore, 0) == pdTRUE)
            {
            }

            size_t recorded_size = 0;
            bsp_codec_set_in_volume(90.0);

            while (recorded_size < RECORD_BUFFER_SIZE && !codec_stop_requested)
            {
                size_t remain_size = RECORD_BUFFER_SIZE - recorded_size;
                size_t chunk_size = remain_size > RECORD_CHUNK_SIZE ? RECORD_CHUNK_SIZE : remain_size;
                bsp_codec_recording(data + recorded_size, chunk_size);
                recorded_size += chunk_size;

                if (xSemaphoreTake(codec_stop_BinarySemaphore, 0) == pdTRUE)
                {
                    codec_stop_requested = true;
                }
            }

            bsp_codec_set_in_volume(0.0);
            bsp_codec_set_out_volume(90.0);

            size_t played_size = 0;
            while (played_size < recorded_size)
            {
                size_t remain_size = recorded_size - played_size;
                size_t chunk_size = remain_size > RECORD_CHUNK_SIZE ? RECORD_CHUNK_SIZE : remain_size;
                bsp_codec_playing(data + played_size, chunk_size);
                played_size += chunk_size;
            }

            bsp_codec_set_out_volume(0.0);
        }
    }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    i2c_master_bus_handle_t i2c_bus_handle;
    i2c_bus_handle = bsp_i2c_init();
    bsp_spi_init();

    codec_recording_BinarySemaphore = xSemaphoreCreateBinary();
    codec_stop_BinarySemaphore = xSemaphoreCreateBinary();

    bsp_display_brightness_init();
    bsp_display_set_brightness(100);

    bsp_power_manager_init();
    bsp_qmi8658_init(i2c_bus_handle);
    bsp_sdcard_init();
    bsp_codec_init(i2c_bus_handle);

    bsp_wifi_init("WSTEST", "waveshare0755");//wifi SSID ,wifi password
    button_init();

    bsp_display_init(&io_handle, &panel_handle, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUFF_HEIGHT);
    bsp_touch_init(&touch_handle, i2c_bus_handle, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES, EXAMPLE_DISPLAY_ROTATION, NULL);
    lv_port_init();
    xTaskCreate(es8311_test_task, "es8311_test_task", 1024 * 4, NULL, 5, NULL);

    if (lvgl_port_lock(0))
    {
        lvgl_ui_init();
        // lv_demo_benchmark();
        // lv_demo_music();
        // lv_demo_widgets();
        lvgl_port_unlock();
    }
}

void lv_port_init(void)
{
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&port_cfg);
    ESP_LOGI(TAG, "Adding LCD screen");
    lvgl_port_display_cfg_t display_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .control_handle = NULL,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUFF_HEIGHT,
        .double_buffer = true,
        .trans_size = 0,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
        .flags = {
            .buff_dma = 1,
            .buff_spiram = 0,
            .sw_rotate = 1,
            .full_refresh = 0,
            .direct_mode = 0,
        },
    };
    lvgl_disp = lvgl_port_add_disp(&display_cfg);
#if EXAMPLE_DISPLAY_ROTATION == 90
    lv_disp_set_rotation(lvgl_disp, LV_DISP_ROT_90);
#elif EXAMPLE_DISPLAY_ROTATION == 180
    lv_disp_set_rotation(lvgl_disp, LV_DISP_ROT_180);
#elif EXAMPLE_DISPLAY_ROTATION == 270
    lv_disp_set_rotation(lvgl_disp, LV_DISP_ROT_270);
#else
    lv_disp_set_rotation(lvgl_disp, LV_DISP_ROT_NONE);
#endif

    if (touch_handle)
    {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lvgl_disp,
            .handle = touch_handle,
        };
        lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);
    }
}
