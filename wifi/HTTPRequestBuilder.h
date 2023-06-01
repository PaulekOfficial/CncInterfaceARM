//
// Created by PaulekOfficial on 23/05/2023.
//

#ifndef INTERFACE_PICO_HTTPREQUESTBUILDER_H
#define INTERFACE_PICO_HTTPREQUESTBUILDER_H
#include <string>
#include <cstring>

enum HTTP_Request_Type {
    GET,
    POST,
    PUT,
    HEAD,
    DELETE,
    CONNECT,
    TRACE,
    PATCH,
    OPTIONS
};
static const char* HTTP_Request_Type_String[] = {
          "GET",
          "POST",
          "PUT",
          "HEAD",
          "DELETE",
          "CONNECT",
          "TRACE",
          "PATCH",
          "OPTIONS"
        };

enum HTTP_Content_Type {
    JSON,
    TEXT,
    MULTIPART,
    TEXT_HTML
};
static const char* HTTP_Content_Type_String[] = {
        "application/json",
        "text/plain",
        "multipart/form-data",
        "text/html; charset=utf-8"
};

class HTTPRequestBuilder {
private:
    char* host;
    uint port;
    char* connection_type;

    HTTP_Request_Type request_type;
    char* request_path;

    HTTP_Content_Type content_type;
    char* payload{};

public:
    HTTPRequestBuilder(char* host_, uint port_, char* connection_type_, HTTP_Request_Type request_type_, char* request_path_, HTTP_Content_Type content_type_)
    {
        host = host_;
        port = port_;
        connection_type = connection_type_;

        request_type = request_type_;
        request_path = request_path_;

        content_type = content_type_;
    }
    char* build_request();

    char *getHost() const;

    void setHost(char *host);

    uint getPort() const;

    void setPort(uint port);

    char *getConnectionType() const;

    void setConnectionType(char *connectionType);

    HTTP_Request_Type getRequestType() const;

    void setRequestType(HTTP_Request_Type requestType);

    char *getRequestPath() const;

    void setRequestPath(char *requestPath);

    HTTP_Content_Type getContentType() const;

    void setContentType(HTTP_Content_Type contentType);

    char *getPayload() const;

    void setPayload(char *payload);
};


#endif //INTERFACE_PICO_HTTPREQUESTBUILDER_H
