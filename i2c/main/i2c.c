#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_log.h"

#define SCL_IO_PIN 10
#define SDA_IO_PIN 11

const static char *TAG = "i2c";

void app_main(void)
{
    esp_err_t err;

    // 配置主机
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = SCL_IO_PIN,
        .sda_io_num = SDA_IO_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));

    // 主机探测
    err = i2c_master_probe(bus_handle, 0x68, -1);
    ESP_LOGI(TAG, "i2c_master_probe result: %s", esp_err_to_name(err));

    // // 配置从机
    // i2c_device_config_t dev_cfg = {
    //     .dev_addr_length = I2C_ADDR_BIT_7,
    //     .device_address = 0x68,
    //     .scl_speed_hz = 100000,
    // };
    // i2c_master_dev_handle_t dev_handle;
    // ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    // // 主机写入
    // // 指定地址+写入数据
    // uint8_t data_wr;
    // i2c_master_transmit(dev_handle, &data_wr, sizeof(data_wr), -1);

    // // 主机读取
    // // 指定地址+接收读取到的数据
    // uint8_t buf[20] = {0x20};
    // uint8_t buffer[2];
    // i2c_master_transmit_receive(dev_handle, buf, sizeof(buf), buffer, 2, -1);

    // // 卸载设备
    // i2c_master_bus_rm_device(dev_handle);

    // // 卸载主机
    // i2c_del_master_bus(bus_handle);
}
