#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ESP01S.h"
#include "net_data.h"
#include <string.h>

/* ── WiFi credentials (change to your network) ─────────────── */
#define WIFI_SSID  "mi"
#define WIFI_PASS  "1244190080"

/* ── Command queue ──────────────────────────────────────────── */
typedef enum {
    WIFI_CMD_CONNECT,
    WIFI_CMD_DISCONNECT,
    WIFI_CMD_HTTP_GET,
    WIFI_CMD_HTTP_POST,
} WifiCmdType_t;

typedef struct {
    WifiCmdType_t  type;
    const char    *host;
    uint16_t       port;
    const char    *path;
    const char    *content_type;
    const char    *post_body;
    uint16_t       post_body_len;
} WifiCmd_t;

QueueHandle_t wifi_cmd_queue;

/* ── HTTP work buffer (static to save stack) ────────────────── */
static char http_buf[2048];

/* ── Timeout error counter for auto-recovery ────────────────── */
static uint8_t s_timeout_count;

/* ── Internal: handle HTTP GET command ──────────────────────── */
static void handle_http_get(const WifiCmd_t *cmd) {
    NetData_t nd = net_data_get();
    nd.http_busy = 1;
    net_data_set(&nd);

    ESP_HTTP_Response_t resp;
    ESP_Result_t ret = ESP_HTTP_GET(cmd->host, cmd->port, cmd->path,
                                    http_buf, sizeof(http_buf), &resp);

    if (ret == ESP_OK && resp.body) {
        net_data_set_http_result(resp.status_code, resp.body, resp.body_len);
        s_timeout_count = 0;
    } else {
        net_data_set_http_result(-1, NULL, 0);
        if (ret == ESP_TIMEOUT) s_timeout_count++;
    }
}

/* ── Internal: handle HTTP POST command ─────────────────────── */
static void handle_http_post(const WifiCmd_t *cmd) {
    NetData_t nd = net_data_get();
    nd.http_busy = 1;
    net_data_set(&nd);

    ESP_HTTP_Response_t resp;
    ESP_Result_t ret = ESP_HTTP_POST(cmd->host, cmd->port, cmd->path,
                                     cmd->content_type,
                                     cmd->post_body, cmd->post_body_len,
                                     http_buf, sizeof(http_buf), &resp);

    if (ret == ESP_OK && resp.body) {
        net_data_set_http_result(resp.status_code, resp.body, resp.body_len);
        s_timeout_count = 0;
    } else {
        net_data_set_http_result(-1, NULL, 0);
        if (ret == ESP_TIMEOUT) s_timeout_count++;
    }
}

/* ── Internal: periodic maintenance ─────────────────────────── */
static void maintenance(void) {
    /* Check WiFi status */
    char ip[16];
    ESP_Result_t ret = ESP_WiFi_GetIP(ip, sizeof(ip));

    if (ret == ESP_OK && ip[0] != '\0' && ip[0] != '0') {
        NetData_t nd = net_data_get();
        nd.wifi_state = ESP_WIFI_CONNECTED;
        strncpy(nd.ip_addr, ip, sizeof(nd.ip_addr) - 1);
        nd.ip_addr[sizeof(nd.ip_addr) - 1] = '\0';
        nd.last_update = xTaskGetTickCount();
        net_data_set(&nd);
        s_timeout_count = 0;
    } else {
        net_data_set_wifi_state(ESP_WIFI_DISCONNECTED);
        /* Auto-reconnect */
        ESP_WiFi_Connect(WIFI_SSID, WIFI_PASS);
        if (ESP_WiFi_Status() == ESP_WIFI_CONNECTED) {
            ret = ESP_WiFi_GetIP(ip, sizeof(ip));
            if (ret == ESP_OK) {
                NetData_t nd = net_data_get();
                nd.wifi_state = ESP_WIFI_CONNECTED;
                strncpy(nd.ip_addr, ip, sizeof(nd.ip_addr) - 1);
                nd.ip_addr[sizeof(nd.ip_addr) - 1] = '\0';
                nd.last_update = xTaskGetTickCount();
                net_data_set(&nd);
            }
        }
    }

    /* Hard recovery if too many consecutive timeouts */
    if (s_timeout_count >= 3) {
        ESP_Reset();
        vTaskDelay(pdMS_TO_TICKS(3000));
        ESP_Init();
        ESP_WiFi_Connect(WIFI_SSID, WIFI_PASS);
        s_timeout_count = 0;
    }
}

/* ── WiFi task entry ────────────────────────────────────────── */
void task_wifi(void *pvParameters) {
    (void)pvParameters;

    wifi_cmd_queue = xQueueCreate(4, sizeof(WifiCmd_t));

    /* Initialize ESP module */
    if (ESP_Init() == ESP_OK) {
        ESP_WiFi_Connect(WIFI_SSID, WIFI_PASS);

        char ip[16];
        if (ESP_WiFi_GetIP(ip, sizeof(ip)) == ESP_OK) {
            NetData_t nd = {0};
            nd.wifi_state = ESP_WIFI_CONNECTED;
            strncpy(nd.ip_addr, ip, sizeof(nd.ip_addr) - 1);
            nd.last_update = xTaskGetTickCount();
            net_data_set(&nd);
        } else {
            net_data_set_wifi_state(ESP_WiFi_Status());
        }
    } else {
        net_data_set_wifi_state(ESP_WIFI_ERROR);
    }

    /* Main loop */
    WifiCmd_t cmd;
    for (;;) {
        if (xQueueReceive(wifi_cmd_queue, &cmd, pdMS_TO_TICKS(10000)) == pdTRUE) {
            switch (cmd.type) {
                case WIFI_CMD_CONNECT:
                    ESP_WiFi_Connect(WIFI_SSID, WIFI_PASS);
                    net_data_set_wifi_state(ESP_WiFi_Status());
                    break;
                case WIFI_CMD_DISCONNECT:
                    ESP_WiFi_Disconnect();
                    net_data_set_wifi_state(ESP_WIFI_DISCONNECTED);
                    break;
                case WIFI_CMD_HTTP_GET:
                    handle_http_get(&cmd);
                    break;
                case WIFI_CMD_HTTP_POST:
                    handle_http_post(&cmd);
                    break;
            }
        } else {
            /* Queue timeout = periodic maintenance */
            maintenance();
        }
    }
}
