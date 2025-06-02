#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SCL_IO_PIN 10
#define SDA_IO_PIN 11

const static char *TAG = "i2c";

#define MPU6050_SMPLRT_DIV 0x19
#define MPU6050_CONFIG 0x1A
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C

#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_ACCEL_XOUT_L 0x3C
#define MPU6050_ACCEL_YOUT_H 0x3D
#define MPU6050_ACCEL_YOUT_L 0x3E
#define MPU6050_ACCEL_ZOUT_H 0x3F
#define MPU6050_ACCEL_ZOUT_L 0x40
#define MPU6050_TEMP_OUT_H 0x41
#define MPU6050_TEMP_OUT_L 0x42
#define MPU6050_GYRO_XOUT_H 0x43
#define MPU6050_GYRO_XOUT_L 0x44
#define MPU6050_GYRO_YOUT_H 0x45
#define MPU6050_GYRO_YOUT_L 0x46
#define MPU6050_GYRO_ZOUT_H 0x47
#define MPU6050_GYRO_ZOUT_L 0x48

#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_PWR_MGMT_2 0x6C
#define MPU6050_WHO_AM_I 0x75

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

    // // 主机探测
    // err = i2c_master_probe(bus_handle, 0x68, -1);
    // ESP_LOGI(TAG, "i2c_master_probe result: %s", esp_err_to_name(err));

    // 配置从机
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_7,
        .device_address = 0x68,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    uint8_t init_buf[2];
    // 电源管理寄存器1，取消睡眠模式，选择时钟源为X轴陀螺仪
    init_buf[0] = MPU6050_PWR_MGMT_1;
    init_buf[1] = 0x01;
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, init_buf, sizeof(init_buf), -1));
    // 电源管理寄存器2，保持默认值0，所有轴均不待机
    init_buf[0] = MPU6050_PWR_MGMT_2;
    init_buf[1] = 0x00;
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, init_buf, sizeof(init_buf), -1));
    // 采样率分频寄存器，配置采样率
    init_buf[0] = MPU6050_SMPLRT_DIV;
    init_buf[1] = 0x09;
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, init_buf, sizeof(init_buf), -1));
    // 配置寄存器，配置DLPF
    init_buf[0] = MPU6050_CONFIG;
    init_buf[1] = 0x06;
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, init_buf, sizeof(init_buf), -1));
    // 陀螺仪配置寄存器，选择满量程为±2000°/s
    init_buf[0] = MPU6050_GYRO_CONFIG;
    init_buf[1] = 0x18;
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, init_buf, sizeof(init_buf), -1));
    // 加速度计配置寄存器，选择满量程为±16g
    init_buf[0] = MPU6050_ACCEL_CONFIG;
    init_buf[1] = 0x18;
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, init_buf, sizeof(init_buf), -1));

    uint8_t who_buf[] = {MPU6050_WHO_AM_I};
    uint8_t who_result[1] = {0};
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, who_buf, sizeof(who_buf), who_result, sizeof(who_result), -1));
    ESP_LOGI(TAG, "i2c_master_transmit_receive who am i: 0x%02X", who_result[0]);

    while (1)
    {
        uint8_t buf[] = {MPU6050_ACCEL_XOUT_H};
        uint8_t buffer[6] = {0};
        ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, buf, sizeof(buf), buffer, sizeof(buffer), -1));
        for (int i = 0; i < sizeof(buffer); i++)
        {
            ESP_LOGI(TAG, "i2c_master_transmit_receive buffer i: %d, data: %d", i, buffer[i]);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // // 卸载设备
    // i2c_master_bus_rm_device(dev_handle);

    // // 卸载主机
    // i2c_del_master_bus(bus_handle);
}
