//
// Created by PaulekOfficial on 23/05/2023.
//

#include <sstream>
#include "HTTPRequestBuilder.h"

const char* HTTPRequestBuilder::build_request()
{
    std::stringstream stream;

    stream << HTTP_Request_Type_String[request_type];
    stream << " ";
    stream << request_path;
    stream << " HTTP/1.1\r\n";
    stream << "Host: ";
    stream << host;
    stream << "\r\n";

    stream << "Content-Type: ";
    stream << HTTP_Content_Type_String[content_type];
    stream << "\r\n";

    if (connection_type[0] != '\0')
    {
        stream << "Connection: ";
        stream << connection_type;
        stream << "\r\n";
    }

    if (payload[0] != '\0')
    {
        stream << "Content-Length: ";
        stream << strlen(payload);
        stream << "\r\n";
        stream << "\r\n";
        stream << payload;
    }


    std::string request = stream.str();

    const int length = request.length();

    // declaring character array (+1 for null terminator)
    char* char_array = new char[length + 1];

    // copying the contents of the
    // string to char array
    strcpy(char_array, request.c_str());

    return char_array;
}

const char *HTTPRequestBuilder::getHost() const {
    return host;
}

void HTTPRequestBuilder::setHost(const char *host) {
    HTTPRequestBuilder::host = host;
}

uint HTTPRequestBuilder::getPort() const {
    return port;
}

void HTTPRequestBuilder::setPort(uint port) {
    HTTPRequestBuilder::port = port;
}

const char *HTTPRequestBuilder::getConnectionType() const {
    return connection_type;
}

void HTTPRequestBuilder::setConnectionType(const char *connectionType) {
    connection_type = connectionType;
}

HTTP_Request_Type HTTPRequestBuilder::getRequestType() const {
    return request_type;
}

void HTTPRequestBuilder::setRequestType(HTTP_Request_Type requestType) {
    request_type = requestType;
}

const char *HTTPRequestBuilder::getRequestPath() const {
    return request_path;
}

void HTTPRequestBuilder::setRequestPath(const char *requestPath) {
    request_path = requestPath;
}

HTTP_Content_Type HTTPRequestBuilder::getContentType() const {
    return content_type;
}

void HTTPRequestBuilder::setContentType(HTTP_Content_Type contentType) {
    content_type = contentType;
}

const char *HTTPRequestBuilder::getPayload() const {
    return payload;
}

void HTTPRequestBuilder::setPayload(const char *payload) {
    HTTPRequestBuilder::payload = payload;
}
