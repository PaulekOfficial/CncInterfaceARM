//
// Created by PaulekOfficial on 30/05/2023.
//

#ifndef INTERFACE_PICO_WIFIMANAGER_H
#define INTERFACE_PICO_WIFIMANAGER_H

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"
#include "SimpleRequestBuilder.h"

#include <string>

class WiFiManager {
private:
    std::string ssid;
    std::string password;

    int wifiConnectionTrys = 0;
    int hardResetAttempts = 0;
    bool trying_to_connect = false;
    int link_up = -1;
public:
    void set_network(std::string ssid_, std::string password_) { ssid = ssid_; password = password_; }
    void connect();
    void disconnect();
    char* http_request(char* hostname, char* request);

    int connected() { return link_up; }
    bool connecting() { return trying_to_connect; }
};


#endif //INTERFACE_PICO_WIFIMANAGER_H
