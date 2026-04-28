#include "net_data.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "cJSON.h"
#include <string.h>

static NetData_t         s_data;
static NetJsonData_t     s_json;
static SemaphoreHandle_t s_mutex;

static void *json_malloc(size_t sz) {
    return pvPortMalloc(sz);
}

static void json_free(void *ptr) {
    vPortFree(ptr);
}

static void copy_text(char *dst, uint16_t dst_len, const char *src) {
    uint16_t i = 0;

    if (!dst || dst_len == 0) return;
    if (!src) {
        dst[0] = '\0';
        return;
    }

    while (i < dst_len - 1 && src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static void json_clear_locked(uint8_t truncated) {
    memset(&s_json, 0, sizeof(s_json));
    s_json.truncated = truncated;
}

static void json_parse_locked(const char *body, uint16_t len, uint8_t truncated) {
    cJSON *root;
    cJSON *item;
    uint8_t count = 0;

    json_clear_locked(truncated);
    if (!body || len == 0 || truncated) return;

    root = cJSON_ParseWithLength(body, len);
    if (!root) return;

    s_json.valid = 1;

    if (!cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return;
    }

    item = root->child;
    while (item && count < NET_JSON_MAX_FIELDS) {
        NetJsonField_t *field = &s_json.fields[count];

        copy_text(field->key, sizeof(field->key), item->string);

        if (cJSON_IsNumber(item)) {
            field->type = NET_JSON_TYPE_NUMBER;
            field->number = item->valuedouble;
        } else if (cJSON_IsString(item) && item->valuestring) {
            field->type = NET_JSON_TYPE_STRING;
            copy_text(field->value, sizeof(field->value), item->valuestring);
        } else if (cJSON_IsBool(item)) {
            field->type = NET_JSON_TYPE_BOOL;
            field->number = cJSON_IsTrue(item) ? 1.0 : 0.0;
            copy_text(field->value, sizeof(field->value), cJSON_IsTrue(item) ? "true" : "false");
        } else if (cJSON_IsNull(item)) {
            field->type = NET_JSON_TYPE_NULL;
            copy_text(field->value, sizeof(field->value), "null");
        } else if (cJSON_IsArray(item)) {
            field->type = NET_JSON_TYPE_ARRAY;
            copy_text(field->value, sizeof(field->value), "[array]");
        } else if (cJSON_IsObject(item)) {
            field->type = NET_JSON_TYPE_OBJECT;
            copy_text(field->value, sizeof(field->value), "{object}");
        }

        count++;
        item = item->next;
    }

    s_json.field_count = count;
    cJSON_Delete(root);
}

void net_data_init(void) {
    cJSON_Hooks hooks;

    hooks.malloc_fn = json_malloc;
    hooks.free_fn = json_free;
    cJSON_InitHooks(&hooks);

    s_mutex = xSemaphoreCreateMutex();
    memset(&s_data, 0, sizeof(s_data));
    memset(&s_json, 0, sizeof(s_json));
}

NetData_t net_data_get(void) {
    NetData_t copy;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    copy = s_data;
    xSemaphoreGive(s_mutex);
    return copy;
}

void net_data_set(const NetData_t *d) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_data = *d;
    xSemaphoreGive(s_mutex);
}

void net_data_set_wifi_state(ESP_WifiState_t state) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_data.wifi_state = state;
    s_data.last_update = xTaskGetTickCount();
    xSemaphoreGive(s_mutex);
}

void net_data_set_http_result(int16_t status, const char *body, uint16_t len) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_data.last_http_status = status;
    s_data.http_busy = 0;
    if (body && len > 0) {
        uint16_t copy_len = len;
        if (copy_len > sizeof(s_data.last_http_body) - 1)
            copy_len = sizeof(s_data.last_http_body) - 1;
        memcpy(s_data.last_http_body, body, copy_len);
        s_data.last_http_body[copy_len] = '\0';
        s_data.last_http_ok = (status >= 200 && status < 300) ? 1 : 0;
        json_parse_locked(s_data.last_http_body, copy_len,
                          (len > sizeof(s_data.last_http_body) - 1) ? 1 : 0);
    } else {
        s_data.last_http_body[0] = '\0';
        s_data.last_http_ok = 0;
        json_clear_locked(0);
    }
    s_data.last_update = xTaskGetTickCount();
    xSemaphoreGive(s_mutex);
}

NetJsonData_t net_data_get_json(void) {
    NetJsonData_t copy;

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    copy = s_json;
    xSemaphoreGive(s_mutex);
    return copy;
}

uint8_t net_data_json_get_number(const char *key, double *out) {
    uint8_t i;
    uint8_t found = 0;

    if (!key || !out) return 0;

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (s_json.valid) {
        for (i = 0; i < s_json.field_count; i++) {
            if (s_json.fields[i].type == NET_JSON_TYPE_NUMBER &&
                strcmp(s_json.fields[i].key, key) == 0) {
                *out = s_json.fields[i].number;
                found = 1;
                break;
            }
        }
    }
    xSemaphoreGive(s_mutex);

    return found;
}

uint8_t net_data_json_get_string(const char *key, char *out, uint16_t out_len) {
    uint8_t i;
    uint8_t found = 0;

    if (!key || !out || out_len == 0) return 0;

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (s_json.valid) {
        for (i = 0; i < s_json.field_count; i++) {
            if (s_json.fields[i].type == NET_JSON_TYPE_STRING &&
                strcmp(s_json.fields[i].key, key) == 0) {
                copy_text(out, out_len, s_json.fields[i].value);
                found = 1;
                break;
            }
        }
    }
    xSemaphoreGive(s_mutex);

    return found;
}
