/*
 * mqtt_task.c
 *
 *  Created on: Jan 7, 2021
 *      Author: ductu
 */
/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "mqtt_task.h"
#include "../../components/json_parser/json_parser.h"
#include "../../Common.h"
#include "../../main.h"

#include "esp_wifi.h"
#include "esp_system.h"

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"

#include "esp_http_client.h"

/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
static const char *TAG = "MQTTS_EXAMPLE";
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
const char *client_cert_pem_start = "-----BEGIN CERTIFICATE-----\n"
"MIIDWTCCAkGgAwIBAgIUVIY20+l8oZIy0IWLak9ArhEdSJ0wDQYJKoZIhvcNAQEL\n"
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIxMDkyMDE2NTEx\n"
"NloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKb6WqdWX+JqAD9FGlQ6\n"
"d75vSKYOlc3pAaNIoCq3jola5kSqpUZvww3Uvooywh1V48n4qVZwN6lczGxXE1+a\n"
"3tMFAzEa+gZ17Ktz05gTKQjV8v8IJywiIHkmFVQWrtzrDvKbc4LHd/EMIpsfM8pr\n"
"1WZhZaKpl9GRcqIhPoRavQzddkd/MMNw8U+7ZAKBX1onlOSBsRC6fDIGAiqfRC8w\n"
"4jkJEVo/BUeWZvdnihCyTU6cSiZEKlsefu2MBOux/xqAbgell8/QFRewLRt3p3ir\n"
"4AHn9d/7oUNZr8fCZBbC0P1jMtP1G1Z6o0iMuA6j5iuUOx60A+cLdwirCHhXsuwr\n"
"1xkCAwEAAaNgMF4wHwYDVR0jBBgwFoAUAmvE5PmvqX2mdLKdhKRnM+aD76UwHQYD\n"
"VR0OBBYEFCHGn2PnwzbtrWUX8kTVBdNJ9cg/MAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBpitHITEGRYWfKgdYG7ZxT97bv\n"
"yFTYLE8G6ORDJ2TKrJ234yp3nG9YxJubbaZKyWtEgtxLkO5ziqpdqlaeN0jCdO7A\n"
"GbsnCN4qO0Hyuys9oJ7/c5lA6PElH+eYK2CmT3bx/BY093O11y5kTy2ORLMeXnFN\n"
"qIbdZHB0Bg6V2s5bMxy95y7b2qOmsqgpkVB6VwYosB7bwpe6iajrDAGLGvUB5r1h\n"
"+z3ROoxlMyxW67DiYigsh0PNRspwENpjOfkVt+oKyP159iYWz9R8jC35P24uOEV7\n"
"LBmJRsJZuAqcLi8Dxd9eANRnFCSDkOf//BiA/QMQPZbFY9SYDXrKza/SYCzx\n"
"-----END CERTIFICATE-----\n";


const char *client_key_pem_start = "-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEowIBAAKCAQEApvpap1Zf4moAP0UaVDp3vm9Ipg6VzekBo0igKreOiVrmRKql\n"
"Rm/DDdS+ijLCHVXjyfipVnA3qVzMbFcTX5re0wUDMRr6BnXsq3PTmBMpCNXy/wgn\n"
"LCIgeSYVVBau3OsO8ptzgsd38Qwimx8zymvVZmFloqmX0ZFyoiE+hFq9DN12R38w\n"
"w3DxT7tkAoFfWieU5IGxELp8MgYCKp9ELzDiOQkRWj8FR5Zm92eKELJNTpxKJkQq\n"
"Wx5+7YwE67H/GoBuB6WXz9AVF7AtG3eneKvgAef13/uhQ1mvx8JkFsLQ/WMy0/Ub\n"
"VnqjSIy4DqPmK5Q7HrQD5wt3CKsIeFey7CvXGQIDAQABAoIBAQCVEbSU59u6On/1\n"
"/C9BOuFkNd1ZwnOi3H4F0/SJrk2l+mzQqLfcZjJwyplAr4f0wJUX3tLuxEgs/xfR\n"
"MPuuwohjQ3pSAI9t7SFY/5LLlW9w3/CMFi9Ci0UKYyFvbMC3oXI6zOUWwBUoMXlD\n"
"m1uVPyfIuvMNgPJ0ubDpZVx2tLtKG4pSVswTuUn7vbZOREIpgpWjRmjmET+5DI/r\n"
"Dzf1S6duICoDsdxTxSqwWm3KXkBlR0h+xNzNKTk30LWKMfhrJsn2WODTjcnT98Ut\n"
"CCkjkwUWdfS9X70KV3T4jBc94VVRc4D0tEhC5YodN85b4iBJb67330mL1a79N5SA\n"
"hCwM3TuxAoGBAN1mO5wMp5EtXsyquz+N281Qhix59qlL5z0KqkQ3zv0dFMxkWzGF\n"
"Y9sA2fFWykiHPW0Xpkm08srL/urhEcIouzniT6b1PobSLJRaFzG2w/92cgaSVbeN\n"
"AyFlyaf1GXPDxdnzshUEDOUqxj+9Z/yAm08NRI4XboKPHSnwvAGHqAB7AoGBAMES\n"
"0wz+T5KNeXpTKOKseW2gtMZJ9RD30DekK4XEFgnj7rQCs63uDPWdhCLdc9Q/hbo4\n"
"KQT05x7ww9d4GjJ9qpQWD8eHqxFD8wst3XdnXitgVxmXPc0a5tg9mRsEn5oDsBBL\n"
"eRD3eyUIZabEx4HLzlaan204riWf+iDchjuwHhR7AoGAJOaEKuclCppgQYZ3PQl8\n"
"yASSyFjvlpnzCYZ8iHAwzJkLPWHAnUlZMkg7CvFnN51qzILzkTdafBhx/V3T4uN9\n"
"CRHCKtaXPEtNaNB3Ky8GDc8FBDlhlf0nt2pKqPa97kCD+maWmNAbAhsV/cuoyDn7\n"
"KLrSYUhgf9wwhSv9edfNVX8CgYB8Pt7uF6GhT8WCwI0pAZDKyYZLrEA2gSLPC6iu\n"
"VjdQegucb7itDunsDUeBID1vsskdwxIyjF9G++0rdbB2GYRx85iH8HhXjgCTixpf\n"
"wndJNqAWactteAhh05H5XwpOFF1yvv7Tuk7W0AaKQUuZ7UuFU8+Qe88li0Ntbjw7\n"
"oORajQKBgBW8CkKALB8WkqexZOhOXPNfiUAn6aMLsIO+uE/Iv2QFVlWgUr5LAb5T\n"
"CgaFu6nJe47K+9O35lT2+0n3sZI+Ie5jsLmcv849tbKsgZcKG0UKpw7JcaTFetw0\n"
"V7zGh7IybmUGKMEOWWAiJ7AFUNY8GMSE//3JJiHwsXRxkij0V7SM\n"
"-----END RSA PRIVATE KEY-----\n";

const char *server_cert_pem_start = "-----BEGIN CERTIFICATE-----\n"
                                    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
                                    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
                                    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
                                    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
                                    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
                                    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
                                    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
                                    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
                                    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
                                    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
                                    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
                                    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
                                    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
                                    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
                                    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
                                    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
                                    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
                                    "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
                                    "-----END CERTIFICATE-----\n";

static esp_mqtt_client_handle_t client;

static void mqtt_app_start(void);
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);
static void mqtt_send_task(void *pvParameters);
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
static void error_message_send(void);
static char *wifi_get_mac(void);
/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void mqtt_task_start(void)
{
    mqtt_app_start(); // init mqtt connect to AWS
    xTaskCreatePinnedToCore(mqtt_send_task, "mqtt_send_task", 3 * 1024, NULL, 6 | portPRIVILEGE_BIT, NULL, 1);
}
/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/
/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/

static void mqtt_app_start(void)
{
    // mqtt_get_cer();
    char client_id[50] = "sdk-nodejs-e07c7d1a-1def-45b2-a492-f6874c5dd09e";
    sprintf(deive_data.mac_add, "%s", wifi_get_mac());
    APP_LOGI("Client id: %s", client_id);
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtts://am25aqsnybb6p-ats.iot.sa-east-1.amazonaws.com:8883",
        .event_handle = mqtt_event_handler,
        .client_cert_pem = (const char *)client_cert_pem_start,
        .client_key_pem = (const char *)client_key_pem_start,
        .cert_pem = (const char *)server_cert_pem_start,
        .client_id = client_id,
        // .use_global_ca_store = true,
    };

    // //using cloud mqt
    // const esp_mqtt_client_config_t mqtt_cfg = {
    //     .host = "m13.cloudmqtt.com",
    //     .port = 11734,
    //     .client_id = "1234adc",
    //     .username = "wcewiofp",
    //     .password = "fyFZMCLNvoD9",
    //     .keepalive = 60,
    //     .event_handle = mqtt_event_handler,
    // };
    // memset(mqtt_cfg.client_id, 0x00, sizeof(MQTT_MAX_CLIENT_LEN));
    memset(mqtt_config.mqtt_topic_pub, 0x00, sizeof(mqtt_config.mqtt_topic_pub));
    memset(mqtt_config.mqtt_topic_pub_err, 0x00, sizeof(mqtt_config.mqtt_topic_pub_err));
    memset(mqtt_config.mqtt_topic_jobsub, 0x00, sizeof(mqtt_config.mqtt_topic_jobsub));
    memset(mqtt_config.mqtt_topic_jobpub, 0x00, sizeof(mqtt_config.mqtt_topic_jobpub));

    sprintf(mqtt_config.mqtt_topic_pub, "%s", "topic_1");
    // sprintf(mqtt_config.mqtt_topic_pub_err, "stag/dt/ard-smartstop/%s/error", client_id);
    sprintf(mqtt_config.mqtt_topic_jobsub, "topic_2");
    // sprintf(mqtt_config.mqtt_topic_jobpub, "stag/job/ard-smartstop/%s/status", client_id);

    // sprintf((char *)mqtt_cfg.client_id, "ard-smartstop-%s", wifi_get_mac());
    APP_LOGI("mqtt_topic_pub = %s", mqtt_config.mqtt_topic_pub);
    // APP_LOGI("mqtt_topic_pub_err = %s", mqtt_config.mqtt_topic_pub_err);
    APP_LOGI("mqtt_topic_jobsub = %s", mqtt_config.mqtt_topic_jobsub);
    // APP_LOGI("mqtt_topic_jobpub = %s", mqtt_config.mqtt_topic_jobpub);
    // APP_LOGI("mqtt_cfg.client_id = %s", mqtt_cfg.client_id);

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}
/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        deive_data.mqtt_status = true;
        msg_id = esp_mqtt_client_subscribe(client, mqtt_config.mqtt_topic_jobsub, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        deive_data.mqtt_status = false;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        bool status = json_parser_job((const char *)event->data, event->data_len);
        APP_LOGI("status = %d", status);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
uint32_t previous_time_start = 0;
uint8_t state_send_data = 0;
bool time_inter_val_send = false;
static void mqtt_send_task(void *pvParameters)
{
    int msg_id = -1;
    while (1)
    {
        if (deive_data.mqtt_status == true)
        {
            if( deive_data.sensor.hammer_detect == 1)
            {
                APP_LOGI("-----user send data to the cloud");
                char *message_packet = (char *)malloc(200 * sizeof(char));
                memset(message_packet, 0x00, 200 * sizeof(char));
                json_packet_message_sensor(message_packet);
                APP_LOGI("send : = %s", message_packet);
                msg_id = esp_mqtt_client_publish(client, mqtt_config.mqtt_topic_pub, message_packet, 0, 0, 0);
                memset(message_packet, 0x00, 200 * sizeof(char));
                free(message_packet);
                APP_LOGI("sent publish successful, msg_id=%d", msg_id);
                deive_data.sensor.hammer_detect = 0; // clean hammer detection
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void mqtt_send_message(uint8_t *action)
{
    
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
static char *wifi_get_mac(void)
{
    char *mac_add;
    mac_add = (char *)malloc(18 + 1);
    //Get the derived MAC address for each network interface
    uint8_t derived_mac_addr[6] = {0};
    //Get MAC address for WiFi Station interface
    ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_WIFI_STA));
    sprintf(mac_add, "%x:%x:%x:%x:%x:%x", derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2], derived_mac_addr[3],
            derived_mac_addr[4], derived_mac_addr[5]);
    APP_LOGD("wifi_get_mac end = %s", mac_add);
    return mac_add;
}