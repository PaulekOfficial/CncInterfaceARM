//
// Created by PaulekOfficial on 23/05/2023.
//

#ifndef INTERFACE_PICO_HTTPREQUESTBUILDER_H
#define INTERFACE_PICO_HTTPREQUESTBUILDER_H
#include <string>
#include <cstring>

const enum HTTP_Request_Type {
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

const enum HTTP_Content_Type {
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
    char* connection_type;

    HTTP_Request_Type request_type;
    char* request_path;

    HTTP_Content_Type content_type;
    char* payload;

public:
    HTTPRequestBuilder(char* host_, double battery0_, double battery1_, char* status_);

    char* build_request();

    char* get_connection_type();

    char* get_host();
};


#endif //INTERFACE_PICO_HTTPREQUESTBUILDER_H
