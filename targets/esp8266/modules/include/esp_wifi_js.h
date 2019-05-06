#ifndef ESP_WIFI_JS_H
#define ESP_WIFI_JS_H

#include <string.h>
#include "jerry_extapi.h"
#include "lwip/api.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/netbuf.h"

#define WIFI_OBJECT_NAME "WIFI"
#define WIFI_CONNECT "connect"
#define WIFI_DISCONNECT "disconnect"
#define WIFI_AVAILABLE "available"
#define WIFI_SEND "send"
#define WIFI_RECEIVE "receive"

#define WIFI_MAX_CONNECT_ATTEMPTS 100
#define WIFI_PACKAGE_SIZE 512

static uint8_t *message_buffer __attribute__((unused));

jerry_value_t send_data_on_tcp (jerry_value_t source, uint32_t bytes_to_send, const char *server, uint32_t port,
                                jerry_char_t *file_name, jerry_size_t file_name_length, void *forward);

void register_wifi_object (jerry_value_t global_object);

#endif
