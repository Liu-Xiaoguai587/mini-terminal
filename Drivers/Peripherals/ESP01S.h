#ifndef __ESP01S_H__
#define __ESP01S_H__

#include "stm32f4xx.h"

/* ── Result codes ───────────────────────────────────────────── */
typedef enum {
    ESP_OK = 0,
    ESP_ERROR,
    ESP_TIMEOUT,
    ESP_BUSY,
    ESP_CONN_CLOSED,
} ESP_Result_t;

/* ── WiFi state ─────────────────────────────────────────────── */
typedef enum {
    ESP_WIFI_DISCONNECTED = 0,
    ESP_WIFI_CONNECTING,
    ESP_WIFI_CONNECTED,
    ESP_WIFI_ERROR,
} ESP_WifiState_t;

/* ── HTTP response ──────────────────────────────────────────── */
typedef struct {
    int16_t  status_code;   /* HTTP status (200, 404, etc), -1 on parse error */
    char    *body;          /* points into resp_buf provided by caller */
    uint16_t body_len;
} ESP_HTTP_Response_t;

/* ── Core AT engine ─────────────────────────────────────────── */
ESP_Result_t ESP_Init(void);
ESP_Result_t ESP_Reset(void);
ESP_Result_t ESP_SendAT(const char *cmd, char *resp_buf, uint16_t resp_max,
                        uint32_t timeout_ms);

/* ── WiFi operations ────────────────────────────────────────── */
ESP_Result_t    ESP_WiFi_Connect(const char *ssid, const char *pass);
ESP_Result_t    ESP_WiFi_Disconnect(void);
ESP_Result_t    ESP_WiFi_GetIP(char *ip_buf, uint8_t buf_len);
ESP_WifiState_t ESP_WiFi_Status(void);

/* ── TCP operations ─────────────────────────────────────────── */
ESP_Result_t ESP_TCP_Connect(const char *host, uint16_t port);
ESP_Result_t ESP_TCP_Send(const uint8_t *data, uint16_t len);
ESP_Result_t ESP_TCP_Close(void);

/* ── HTTP convenience (built on TCP) ────────────────────────── */
ESP_Result_t ESP_HTTP_GET(const char *host, uint16_t port,
                          const char *path,
                          char *resp_buf, uint16_t resp_max,
                          ESP_HTTP_Response_t *out);

ESP_Result_t ESP_HTTP_POST(const char *host, uint16_t port,
                           const char *path,
                           const char *content_type,
                           const char *post_body, uint16_t body_len,
                           char *resp_buf, uint16_t resp_max,
                           ESP_HTTP_Response_t *out);

#endif /* __ESP01S_H__ */
