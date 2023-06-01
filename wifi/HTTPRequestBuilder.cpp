//
// Created by PaulekOfficial on 23/05/2023.
//

#include <sstream>
#include "HTTPRequestBuilder.h"

HTTPRequestBuilder::HTTPRequestBuilder(char* host_, double battery0_, double battery1_, char* status_)
{
    host = host_;
}

char* HTTPRequestBuilder::build_request()
{
    std::stringstream stream;

    stream << HTTP_Request_Type_String[request_type];
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
        stream << sizeof(payload);
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
