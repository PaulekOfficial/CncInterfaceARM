//
// Created by PaulekOfficial on 23/05/2023.
//

#include <sstream>
#include "SimpleRequestBuilder.h"

SimpleRequestBuilder::SimpleRequestBuilder(char* host_, double battery0_, double battery1_, char* status_)
{
    battery0 = battery0_;
    battery1 = battery1_;
    status = status_;
    host = host_;
}

char* SimpleRequestBuilder::getUrl()
{
    std::stringstream stream;
    std::stringstream body;

    body << "{\"interfaceUUID\":\"352da5cf-7e92-45ca-88a5-639e5dc2f592\",\"status\":\"BATTERY_MODE\",\"measurementList\": [{\"name\":\"0\", \"type\":\"BATTERY_VOLTAGE\", \"value\":\"6.14\"},{\"name\":\"1\", \"type\":\"BATTERY_VOLTAGE\", \"value\":\"2.16\"}]}";
    body << "\r\n";

    stream << "POST /smart-interface/measurement";
    stream << " HTTP/1.1\r\n";
    stream << "Host: ";
    stream << host;
    stream << "\r\n";
    stream << "Content-Type: application/json\r\n";
    stream << "Content-Length: ";
    stream << body.str().length();
    stream << "\r\n";
    stream << "\r\n";
    stream << body.str();

    std::string request = stream.str();

    const int length = request.length();

    // declaring character array (+1 for null terminator)
    char* char_array = new char[length + 1];

    // copying the contents of the
    // string to char array
    strcpy(char_array, request.c_str());

    return char_array;
}

char* SimpleRequestBuilder::getHost()
{
    return host;
}

char* SimpleRequestBuilder::getConnectionBehaviur()
{
    return "Connection: close\r\n";
}
