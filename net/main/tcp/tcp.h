#ifndef TCP_H
#define TCP_H

#include <esp_types.h>

void tcp_client(char *ip, uint16_t port);
int send_msg(char *payload);

#endif