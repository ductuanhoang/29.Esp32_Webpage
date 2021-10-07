/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_spiffs.h"

#include "../Common.h"
#include "../task/mqtt_task.h"
#include "../task/plan_task.h"

#include "../components/json_parser/json_parser.h"
#include "../components/esp32_wifi_manager/src/wifi_manager.h"
#include "../components/peripheral/user_timer.h"

#include "../user_driver/user_buttons.h"
#include "../user_driver/user_leds.h"
#include "../user_driver/user_vibration_motor.h"
/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
deive_data_t deive_data;
mqtt_config_t mqtt_config;

esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true};

void flash_file_init(void)
{
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            APP_LOGE("Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            APP_LOGE("Failed to find SPIFFS partition");
        }
        else
        {
            APP_LOGE("Failed to initialize SPIFFS (%d)", ret);
        }
    }
}

void flash_erase_all_partions(void)
{
    APP_LOGI("erase all nvs partions");
    nvs_flash_erase();
    APP_LOGI("erase all spiffs partions");
    esp_spiffs_format(conf.partition_label);
    esp_vfs_spiffs_unregister(&conf);
}

void cb_connection_ok(void *pvParameter)
{
    ip_event_got_ip_t *param = (ip_event_got_ip_t *)pvParameter;

    /* transform IP to human readable string */
    char str_ip[16];
    esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

    APP_LOGI("I have a connection and my IP is %s!", str_ip);
    deive_data.wifi_status = true;
    mqtt_task_start();
}

void app_main(void)
{
    APP_LOGI("--- APP_MAIN: Smart Chair Update 05/03/2021......");
    APP_LOGI("--- APP_MAIN: Free memory: %d bytes", esp_get_free_heap_size());
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    flash_file_init();
    ESP_ERROR_CHECK(ret);
    // load save param
    UserTimer_Init();

    deive_data.sensor.hammer_detect = 0;
    deive_data.sensor.vibration_level = 0;

    buttons_gpio_init();
    leds_gpio_init();
    vibration_init();
    /* start the wifi manager */
    wifi_manager_start();
    /* register a callback as an example to how you can integrate your code with the wifi manager */
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &cb_connection_ok);

    plan_task();
}
