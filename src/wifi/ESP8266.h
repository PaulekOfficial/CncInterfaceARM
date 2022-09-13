#ifndef CNCSMARTINTERFACE_ESP8266_H
#define CNCSMARTINTERFACE_ESP8266_H

#endif //CNCSMARTINTERFACE_ESP8266_H
#include <utils/utils.h>
#include <pico/stdlib.h>
#include <string>

using namespace std;

#define COMMAND_BASE "AT"
#define COMMAND_RESET "+RST"
#define COMMAND_MODE "+CWMODE="
#define COMMAND_LIST_NETWORKS "+CWLAP"
#define COMMAND_CONNECT_AP "+CWJAP="
#define COMMAND_DISCONNECT_AP "+CWQAP"
#define COMMAND_CONNECTION_STATUS "+CIPSTATUS"
#define COMMAND_PING "+PING="
#define COMMAND_AUTOCONNECT "+CWAUTOCONN"
#define COMMAND_QUERY_SET_IP_ESP "+CIPSTA"
#define COMMAND_HTTP_CLIENT "+HTTPCLIENT="
#define COMMAND_HTTP_GET_SIZE "+HTTPGETSIZE="
#define COMMAND_HTTP_POST "+HTTPCPOST="

enum ESP_MODE {Station = 1, SoftAccessPoint = 2, StationAP = 3};
enum CONNECTION_STATUS {Client = 0, Server = 1, ConnectedWitchIp = 2, Transmission = 3, Disconnected = 4, NotConnecting = 5};
enum CLIENT_REQUEST {Head = 1, Get = 2, Post = 3, Put = 4, Delete = 5};
enum CLIENT_CONTENT_TYPE {ApplicationForm = 0, ApplicationJson = 1, Multipart = 2, TextXml = 3};
enum CLIENT_TRANSPORT_TYPE {OverTCP = 1, OverSSL = 2};
enum DHCP_MODE {StationM = 0, SoftAPM = 1, EthernetM = 2};

class ESPIPSettings {
public:
    string ip;
    string gateway;
    string netmask;
    string ipv6;
    string ipv6g;
};

class ESP8266 {
private:
    uart_inst* uart_id;
    string ssid;
    string password;
    string mac;
    bool moduleDetected{};
    bool connected;

    list<string> listNetworks();
    ESPIPSettings getIpSettingsESP();
public:
    explicit ESP8266(uart_inst* uart);

    bool resetModule();
    void detectModule();
    void setUARTPort(uart_inst* uart);

    void setMode(ESP_MODE mode);
    void showAvailableNetworks();
    CONNECTION_STATUS getConnectionStatus();

    bool connect(string name, string pass);
    bool disconnect();

    bool dhcp(bool enable, DHCP_MODE mode);
    bool autoConnect(bool enable);
    bool staticIp(string ip, string gateway, string netmask, string ipv6Address);

    ESPIPSettings getAddress();

    string http(CLIENT_REQUEST requestType, CLIENT_CONTENT_TYPE contentType, string url, string host, string path, CLIENT_TRANSPORT_TYPE transportType, string data, string header);
    string httpPost(string url, string data);
    int httpGetSize(string url);
    int pingIp(string ip);
};
