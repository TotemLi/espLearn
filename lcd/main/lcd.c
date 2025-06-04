#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "yingwu.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_err.h"
#include "esp_check.h"

static char *TAG = "lcd";

#define LCD_HOST SPI2_HOST
#define ROTATE_FRAME 30

#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL 1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL

#define PIN_NUM_MOSI 7
#define PIN_NUM_SCK 6
#define PIN_NUM_CS 17      // 片选信号，低电平使能
#define PIN_NUM_DC 15      // 寄存器/数据选择信号，低电平：寄存器，高电平：数据
#define PIN_NUM_RST 16     // 复位信号，低电平复位
#define PIN_NUM_BK_LIGHT 5 // 背光控制，高电平点亮，如无需控制则接3.3V常亮

#define LCD_H_RES 320
#define LCD_V_RES 240
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define LCD_DRAW_BUFF_HEIGHT (50)
#define LCD_DRAW_BUFF_DOUBLE (1)

LV_IMG_DECLARE(esp_logo);

static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_display_t *lvgl_disp = NULL;

static esp_err_t lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_NUM_BK_LIGHT,
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_SCK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_OFF_LEVEL));

    // Reset the display
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

    // Initialize LCD panel
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    // Turn on the screen
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // 反色
    // ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));

    // 交换xy轴
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));

    // 镜像
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));

    // 把buffer中的颜色数据绘制到屏幕上
    // ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_H_RES, LCD_V_RES, NULL));

    ESP_ERROR_CHECK(gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL));

    return ret;
}

void lcd_set_color(esp_lcd_panel_handle_t panel, uint16_t color)
{
    uint16_t *buffer = (uint16_t *)heap_caps_malloc(LCD_H_RES * sizeof(uint16_t), MALLOC_CAP_DMA);
    for (size_t i = 0; i < LCD_H_RES; i++)
    {
        buffer[i] = color;
    }
    for (int y = 0; y < LCD_V_RES; y++)
    {
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, 0, y, LCD_H_RES, y + 1, buffer));
    }
    free(buffer);
}

// 显示图片
void lcd_draw_pictrue(esp_lcd_panel_handle_t panel, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const unsigned char *gImage)
{
    uint16_t *buffer = (uint16_t *)heap_caps_malloc(x_end * sizeof(uint16_t), MALLOC_CAP_DMA);
    for (uint16_t y = y_start; y < y_end; y++)
    {
        for (uint16_t x = x_start; x < x_end; x++)
        {
            buffer[x] = (gImage[y * x_end * 2 + x * 2 + 1] << 8) | gImage[y * x_end * 2 + x * 2];
        }
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, x_start, y, x_end, y + 1, buffer));
    }
    free(buffer);
}

static esp_err_t lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 4096,       /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = LCD_H_RES * LCD_DRAW_BUFF_HEIGHT,
        .double_buffer = LCD_DRAW_BUFF_DOUBLE,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = true,
            .mirror_x = true,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = true,
        },
    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    return ESP_OK;
}

static void app_main_display(void)
{
    lv_obj_t *scr = lv_scr_act();
    lvgl_port_lock(0);

    lv_obj_t *img_logo = lv_img_create(scr);
    lv_img_set_src(img_logo, &esp_logo);
    // lv_obj_align(img_logo, LV_ALIGN_TOP_MID, 0, 20);

    lvgl_port_unlock();
}

void app_main(void)
{
    ESP_ERROR_CHECK(lcd_init());
    ESP_ERROR_CHECK(lvgl_init());

    lcd_set_color(panel_handle, 0x0000);

    app_main_display();

    // 全屏显示未红色
    // lcd_set_color(panel_handle, 0x0000);

    // 显示图片
    // lcd_draw_pictrue(panel_handle, 0, 0, 320, 240, gImage_yingwu);
}
