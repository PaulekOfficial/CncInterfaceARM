#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

#define TLS_CLIENT_SERVER        "worldtimeapi.org"
#define TLS_CLIENT_HTTP_REQUEST  "GET /api/ip HTTP/1.1\r\n" \
                                 "Host: " TLS_CLIENT_SERVER "\r\n" \
                                 "Connection: close\r\n" \
                                 "\r\n"
#define TLS_CLIENT_TIMEOUT_SECS  15


typedef struct TLS_CLIENT_T_ {
    struct altcp_pcb *pcb;
    bool complete;
} TLS_CLIENT_T;

static struct altcp_tls_config *tls_config = NULL;

static err_t tls_client_close(void *arg);

static err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err);

static err_t tls_client_poll(void *arg, struct altcp_pcb *pcb);

static void tls_client_err(void *arg, err_t err);

static err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err);

static void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, TLS_CLIENT_T *state);

static void tls_client_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg);


static bool tls_client_open(const char *hostname, void *arg);

// Perform initialisation
static TLS_CLIENT_T* tls_client_init(void);

void run_tls_client_test(void);