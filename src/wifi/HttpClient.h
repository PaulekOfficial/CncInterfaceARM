#ifndef CNCSMARTINTERFACE_HTTPCLIENT_H
#define CNCSMARTINTERFACE_HTTPCLIENT_H

#include <utils/utils.h>
#include <pico/stdlib.h>
#include <string>

enum RequestType {HEAD = 1, GET = 2, POST = 3, PUT = 4, DELETE = 5};
enum ContentType {XFORM = 0, JSON = 1, FORM_DATA = 2, XML = 3};
enum TransportType {HTTP_TRANSPORT_OVER_TCP = 0, HTTP_TRANSPORT_OVER_SSL = 1};

class HttpClient {

private:
    string url;
    string host;
    string path;
    string data;
    string reqHeader;
    RequestType requestType;
    ContentType contentType;
    TransportType transportType;

public:
    HttpClient(ContentType content, TransportType transport);

    void sendRequest();
    void setUrl(string url);
    void setHost(string host);
    void setPath(string path);
    void setData(string data);
    void setRequestHeader(string reqHeader);
    void setRequestType(RequestType type);
    void setContentType(ContentType type);
    void setTransportType(TransportType type);

private:
    string buildCommand();
    bool writeToESP();
    string getPureResonse();
};

#endif //CNCSMARTINTERFACE_HTTPCLIENT_H
