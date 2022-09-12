#include <wifi/ESP8266.h>

list<string> ESP8266::listNetworks() {
    string command = (string) COMMAND_BASE + COMMAND_LIST_NETWORKS;
    sendSerialMessage(command, uart_id);
    list<string> networks = collectSerialMessagesTillOK(10000);
    return networks;
}

CONNECTION_STATUS ESP8266::getConnectionStatus() {
    string command = (string) COMMAND_BASE + COMMAND_CONNECTION_STATUS;
    sendSerialMessage(command, uart_id);

    string rawStatus = waitUntilSerialStartsWith("STATUS:", 10000);
    replace(rawStatus, "STATUS:", "");
    replace(rawStatus, "\r\n", "");


    info(rawStatus);
    int status = atoi(rawStatus.c_str());

    if (rawStatus == "ERROR") {
        info("ESP8266 status error");
        return NotConnecting;
    }

    switch (status) {
        case 0:
            return Client;
        case 1:
            return Server;
        case 2:
            return ConnectedWitchIp;
        case 3:
            return Transmission;
        case 4:
            return Disconnected;
        default:
            return NotConnecting;
    }
}

ESP8266::ESP8266(uart_inst* uart) {
    uart_id = uart;

    connected = false;
}

void ESP8266::showAvailableNetworks() {
    if (!moduleDetected) return;

    list<string> networks = listNetworks();

    info("Available networks list:");
    for (string s : networks) {
        info(s);
    }
}

void ESP8266::setMode(ESP_MODE mode) {
    if (!moduleDetected) return;

    string command = (string) COMMAND_BASE + COMMAND_MODE + to_string(mode);
    sendSerialMessage(command, uart_id);

    if(!waitUntilSerialOK(1000)) {
        debug("Failed to set wifi module mode");
    }
}

void ESP8266::setUARTPort(uart_inst* uart) {
    uart_id = uart;
};

void ESP8266::detectModule() {
    sendSerialMessage(COMMAND_BASE, uart_id);
    moduleDetected = waitUntilSerialOK(10000);
    if (!moduleDetected) {
        info("WARNING! No ESP8266 installed.");
    }

    if (moduleDetected) {
        info("ESP8266 detected, ready to connect.");
    }
}

bool ESP8266::resetModule() {
    info("Resetting wifi module...");
    string command = (string) COMMAND_BASE + COMMAND_RESET;
    sendSerialMessage(command, uart_id);
    bool error = waitUntilSerialOK(100000);

    if (!error) {
        info("Reset wifi fail!");
    }

    return error;
}

bool ESP8266::connect(string name, string pass) {
    if (!moduleDetected) return false;

    ssid = name;
    password = pass;

    string command = (string) COMMAND_BASE + COMMAND_CONNECT_AP + "\"" + ssid + "\"" + "," + "\"" + password + "\"";
    sendSerialMessage(command, uart_id);
    connected = waitUntilSerialOK(100000);
    if (!connected) {
        info("Failed to connect.");
    }

    if (connected) {
        info("Connected to network.");
    }

    return connected;
}

int ESP8266::pingIp(string ip) {
    if (!moduleDetected) return false;

    string command = (string) COMMAND_BASE + COMMAND_PING + "\"" + ip + "\"";
    sendSerialMessage(command, uart_id);
    string ping = waitUntilSerialFirstChar('+', 100000);
    bool error = waitUntilSerialOK(100000);

    replace(ping, "\r\n", "");
    replace(ping, "+", "");

    if (ping == "ERROR" || !error) {
        info("Ping ERROR");
        return false;
    }

    info("Ping " + ping + "ms");

    return atoi(ping.c_str());
}

string ESP8266::http(CLIENT_REQUEST requestType, CLIENT_CONTENT_TYPE contentType, string url, string host, string path, CLIENT_TRANSPORT_TYPE transportType, string data, string header) {
//    <opt>: method of HTTP client request.
//    1: HEAD
//    2: GET
//    3: POST
//    4: PUT
//    5: DELETE
//       <content-type>: data type of HTTP client request.
//    0: application/x-www-form-urlencoded
//    1: application/json
//    2: multipart/form-data
//    3: text/xml
//            <”url”>: HTTP URL. The parameter can override the <host> and <path> parameters if they are null.
//                                                                                                       <”host”>: domain name or IP address.
//                                                                                                                                   <”path”>: HTTP Path.
//                                                                                                                                                  <transport_type>: HTTP Client transport type. Default: 1.
//    1: HTTP_TRANSPORT_OVER_TCP
//    2: HTTP_TRANSPORT_OVER_SSL
//            <”data”>: If <opt> is a POST request, this parameter holds the data you send to the HTTP server. If not, this parameter does not exist, which means there is no need to input a comma to indicate this parameter.
//<http_req_header>: you can send more than one request header to the server.
    return std::__cxx11::string();
}

string ESP8266::httpPost(string url, string data) {
//    <url>: HTTP URL.
//    <length>: HTTP data length to POST. The maximum length is equal to the system allocable heap size.
//    <http_req_header_cnt>: the number of <http_req_header> parameters.
//    [<http_req_header>]: you can send more than one request header to the server.
    return std::__cxx11::string();
}

int ESP8266::httpGetSize(string url) {
//    <url>: HTTP URL.
//    <size>: HTTP resource size.
    return 0;
}

bool ESP8266::disconnect() {
    string command = (string) COMMAND_BASE + COMMAND_DISCONNECT_AP;
    sendSerialMessage(command, uart_id);
    return waitUntilSerialOK(100000);
}

bool ESP8266::dhcp(bool enable, DHCP_MODE mode) {
    string command = (string) COMMAND_BASE + COMMAND_CONNECT_AP + "\"" + to_string(int(enable)) + "\"" + "," + "\"" +
            to_string(mode) + "\"";
    sendSerialMessage(command, uart_id);
    return waitUntilSerialOK(100000);
}

bool ESP8266::autoConnect(bool enable) {
    string command = (string) COMMAND_BASE + COMMAND_AUTOCONNECT + "\"" + to_string(int(enable));
    sendSerialMessage(command, uart_id);
    return waitUntilSerialOK(100000);
}

bool ESP8266::staticIp(string ip, string gateway, string netmask, string ipv6Address) {

}

string ESP8266::getIp() {
    return std::__cxx11::string();
}

string ESP8266::getGateway() {
    return std::__cxx11::string();
}

string ESP8266::getNetmask() {
    return std::__cxx11::string();
}

string ESP8266::getMac() {
    return std::__cxx11::string();
}
