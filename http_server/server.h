//
// Created by pawel on 02/06/2023.
//

#ifndef INTERFACE_PICO_SERVER_H
#define INTERFACE_PICO_SERVER_H

#include <lwip/ip_addr.h>
#include <unordered_map>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define TCP_PORT 80
#define DEBUG_printf printf
#define POLL_TIME_S 5
#define HTTP_GET "GET"
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"
#define HTTP_RESPONSE_REDIRECT "HTTP/1.1 302 Redirect\nLocation: http://192.168.4.1/setup\n\n"

char* body = "<html>\n"
             "<body>\n"
             "\n"
             "<h1>Konfiguracja Interfejsu</h1>\n"
             "\n"
             "<form action=\"/setup\" method=\"get\">\n"
             "    <h3>Ustawienia sieci wifi</h3>\n"
             "    <p>wpisz dane potrzebne do dolaczenia do sieci wlan (wireless lan)</p><br>\n"
             "    <label for=\"ssid\">SSID:</label><br>\n"
             "    <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"\"><br>\n"
             "    <label for=\"pass\">Password:</label><br>\n"
             "    <input type=\"password\" id=\"pass\" name=\"password\" value=\"\"><br>\n"
             "\n"
             "    <br><br>\n"
             "    <h3>Ustawienia serwera api</h3>\n"
             "    <p>edytuj te ustawienia jezeli wiesz co robisz, wprowadzenie blednych danych moze uszkodzic urzadzenie</p><br>\n"
             "    <label for=\"hostname\">Hostname:</label><br>\n"
             "    <input type=\"text\" id=\"hostname\" name=\"hostname\" value=\"api.pauleklab.com\"><br>\n"
             "    <label for=\"port\">Port:</label><br>\n"
             "    <input type=\"text\" id=\"port\" name=\"port\" value=\"8443\"><br><br>\n"
             "\n"
             "\n"
             "    <input type=\"submit\" value=\"Przeslij\">\n"
             "</form>\n"
             "\n"
             "<p>Po kliknieciu przycisku \"Przeslij\", urzadzenie zaktualizuje ustawienia wifi, oraz przeprowadzi probe polaczenia z siecia</p>\n"
             "<p>jezeli poloczenie nie zakonczy sie pomyslnie urzadzenie zresetuje sie do ustawien fabrycznych</p>\n"
             "\n"
             "</body>\n"
             "</html>";

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
} TCP_SERVER_T;

typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[128];
    char result[1500];
    int header_len;
    int result_len;
    ip_addr_t *gw;
    TCP_SERVER_T *server;
} TCP_CONNECT_STATE_T;

static   bool kill_server = false;

static const char* ssid_;
static const char* password_;

static const char* hostname_;
static uint port_;

static err_t tcpCloseClientConnection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        assert(con_state && con_state->pcb == client_pcb);
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

static void tcpServerClose(TCP_SERVER_T *state) {
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
}

static err_t tcpServerSent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    auto *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcpServerSent %u\n", len);
    con_state->sent_len += len;
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        DEBUG_printf("all done\n");
        return tcpCloseClientConnection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}

static std::unordered_map<std::string, std::string> parseGetQuery(const std::string& query) {
    std::unordered_map<std::string, std::string> data;

    size_t startPos = 0;
    while (startPos < query.length()) {
        size_t delimiterPos = query.find('&', startPos);
        if (delimiterPos == std::string::npos) {
            delimiterPos = query.length();
        }

        size_t equalsPos = query.find('=', startPos);
        if (equalsPos != std::string::npos && equalsPos < delimiterPos) {
            std::string key = query.substr(startPos, equalsPos - startPos);
            std::string value = query.substr(equalsPos + 1, delimiterPos - equalsPos - 1);
            data[key] = value;
        }

        startPos = delimiterPos + 1;
    }

    return data;
}

static int serverContent(const char *request, const char *params, char *result, size_t max_result_len) {
    int len = 0;
    if (strncmp(request, "/setup", sizeof("/setup") - 1) == 0) {

        if (params == nullptr) {
            return snprintf(result, max_result_len, body);
        }

        DEBUG_printf("parsing params...\n");
        std::unordered_map<std::string, std::string> queryParams = parseGetQuery(params);

        for (const auto& pair : queryParams) {
            if (pair.first == "port") {
                port_ = atoi(pair.second.c_str());
            }

            if (pair.first == "hostname") {
                hostname_ = pair.second.c_str();
            }

            if (pair.first == "password") {
                password_ = pair.second.c_str();
            }

            if (pair.first == "ssid") {
                ssid_ = pair.second.c_str();
            }
        }

        DEBUG_printf("data saved!\n");
        kill_server = true;

        len = snprintf(result, max_result_len, body);
    }
    return len;
}

err_t tcpServerRecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    auto *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (!p) {
        DEBUG_printf("connection closed\n");
        return tcpCloseClientConnection(con_state, pcb, ERR_OK);
    }
    assert(con_state && con_state->pcb == pcb);
    if (p->tot_len > 0) {
        DEBUG_printf("tcpServerRecv %d err %d\n", p->tot_len, err);
#if 0
        for (struct pbuf *q = p; q != NULL; q = q->next) {
            DEBUG_printf("in: %.*s\n", q->len, q->payload);
        }
#endif
        // Copy the request into the buffer
        pbuf_copy_partial(p, con_state->headers, p->tot_len > sizeof(con_state->headers) - 1 ? sizeof(con_state->headers) - 1 : p->tot_len, 0);

        // Handle GET request
        if (strncmp(HTTP_GET, con_state->headers, sizeof(HTTP_GET) - 1) == 0) {
            char *request = con_state->headers + sizeof(HTTP_GET); // + space
            char *params = strchr(request, '?');
            if (params) {
                if (*params) {
                    char *space = strchr(request, ' ');
                    *params++ = 0;
                    if (space) {
                        *space = 0;
                    }
                } else {
                    params = NULL;
                }
            }

            // Generate content
            con_state->result_len = serverContent(request, params, con_state->result, sizeof(con_state->result));
            DEBUG_printf("Request: %s?%s\n", request, params);
            DEBUG_printf("Result: %d\n", con_state->result_len);

            // Check we had enough buffer space
            if (con_state->result_len > sizeof(con_state->result) - 1) {
                DEBUG_printf("Too much result data %d\n", con_state->result_len);
                return tcpCloseClientConnection(con_state, pcb, ERR_CLSD);
            }

            // Generate web page
            if (con_state->result_len > 0) {
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS,
                                                 200, con_state->result_len);
                if (con_state->header_len > sizeof(con_state->headers) - 1) {
                    DEBUG_printf("Too much header data %d\n", con_state->header_len);
                    return tcpCloseClientConnection(con_state, pcb, ERR_CLSD);
                }
            } else {
                // Send redirect
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_REDIRECT,
                                                 ipaddr_ntoa(con_state->gw));
                DEBUG_printf("Sending redirect %s", con_state->headers);
            }

            // Send the headers to the client
            con_state->sent_len = 0;
            err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (err != ERR_OK) {
                DEBUG_printf("failed to write header data %d\n", err);
                return tcpCloseClientConnection(con_state, pcb, err);
            }

            // Send the body to the client
            if (con_state->result_len) {
                err = tcp_write(pcb, con_state->result, con_state->result_len, 0);
                if (err != ERR_OK) {
                    DEBUG_printf("failed to write result data %d\n", err);
                    return tcpCloseClientConnection(con_state, pcb, err);
                }
            }
        }
        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcpServerPoll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_poll_fn\n");

    return tcpCloseClientConnection(con_state, pcb, ERR_OK); // Just disconnect clent?
}

static void tcpServerErr(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err_fn %d\n", err);
        tcpCloseClientConnection(con_state, con_state->pcb, err);
    }
}

static err_t tcpServerAccept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("failure in accept\n");
        return ERR_VAL;
    }
    DEBUG_printf("client connected\n");

    // Create the state for the connection
    auto *con_state = static_cast<TCP_CONNECT_STATE_T *>(calloc(1, sizeof(TCP_CONNECT_STATE_T)));
    if (!con_state) {
        DEBUG_printf("failed to allocate connect state\n");
        return ERR_MEM;
    }
    con_state->pcb = client_pcb; // for checking
    con_state->gw = &state->gw;
    con_state->server = state;

    // setup connection to client
    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcpServerSent);
    tcp_recv(client_pcb, tcpServerRecv);
    tcp_poll(client_pcb, tcpServerPoll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcpServerErr);

    if (kill_server) {
        state->complete = true;
        DEBUG_printf("Server finished\n");
    }

    return ERR_OK;
}

static bool tcpServerOpen(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    DEBUG_printf("starting server on port %u\n", TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err) {
        DEBUG_printf("failed to bind to port %d\n");
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcpServerAccept);

    return true;
}


#endif //INTERFACE_PICO_SERVER_H
