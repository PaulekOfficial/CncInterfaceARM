//
// Created by PaulekOfficial on 23/05/2023.
//

#include <sstream>
#include "HTTPRequestBuilder.h"

char* HTTPRequestBuilder::build_request()
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
        stream << payload.size();
        stream << "\r\n";
        stream << "\r\n";
        stream << payload;
    }


    std::string request = stream.str();

    const uint length = request.length();

    // declaring character array (+1 for null terminator)
    char* char_array = new char[length + 1];

    // copying the contents of the
    // string to char array
    strcpy(char_array, request.c_str());

    return char_array;
}

char *HTTPRequestBuilder::getHost() const {
    return host;
}

void HTTPRequestBuilder::setHost(char *host_) {
    HTTPRequestBuilder::host = host_;
}

uint HTTPRequestBuilder::getPort() const {
    return port;
}

void HTTPRequestBuilder::setPort(uint port_) {
    HTTPRequestBuilder::port = port_;
}

char *HTTPRequestBuilder::getConnectionType() const {
    return connection_type;
}

void HTTPRequestBuilder::setConnectionType(char *connectionType) {
    connection_type = connectionType;
}

HTTP_Request_Type HTTPRequestBuilder::getRequestType() const {
    return request_type;
}

void HTTPRequestBuilder::setRequestType(HTTP_Request_Type requestType) {
    request_type = requestType;
}

char *HTTPRequestBuilder::getRequestPath() const {
    return request_path;
}

void HTTPRequestBuilder::setRequestPath(char *requestPath) {
    request_path = requestPath;
}

HTTP_Content_Type HTTPRequestBuilder::getContentType() const {
    return content_type;
}

void HTTPRequestBuilder::setContentType(HTTP_Content_Type contentType) {
    content_type = contentType;
}

std::string HTTPRequestBuilder::getPayload() const {
    return payload;
}

void HTTPRequestBuilder::setPayload(std::string payload_) {
    HTTPRequestBuilder::payload = payload_;
}
