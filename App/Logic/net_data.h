#ifndef __NET_DATA_H__
#define __NET_DATA_H__

#include "FreeRTOS.h"
#include "ESP01S.h"

#define NET_HTTP_BODY_SIZE       512
#define NET_JSON_MAX_FIELDS      8
#define NET_JSON_KEY_SIZE        24
#define NET_JSON_VALUE_SIZE      64

typedef enum {
    NET_JSON_TYPE_NONE = 0,
    NET_JSON_TYPE_NUMBER,
    NET_JSON_TYPE_STRING,
    NET_JSON_TYPE_BOOL,
    NET_JSON_TYPE_NULL,
    NET_JSON_TYPE_OBJECT,
    NET_JSON_TYPE_ARRAY,
} NetJsonType_t;

typedef struct {
    char          key[NET_JSON_KEY_SIZE];
    NetJsonType_t type;
    double        number;
    char          value[NET_JSON_VALUE_SIZE];
} NetJsonField_t;

typedef struct {
    uint8_t        valid;
    uint8_t        truncated;
    uint8_t        field_count;
    NetJsonField_t fields[NET_JSON_MAX_FIELDS];
} NetJsonData_t;

typedef struct {
    ESP_WifiState_t wifi_state;
    char            ip_addr[16];      /* "xxx.xxx.xxx.xxx\0" */
    int8_t          rssi;
    uint8_t         http_busy;        /* 1 = request in progress */
    int16_t         last_http_status; /* HTTP status code */
    char            last_http_body[NET_HTTP_BODY_SIZE];
    uint8_t         last_http_ok;     /* 1 = last request succeeded */
    TickType_t      last_update;
} NetData_t;

void      net_data_init(void);
NetData_t net_data_get(void);
void      net_data_set(const NetData_t *d);
void      net_data_set_wifi_state(ESP_WifiState_t state);
void      net_data_set_http_result(int16_t status, const char *body, uint16_t len);
NetJsonData_t net_data_get_json(void);
uint8_t   net_data_json_get_number(const char *key, double *out);
uint8_t   net_data_json_get_string(const char *key, char *out, uint16_t out_len);

#endif /* __NET_DATA_H__ */
