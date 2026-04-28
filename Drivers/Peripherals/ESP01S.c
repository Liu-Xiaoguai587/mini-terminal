#include "ESP01S.h"
#include "USART2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Internal state ─────────────────────────────────────────── */
static ESP_WifiState_t s_wifi_state = ESP_WIFI_DISCONNECTED;

/* ── Line buffer for AT response parsing ────────────────────── */
#define LINE_BUF_SIZE  256
static char s_line[LINE_BUF_SIZE];

/* ── Helper: check if line starts with prefix ───────────────── */
static uint8_t starts_with(const char *line, const char *prefix) {
    return (strncmp(line, prefix, strlen(prefix)) == 0);
}

/* ── Helper: process unsolicited WiFi events ────────────────── */
static void process_unsolicited(const char *line) {
    if (starts_with(line, "WIFI CONNECTED")) {
        s_wifi_state = ESP_WIFI_CONNECTING;
    } else if (starts_with(line, "WIFI GOT IP")) {
        s_wifi_state = ESP_WIFI_CONNECTED;
    } else if (starts_with(line, "WIFI DISCONNECT")) {
        s_wifi_state = ESP_WIFI_DISCONNECTED;
    }
}

/* ── Core: send AT command and collect response ─────────────── */
ESP_Result_t ESP_SendAT(const char *cmd, char *resp_buf, uint16_t resp_max,
                        uint32_t timeout_ms) {
    uint16_t resp_pos = 0;

    if (resp_buf && resp_max > 0) resp_buf[0] = '\0';

    /* Send command */
    USART2_FlushRX();
    USART2_SendStr(cmd);
    USART2_SendStr("\r\n");

    /* Read response lines until OK/ERROR/timeout */
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);

    for (;;) {
        TickType_t now = xTaskGetTickCount();
        int32_t remain = (int32_t)(deadline - now);
        if (remain <= 0) return ESP_TIMEOUT;

        uint16_t len = USART2_ReadLine(s_line, LINE_BUF_SIZE, (uint32_t)remain);
        if (len == 0) return ESP_TIMEOUT;

        /* Skip echo of our command */
        if (starts_with(s_line, cmd)) continue;
        /* Skip empty lines */
        if (s_line[0] == '\r' || s_line[0] == '\n') continue;

        /* Check terminal responses */
        if (starts_with(s_line, "OK")) return ESP_OK;
        if (starts_with(s_line, "ERROR")) return ESP_ERROR;
        if (starts_with(s_line, "FAIL")) return ESP_ERROR;
        if (starts_with(s_line, "busy")) return ESP_BUSY;

        /* Handle unsolicited WiFi events */
        if (starts_with(s_line, "WIFI ")) {
            process_unsolicited(s_line);
            continue;
        }

        /* Append to response buffer */
        if (resp_buf && resp_pos < resp_max - 1) {
            uint16_t copy_len = len;
            if (resp_pos + copy_len >= resp_max - 1)
                copy_len = resp_max - 1 - resp_pos;
            memcpy(resp_buf + resp_pos, s_line, copy_len);
            resp_pos += copy_len;
            resp_buf[resp_pos] = '\0';
        }
    }
}

/* ── Init: verify AT communication ─────────────────────────── */
ESP_Result_t ESP_Init(void) {
    ESP_Result_t ret;

    vTaskDelay(pdMS_TO_TICKS(500));  /* wait for ESP boot */

    /* Try AT up to 3 times */
    for (int i = 0; i < 3; i++) {
        ret = ESP_SendAT("AT", NULL, 0, 2000);
        if (ret == ESP_OK) goto init_ok;
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    /* Try reset */
    ESP_Reset();
    vTaskDelay(pdMS_TO_TICKS(3000));

    ret = ESP_SendAT("AT", NULL, 0, 2000);
    if (ret != ESP_OK) return ESP_ERROR;

init_ok:
    /* Disable echo */
    ESP_SendAT("ATE0", NULL, 0, 1000);
    /* Set station mode */
    ESP_SendAT("AT+CWMODE=1", NULL, 0, 1000);
    /* Single connection mode */
    ESP_SendAT("AT+CIPMUX=0", NULL, 0, 1000);

    return ESP_OK;
}

/* ── Reset ──────────────────────────────────────────────────── */
ESP_Result_t ESP_Reset(void) {
    USART2_FlushRX();
    USART2_SendStr("AT+RST\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));
    USART2_FlushRX();
    s_wifi_state = ESP_WIFI_DISCONNECTED;
    return ESP_OK;
}

/* ── WiFi Connect ───────────────────────────────────────────── */
ESP_Result_t ESP_WiFi_Connect(const char *ssid, const char *pass) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, pass);

    s_wifi_state = ESP_WIFI_CONNECTING;
    ESP_Result_t ret = ESP_SendAT(cmd, NULL, 0, 15000);

    if (ret == ESP_OK) {
        s_wifi_state = ESP_WIFI_CONNECTED;
    } else {
        s_wifi_state = ESP_WIFI_ERROR;
    }
    return ret;
}

/* ── WiFi Disconnect ────────────────────────────────────────── */
ESP_Result_t ESP_WiFi_Disconnect(void) {
    ESP_Result_t ret = ESP_SendAT("AT+CWQAP", NULL, 0, 5000);
    s_wifi_state = ESP_WIFI_DISCONNECTED;
    return ret;
}

/* ── WiFi Get IP ────────────────────────────────────────────── */
ESP_Result_t ESP_WiFi_GetIP(char *ip_buf, uint8_t buf_len) {
    char resp[128];
    ESP_Result_t ret = ESP_SendAT("AT+CIFSR", resp, sizeof(resp), 5000);
    if (ret != ESP_OK) return ret;

    /* Parse: +CIFSR:STAIP,"x.x.x.x" */
    char *p = strstr(resp, "STAIP,\"");
    if (!p) { ip_buf[0] = '\0'; return ESP_ERROR; }
    p += 7;  /* skip STAIP," */
    char *end = strchr(p, '"');
    if (!end) { ip_buf[0] = '\0'; return ESP_ERROR; }

    uint8_t len = (uint8_t)(end - p);
    if (len >= buf_len) len = buf_len - 1;
    memcpy(ip_buf, p, len);
    ip_buf[len] = '\0';
    return ESP_OK;
}

/* ── WiFi Status ────────────────────────────────────────────── */
ESP_WifiState_t ESP_WiFi_Status(void) {
    return s_wifi_state;
}

/* ── TCP Connect ────────────────────────────────────────────── */
ESP_Result_t ESP_TCP_Connect(const char *host, uint16_t port) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u", host, port);
    return ESP_SendAT(cmd, NULL, 0, 10000);
}

/* ── TCP Send ───────────────────────────────────────────────── */
ESP_Result_t ESP_TCP_Send(const uint8_t *data, uint16_t len) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u", len);

    /* Send CIPSEND command and wait for '>' prompt */
    USART2_FlushRX();
    USART2_SendStr(cmd);
    USART2_SendStr("\r\n");

    /* Wait for '>' */
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(5000);
    for (;;) {
        TickType_t now = xTaskGetTickCount();
        int32_t remain = (int32_t)(deadline - now);
        if (remain <= 0) return ESP_TIMEOUT;

        uint16_t n = USART2_ReadLine(s_line, LINE_BUF_SIZE, (uint32_t)remain);
        if (n == 0) return ESP_TIMEOUT;
        if (strchr(s_line, '>')) break;
        if (starts_with(s_line, "ERROR")) return ESP_ERROR;
        if (starts_with(s_line, "link is not")) return ESP_CONN_CLOSED;
    }

    /* Send actual data */
    USART2_SendBuf(data, len);

    /* Wait for SEND OK */
    for (;;) {
        TickType_t now = xTaskGetTickCount();
        int32_t remain = (int32_t)(deadline - now);
        if (remain <= 0) return ESP_TIMEOUT;

        uint16_t n = USART2_ReadLine(s_line, LINE_BUF_SIZE, (uint32_t)remain);
        if (n == 0) return ESP_TIMEOUT;
        if (starts_with(s_line, "SEND OK")) return ESP_OK;
        if (starts_with(s_line, "SEND FAIL")) return ESP_ERROR;
        if (starts_with(s_line, "ERROR")) return ESP_ERROR;
    }
}

/* ── TCP Close ──────────────────────────────────────────────── */
ESP_Result_t ESP_TCP_Close(void) {
    return ESP_SendAT("AT+CIPCLOSE", NULL, 0, 5000);
}

/* ── Helper: receive +IPD data into buffer ──────────────────── */
static uint16_t recv_ipd_data(char *buf, uint16_t max_len, uint32_t timeout_ms) {
    uint16_t total = 0;
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);

    for (;;) {
        TickType_t now = xTaskGetTickCount();
        int32_t remain = (int32_t)(deadline - now);
        if (remain <= 0) break;

        uint16_t n = USART2_ReadLine(s_line, LINE_BUF_SIZE, (uint32_t)remain);
        if (n == 0) break;

        /* Connection closed by server */
        if (starts_with(s_line, "CLOSED")) break;
        if (starts_with(s_line, "ERROR")) break;

        /* Handle unsolicited */
        if (starts_with(s_line, "WIFI ")) {
            process_unsolicited(s_line);
            continue;
        }

        /* +IPD,<len>:<data...> */
        if (starts_with(s_line, "+IPD,")) {
            char *comma = s_line + 5;
            char *colon = strchr(comma, ':');
            if (!colon) continue;

            uint16_t ipd_len = (uint16_t)atoi(comma);
            colon++;  /* skip ':' */

            /* Data after colon in this line */
            uint16_t inline_len = n - (uint16_t)(colon - s_line);
            if (inline_len > 0 && total < max_len) {
                uint16_t copy = inline_len;
                if (total + copy > max_len) copy = max_len - total;
                memcpy(buf + total, colon, copy);
                total += copy;
            }

            /* Remaining bytes to read */
            uint16_t remaining = (ipd_len > inline_len) ? (ipd_len - inline_len) : 0;
            if (remaining > 0 && total < max_len) {
                uint16_t space = max_len - total;
                uint16_t to_read = (remaining < space) ? remaining : space;
                uint16_t got = USART2_ReadBytes((uint8_t *)(buf + total), to_read,
                                               (uint32_t)remain);
                total += got;
                /* Discard excess if buffer full */
                if (remaining > to_read) {
                    uint16_t discard = remaining - to_read;
                    uint8_t tmp;
                    while (discard--) USART2_ReadBytes(&tmp, 1, 100);
                }
            }
            continue;
        }
    }

    if (total < max_len) buf[total] = '\0';
    return total;
}

/* ── Helper: parse HTTP response ────────────────────────────── */
static void parse_http_response(char *buf, uint16_t len, ESP_HTTP_Response_t *out) {
    out->status_code = -1;
    out->body = NULL;
    out->body_len = 0;

    if (len == 0 || !buf) return;

    /* Parse status line: "HTTP/1.x <code> ..." */
    char *p = strstr(buf, "HTTP/");
    if (!p) return;
    char *sp = strchr(p, ' ');
    if (!sp) return;
    out->status_code = (int16_t)atoi(sp + 1);

    /* Find body: separated by \r\n\r\n */
    char *body = strstr(buf, "\r\n\r\n");
    if (body) {
        body += 4;
        out->body = body;
        out->body_len = len - (uint16_t)(body - buf);
    }
}

/* ── HTTP GET ───────────────────────────────────────────────── */
ESP_Result_t ESP_HTTP_GET(const char *host, uint16_t port,
                          const char *path,
                          char *resp_buf, uint16_t resp_max,
                          ESP_HTTP_Response_t *out) {
    ESP_Result_t ret;

    out->status_code = -1;
    out->body = NULL;
    out->body_len = 0;

    /* Open TCP connection */
    ret = ESP_TCP_Connect(host, port);
    if (ret != ESP_OK) return ret;

    /* Build HTTP request */
    char req[256];
    int req_len = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, host);

    /* Send request */
    ret = ESP_TCP_Send((const uint8_t *)req, (uint16_t)req_len);
    if (ret != ESP_OK) { ESP_TCP_Close(); return ret; }

    /* Receive response */
    uint16_t recv_len = recv_ipd_data(resp_buf, resp_max - 1, 15000);
    if (recv_len < resp_max) resp_buf[recv_len] = '\0';

    /* Parse */
    parse_http_response(resp_buf, recv_len, out);

    return ESP_OK;
}

/* ── HTTP POST ──────────────────────────────────────────────── */
ESP_Result_t ESP_HTTP_POST(const char *host, uint16_t port,
                           const char *path,
                           const char *content_type,
                           const char *post_body, uint16_t body_len,
                           char *resp_buf, uint16_t resp_max,
                           ESP_HTTP_Response_t *out) {
    ESP_Result_t ret;

    out->status_code = -1;
    out->body = NULL;
    out->body_len = 0;

    /* Open TCP connection */
    ret = ESP_TCP_Connect(host, port);
    if (ret != ESP_OK) return ret;

    /* Build HTTP request header */
    char hdr[256];
    int hdr_len = snprintf(hdr, sizeof(hdr),
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %u\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, host, content_type, body_len);

    /* Send header + body as one CIPSEND */
    uint16_t total_len = (uint16_t)hdr_len + body_len;
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u", total_len);

    USART2_FlushRX();
    USART2_SendStr(cmd);
    USART2_SendStr("\r\n");

    /* Wait for '>' */
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(5000);
    for (;;) {
        TickType_t now = xTaskGetTickCount();
        int32_t remain = (int32_t)(deadline - now);
        if (remain <= 0) { ESP_TCP_Close(); return ESP_TIMEOUT; }

        uint16_t n = USART2_ReadLine(s_line, LINE_BUF_SIZE, (uint32_t)remain);
        if (n == 0) { ESP_TCP_Close(); return ESP_TIMEOUT; }
        if (strchr(s_line, '>')) break;
        if (starts_with(s_line, "ERROR")) { ESP_TCP_Close(); return ESP_ERROR; }
    }

    /* Send header then body */
    USART2_SendBuf((const uint8_t *)hdr, (uint16_t)hdr_len);
    USART2_SendBuf((const uint8_t *)post_body, body_len);

    /* Wait for SEND OK */
    for (;;) {
        TickType_t now = xTaskGetTickCount();
        int32_t remain = (int32_t)(deadline - now);
        if (remain <= 0) { ESP_TCP_Close(); return ESP_TIMEOUT; }

        uint16_t n = USART2_ReadLine(s_line, LINE_BUF_SIZE, (uint32_t)remain);
        if (n == 0) { ESP_TCP_Close(); return ESP_TIMEOUT; }
        if (starts_with(s_line, "SEND OK")) break;
        if (starts_with(s_line, "SEND FAIL")) { ESP_TCP_Close(); return ESP_ERROR; }
    }

    /* Receive response */
    uint16_t recv_len = recv_ipd_data(resp_buf, resp_max - 1, 15000);
    if (recv_len < resp_max) resp_buf[recv_len] = '\0';

    /* Parse */
    parse_http_response(resp_buf, recv_len, out);

    return ESP_OK;
}
