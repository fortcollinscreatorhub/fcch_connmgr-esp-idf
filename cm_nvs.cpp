// Copyright 2024-2025 Stephen Warren <swarren@wwwdotorg.org>
// SPDX-License-Identifier: MIT

#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <esp_log.h>
#include <esp_mac.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "cm_nvs.h"

static const char *TAG = "cm_nvs";

static nvs_handle_t cm_nvs_handle;

void cm_nvs_init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_LOGI(TAG, "Erasing NVS flash");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = nvs_open("rfid", NVS_READWRITE, &cm_nvs_handle);
    ESP_ERROR_CHECK(err);
}

void cm_nvs_wipe() {
    ESP_ERROR_CHECK(nvs_flash_erase());
    cm_nvs_init();
}

esp_err_t cm_nvs_read_str(const char* name, const char **p_val) {
    *p_val = NULL;

    size_t length = 0;
    esp_err_t err = nvs_get_str(cm_nvs_handle, name, NULL, &length);
    if (err != ESP_OK)
        return err;

    size_t buf_length = length + 1;
    char *val = (char *)malloc(buf_length);
    if (val == NULL)
        return ESP_ERR_NO_MEM;

    err = nvs_get_str(cm_nvs_handle, name, val, &length);
    if (err != ESP_OK) {
        free(val);
        return err;
    }

    assert(length < buf_length);
    val[length] = '\0';
    *p_val = val;
    return ESP_OK;
}

esp_err_t cm_nvs_read_u32(const char* name, uint32_t *p_val) {
    return nvs_get_u32(cm_nvs_handle, name, p_val);
}

esp_err_t cm_nvs_read_u16(const char* name, uint16_t *p_val) {
    return nvs_get_u16(cm_nvs_handle, name, p_val);
}

esp_err_t cm_nvs_read_float(const char* name, float *p_val) {
    size_t length = 0;
    esp_err_t err = nvs_get_blob(cm_nvs_handle, name, NULL, &length);
    if (err != ESP_OK)
        return err;
    if (length != sizeof(float))
        return ESP_ERR_INVALID_SIZE;

    err = nvs_get_blob(cm_nvs_handle, name, p_val, &length);
    if (err != ESP_OK)
        return err;

    return ESP_OK;
}

esp_err_t cm_nvs_write_str(const char* name, const char *val) {
    return nvs_set_str(cm_nvs_handle, name, val);
}

esp_err_t cm_nvs_write_u32(const char* name, uint32_t val) {
    return nvs_set_u32(cm_nvs_handle, name, val);
}

esp_err_t cm_nvs_write_u16(const char* name, uint16_t val) {
    return nvs_set_u16(cm_nvs_handle, name, val);
}

esp_err_t cm_nvs_write_float(const char* name, float val) {
    return nvs_set_blob(cm_nvs_handle, name, &val, sizeof(val));
}
