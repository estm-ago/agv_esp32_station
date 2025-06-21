/* Keep Alive engine for wss server example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h> 

typedef struct wss_keep_alive_storage* wss_keep_alive_t;

typedef bool (*wss_check_client_alive_cb_t) (wss_keep_alive_t alive_handle, int sockfd);
typedef bool (*wss_client_not_alive_cb_t) (wss_keep_alive_t alive_handle, int sockfd);

typedef enum {
    NO_CLIENT = 0,
    CLIENT_FD_ADD,
    CLIENT_FD_REMOVE,
    CLIENT_UPDATE,
    CLIENT_ACTIVE,
    STOP_TASK,
} client_fd_action_type_t;

typedef struct {
    client_fd_action_type_t type;
    int fd;
    uint64_t last_seen;
} client_fd_action_t;

typedef struct wss_keep_alive_storage {
    size_t max_clients;
    wss_check_client_alive_cb_t check_client_alive_cb;
    wss_check_client_alive_cb_t client_not_alive_cb;
    size_t keep_alive_period_ms;
    size_t not_alive_after_ms;
    void * user_ctx;
    QueueHandle_t q;
    client_fd_action_t clients[];
} wss_keep_alive_storage_t;

/**
 * @brief Confiuration struct
 */
typedef struct {
    size_t max_clients;                                      /*!< max number of clients */
    size_t task_stack_size;                                  /*!< stack size of the created task */
    size_t task_prio;                                        /*!< priority of the created task */
    size_t keep_alive_period_ms;                             /*!< check every client after this time */
    size_t not_alive_after_ms;                               /*!< consider client not alive after this time */
    wss_check_client_alive_cb_t check_client_alive_cb;       /*!< callback function to check if client is alive */
    wss_client_not_alive_cb_t client_not_alive_cb;           /*!< callback function to notify that the client is not alive */
    void *user_ctx;                                          /*!< user context available in the keep-alive handle */
} wss_keep_alive_config_t;

#define KEEP_ALIVE_CONFIG_DEFAULT() \
    { \
    .max_clients = 10,                      \
    .task_stack_size = 2048,                \
    .task_prio = tskIDLE_PRIORITY+1,        \
    .keep_alive_period_ms = 5000,           \
    .not_alive_after_ms = 10000,            \
}

/**
 * @brief Adds a new client to internal set of clients to keep an eye on
 *
 * @param alive_handle keep-alive handle
 * @param sockfd socket file descriptor for this client
 * @return ESP_OK on success
 */
esp_err_t wss_keep_alive_add_client(wss_keep_alive_t alive_handle, int sockfd);

/**
 * @brief Removes this client from the set
 *
 * @param alive_handle keep-alive handle
 * @param sockfd socket file descriptor for this client
 * @return ESP_OK on success
 */
esp_err_t wss_keep_alive_remove_client(wss_keep_alive_t alive_handle, int sockfd);

/**
 * @brief Notify that this client is alive
 *
 * @param alive_handle keep-alive handle
 * @param sockfd socket file descriptor for this client
 * @return ESP_OK on success
 */

esp_err_t wss_keep_alive_client_is_active(wss_keep_alive_t alive_handle, int sockfd);

/**
 * @brief Starts keep-alive engine
 *
 * @param config keep-alive configuration
 * @return keep alive handle
 */
wss_keep_alive_t wss_keep_alive_start(wss_keep_alive_config_t *config);

/**
 * @brief Stops keep-alive engine
 *
 * @param alive_handle keep-alive handle
 */
void wss_keep_alive_stop(wss_keep_alive_t alive_handle);

/**
 * @brief Sets user defined context
 *
 * @param alive_handle keep-alive handle
 * @param ctx user context
 */
void wss_keep_alive_set_user_ctx(wss_keep_alive_t alive_handle, void *ctx);

/**
 * @brief Gets user defined context
 *
 * @param alive_handle keep-alive handle
 * @return ctx user context
 */
void* wss_keep_alive_get_user_ctx(wss_keep_alive_t alive_handle);
