//
// Created by PaulekOfficial on 30/05/2023.
//

#ifndef INTERFACE_PICO_TLS2_CLIENT_H
#define INTERFACE_PICO_TLS2_CLIENT_H

#include <string.h>
#include <time.h>
#include <mbedtls/debug.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"
#include "HTTPRequestBuilder.h"

#define TLS_CLIENT_TIMEOUT_SECS  15

typedef struct TLS_CLIENT_T_ {
    struct altcp_pcb *pcb;
    bool complete;
} TLS_CLIENT_T;

/** Our global mbedTLS configuration (server-specific, not connection-specific) */
struct altcp_tls_config {
    mbedtls_ssl_config conf;
    mbedtls_x509_crt *cert;
    mbedtls_pk_context *pkey;
    u8_t cert_count;
    u8_t cert_max;
    u8_t pkey_count;
    u8_t pkey_max;
    mbedtls_x509_crt *ca;
#if defined(MBEDTLS_SSL_CACHE_C) && ALTCP_MBEDTLS_USE_SESSION_CACHE
    /** Inter-connection cache for fast connection startup */
  struct mbedtls_ssl_cache_context cache;
#endif
#if defined(MBEDTLS_SSL_SESSION_TICKETS) && ALTCP_MBEDTLS_USE_SESSION_TICKETS
    mbedtls_ssl_ticket_context ticket_ctx;
#endif
};

static struct altcp_tls_config *tls_config = NULL;

static char* http_request_string;
static uint portt;

class tls2_client {
};

static err_t tls_client_close(void *arg) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    err_t err = ERR_OK;

    state->complete = true;
    if (state->pcb != NULL) {
        altcp_arg(state->pcb, NULL);
        altcp_poll(state->pcb, NULL, 0);
        altcp_recv(state->pcb, NULL);
        altcp_err(state->pcb, NULL);
        err = altcp_close(state->pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            altcp_abort(state->pcb);
            err = ERR_ABRT;
        }
        state->pcb = NULL;
    }
    return err;
}

static err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tls_client_close(state);
    }

    printf("connected to server, sending request\n");
    err = altcp_write(state->pcb, http_request_string, strlen(http_request_string), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("error writing data, err=%d", err);
        return tls_client_close(state);
    }

    return ERR_OK;
}

static err_t tls_client_poll(void *arg, struct altcp_pcb *pcb) {
    printf("timed out");
    return tls_client_close(arg);
}

static void tls_client_err(void *arg, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    printf("tls_client_err %d\n", err);
    state->pcb = NULL; /* pcb freed by lwip when _err function is called */
}

static err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (!p) {
        printf("connection closed\n");
        return tls_client_close(state);
    }

    if (p->tot_len > 0) {
        /* For simplicity this examples creates a buffer on stack the size of the data pending here,
           and copies all the data to it in one go.
           Do be aware that the amount of data can potentially be a bit large (TLS record size can be 16 KB),
           so you may want to use a smaller fixed size buffer and copy the data to it using a loop, if memory is a concern */
        char buf[p->tot_len + 1];

        pbuf_copy_partial(p, buf, p->tot_len, 0);
        buf[p->tot_len] = 0;

        printf("***\nnew data received from server:\n***\n\n%s\n", buf);

        altcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);

    return ERR_OK;
}

static void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, TLS_CLIENT_T *state)
{
    err_t err;

    printf("connecting to server IP %s port %d\n", ipaddr_ntoa(ipaddr), portt);
    err = altcp_connect(state->pcb, ipaddr, portt, tls_client_connected);
    if (err != ERR_OK)
    {
        fprintf(stderr, "error initiating connect, err=%d\n", err);
        tls_client_close(state);
    }
}

static void tls_client_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr)
    {
        printf("DNS resolving complete\n");
        tls_client_connect_to_server_ip(ipaddr, (TLS_CLIENT_T *) arg);
    }
    else
    {
        printf("error resolving hostname %s\n", hostname);
        tls_client_close(arg);
    }
}

static void my_debug(void *ctx, int level, const char *file, int line,
                     const char *str)
{
    const char *p, *basename;
    (void) ctx;

    /* Extract basename from file */
    for(p = basename = file; *p != '\0'; p++) {
        if(*p == '/' || *p == '\\') {
            basename = p + 1;
        }
    }

    printf("%s:%04d: |%d| %s \n", basename, line, level, str);
}

/**
 * Certificate verification callback for mbed TLS
 * Here we only use it to display information on each cert in the chain
 */
static int my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
{
    const uint32_t buf_size = 1024;
    char *buf = new char[buf_size];
    (void) data;

    printf("\nVerifying certificate at depth %d:\n", depth);
    mbedtls_x509_crt_info(buf, buf_size - 1, "  ", crt);
    printf("%s \n", buf);

    if (*flags == 0)
        printf("No verification issue for this certificate\n");
    else
    {
        mbedtls_x509_crt_verify_info(buf, buf_size, "  ! \n", *flags);
        printf("%s\n", buf);
    }

    delete[] buf;
    return 0;
}

static bool tls_client_open(const char *hostname, uint port, void *arg) {
    err_t err;
    ip_addr_t server_ip;
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;

    state->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_V4);
    if (!state->pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, tls_client_poll, TLS_CLIENT_TIMEOUT_SECS * 2);
    altcp_recv(state->pcb, tls_client_recv);
    altcp_err(state->pcb, tls_client_err);

    /* Assign the TLS config to the TLS context. */
    if (mbedtls_ssl_setup((mbedtls_ssl_context*) altcp_tls_context(state->pcb), &tls_config->conf) != 0) {
        exit(EXIT_FAILURE);
    }

    mbedtls_ssl_conf_verify(&tls_config->conf, my_verify, nullptr);
    mbedtls_ssl_conf_dbg(&tls_config->conf, my_debug, nullptr);
    mbedtls_debug_set_threshold(4);

    /* However, we accept only TLS 1.2 and higher. */
    mbedtls_ssl_conf_min_version(&tls_config->conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                                 MBEDTLS_SSL_MINOR_VERSION_3);
    /* We need to set the option to validate the peer certificate chain.
    ** If we skipped this step, an active attacker could impersonate the server. */
    mbedtls_ssl_conf_authmode(&tls_config->conf, MBEDTLS_SSL_VERIFY_NONE);

    printf("tls2client hostname: %s \n", hostname);
    printf("tls2client port: %d \n", port);

    /* Set hostname for verification.
    ** Not setting the hostname would mean that we would accept a certificate of any trusted server.
    ** It also sets the Server Name Indication TLS extension.
    ** This is required when multiple servers are running at the same IP address (virtual hosting). */
    mbedtls_ssl_set_hostname((mbedtls_ssl_context*) altcp_tls_context(state->pcb), hostname);
    portt = port;

    printf("resolving %s \n", hostname);

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();

    err = dns_gethostbyname(hostname, &server_ip, tls_client_dns_found, state);
    if (err == ERR_OK)
    {
        /* host is in DNS cache */
        tls_client_connect_to_server_ip(&server_ip, state);
    }
    else if (err != ERR_INPROGRESS)
    {
        printf("error initiating DNS resolving, err=%d\n", err);
        tls_client_close(state->pcb);
    }

    cyw43_arch_lwip_end();

    return err == ERR_OK || err == ERR_INPROGRESS;
}

// Perform initialisation
static TLS_CLIENT_T* tls_client_init(void) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*) (calloc(1, sizeof(TLS_CLIENT_T)));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }

    return state;
}


#endif //INTERFACE_PICO_TLS2_CLIENT_H
