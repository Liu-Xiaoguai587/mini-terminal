#include "ESP01S.h"
#include "USART2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Internal state ─────────────────────────────────────────── */
static ESP_WifiState_t s_wifi_state = ESP_WIFI_DISCONNECTED;
volatile uint8_t  g_esp_http_stage;
volatile uint16_t g_esp_recv_len;
volatile int16_t  g_esp_http_status;

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
    /* Normal TCP mode, not transparent passthrough. */
    ESP_SendAT("AT+CIPMODE=0", NULL, 0, 1000);
    /* Active receive — push +IPD as soon as data arrives. Some ESP-AT v2.x
     * firmwares default to passive (=1), which silently drops responses
     * because we never call AT+CIPRECVDATA to pull them. */
    ESP_SendAT("AT+CIPRECVMODE=0", NULL, 0, 1000);
    /* Plain "+IPD,<len>:" header (no remote IP/port) so the parser matches. */
    ESP_SendAT("AT+CIPDINFO=0", NULL, 0, 1000);

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
    char prompt_buf[16];
    uint8_t prompt_pos = 0;
    uint8_t c;
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u", len);

    /* Send CIPSEND command and wait for '>' prompt */
    USART2_FlushRX();
    USART2_SendStr(cmd);
    USART2_SendStr("\r\n");

    /* Wait for '>' prompt. It normally has no trailing newline, so do not use
     * USART2_ReadLine() here or every send waits until the timeout expires. */
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(5000);
    for (;;) {
        TickType_t now = xTaskGetTickCount();
        int32_t remain = (int32_t)(deadline - now);
        uint32_t wait_ms;
        if (remain <= 0) return ESP_TIMEOUT;
        wait_ms = (remain > 50) ? 50U : (uint32_t)remain;

        if (USART2_ReadBytes(&c, 1, wait_ms) == 0) continue;
        if (c == '>') break;

        if (prompt_pos < sizeof(prompt_buf) - 1) {
            prompt_buf[prompt_pos++] = (char)c;
        } else {
            memmove(prompt_buf, prompt_buf + 1, sizeof(prompt_buf) - 2);
            prompt_buf[sizeof(prompt_buf) - 2] = (char)c;
            prompt_pos = sizeof(prompt_buf) - 1;
        }
        prompt_buf[prompt_pos] = '\0';

        if (strstr(prompt_buf, "ERROR") || strstr(prompt_buf, "FAIL")) {
            return ESP_ERROR;
        }
        if (strstr(prompt_buf, "link is not")) {
            return ESP_CONN_CLOSED;
        }
    }

    /* Send actual data and return immediately. We deliberately do NOT
     * consume "SEND OK" / "Recv N bytes" / "+IPD,..." here, because line-based
     * matching against ESP-AT's bursty output races the response on fast LANs
     * and silently drops the +IPD prefix. Let recv_ipd_data slurp the whole
     * UART stream as raw bytes and parse_http_response find HTTP/ inside. */
    USART2_SendBuf(data, len);
    return ESP_OK;
}

/* ── TCP Close ──────────────────────────────────────────────── */
ESP_Result_t ESP_TCP_Close(void) {
    return ESP_SendAT("AT+CIPCLOSE", NULL, 0, 5000);
}

/* ── Helper: slurp raw UART bytes until CLOSED or idle ─────────
 *
 * Instead of trying to track ESP-AT line states (which vary by firmware
 * version and race against fast servers), we just dump every byte that
 * comes out of the UART into the buffer. The HTTP response (including
 * "HTTP/1.1 ..." status line and "\r\n\r\n" body separator) ends up
 * verbatim somewhere in the buffer, mixed with AT noise like
 * "Recv N bytes\r\n", "+IPD,M:", "SEND OK\r\n". parse_http_response
 * locates "HTTP/" in the stream and ignores the surrounding noise.
 *
 * Termination:
 *   - the trailing "CLOSED\r\n" tail is detected and stops the loop
 *   - or no byte arrives for `idle_ms` (response stream is done)
 *   - or the absolute timeout / buffer fills
 */
static uint16_t recv_ipd_data(char *buf, uint16_t max_len, uint32_t timeout_ms) {
    uint16_t total = 0;
    const uint32_t idle_ms = 150;
    uint8_t saw_http = 0;
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);

    while (total < max_len - 1) {
        TickType_t now = xTaskGetTickCount();
        int32_t remain_total = (int32_t)(deadline - now);
        if (remain_total <= 0) break;
        uint32_t this_wait = (uint32_t)remain_total;
        if (this_wait > idle_ms) this_wait = idle_ms;

        uint8_t c;
        uint16_t got = USART2_ReadBytes(&c, 1, this_wait);
        if (got == 0) {
            /* Ignore idle gaps before the HTTP response; AT noise such as
             * "SEND OK" can arrive earlier than +IPD on some firmware. */
            if (saw_http) break;
            continue;
        }
        buf[total++] = (char)c;
        buf[total] = '\0';

        if (!saw_http && strstr(buf, "HTTP/")) {
            saw_http = 1;
        }

        /* Stop when we see the "CLOSED\r\n" tail emitted by ESP-AT
         * after the server closes the TCP connection. */
        if (total >= 8 && memcmp(buf + total - 8, "CLOSED\r\n", 8) == 0) {
            total -= 8;  /* trim the trailing AT marker */
            break;
        }
    }

    if (total < max_len) buf[total] = '\0';
    return total;
}

/* ── Helper: parse HTTP response ────────────────────────────── */
static void parse_http_response(char *buf, uint16_t len, ESP_HTTP_Response_t *out) {
    char *p;
    char *sp;
    char *body;
    char *json_body;
    uint8_t sep_len = 0;

    out->status_code = -1;
    out->body = NULL;
    out->body_len = 0;

    if (len == 0 || !buf) return;

    /* Parse status line: "HTTP/1.x <code> ..." */
    p = strstr(buf, "HTTP/");
    if (!p) return;
    sp = strchr(p, ' ');
    if (!sp) return;
    out->status_code = (int16_t)atoi(sp + 1);

    /* Find body from the HTTP status line, not from ESP-AT noise before it.
     * Prefer the standard CRLF separator, but accept LF-only headers too. */
    body = strstr(p, "\r\n\r\n");
    if (body) {
        sep_len = 4;
    } else {
        body = strstr(p, "\n\n");
        if (body) sep_len = 2;
    }

    if (body) {
        uint16_t body_offset;
        uint16_t body_len;
        char *cl;

        body += sep_len;
        body_offset = (uint16_t)(body - buf);
        body_len = len - body_offset;

        /* Prefer Content-Length when present so trailing AT text such as
         * "CLOSED" or "SEND OK" is not passed to the JSON parser. */
        cl = strstr(p, "Content-Length:");
        if (cl && cl < body) {
            uint16_t declared = (uint16_t)atoi(cl + 15);
            if (declared < body_len) body_len = declared;
        }

        out->body = body;
        out->body_len = body_len;
    } else {
        /* Fallback for displays/firmware paths that normalize line endings:
         * locate the JSON payload directly instead of exposing ESP-AT noise. */
        char *obj = strchr(p, '{');
        char *arr = strchr(p, '[');

        if (obj && arr) {
            json_body = (obj < arr) ? obj : arr;
        } else {
            json_body = obj ? obj : arr;
        }

        if (json_body) {
            char *cl = strstr(p, "Content-Length:");
            uint16_t body_len = len - (uint16_t)(json_body - buf);

            if (cl && cl < json_body) {
                uint16_t declared = (uint16_t)atoi(cl + 15);
                if (declared < body_len) body_len = declared;
            }

            out->body = json_body;
            out->body_len = body_len;
        }
    }
}

/* ── HTTP GET ───────────────────────────────────────────────── */
ESP_Result_t ESP_HTTP_GET(const char *host, uint16_t port,
                          const char *path,
                          char *resp_buf, uint16_t resp_max,
                          ESP_HTTP_Response_t *out) {
    ESP_Result_t ret;

    g_esp_http_stage = 1;
    g_esp_recv_len = 0;
    g_esp_http_status = -1;

    out->status_code = -1;
    out->body = NULL;
    out->body_len = 0;

    /* Defensive: drop any lingering TCP socket from a previous attempt.
     * Otherwise CIPSTART can return "ALREADY CONNECTED" + "ERROR" and the
     * server only ever sees the *first* request. Errors here are fine. */
    ESP_TCP_Close();

    /* Open TCP connection */
    g_esp_http_stage = 2;
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
    g_esp_http_stage = 3;
    ret = ESP_TCP_Send((const uint8_t *)req, (uint16_t)req_len);
    if (ret != ESP_OK) return ret;

    /* Receive response. Absolute timeout 5s — local LAN responses come back
     * within a few hundred ms, the idle break inside recv_ipd_data will
     * end the loop earlier when bytes stop flowing. */
    g_esp_http_stage = 4;
    uint16_t recv_len = recv_ipd_data(resp_buf, resp_max - 1, 5000);
    g_esp_recv_len = recv_len;
    if (recv_len < resp_max) resp_buf[recv_len] = '\0';

    /* Parse */
    g_esp_http_stage = 5;
    parse_http_response(resp_buf, recv_len, out);
    g_esp_http_status = out->status_code;
    g_esp_http_stage = 6;

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

    /* Defensive close — see comment in ESP_HTTP_GET. */
    ESP_TCP_Close();

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
