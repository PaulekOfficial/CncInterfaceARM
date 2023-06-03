//
// Created by pawel on 02/06/2023.
//

#ifndef INTERFACE_PICO_DNSSERVER_H
#define INTERFACE_PICO_DNSSERVER_H


#include "lwip/ip_addr.h"

typedef struct dns_server_t_ {
    struct udp_pcb *udp;
    ip_addr_t ip;
} dns_server_t;

void dns_server_init(dns_server_t *d, ip_addr_t *ip);
void dns_server_deinit(dns_server_t *d);


#endif //INTERFACE_PICO_DNSSERVER_H
