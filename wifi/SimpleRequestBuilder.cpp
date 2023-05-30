//
// Created by PaulekOfficial on 23/05/2023.
//

#include <sstream>
#include "SimpleRequestBuilder.h"

SimpleRequestBuilder::SimpleRequestBuilder(double battery0_, double battery1_, std::string status_)
{
    battery0 = battery0_;
    battery1 = battery1_;
    status = status_;
}

char* SimpleRequestBuilder::getUrl()
{
    std::stringstream stream;

    stream << "POST /smart-interface/measurement";
    stream << " HTTP/1.2\r\n";
    stream << "Host: " TLS_CLIENT_SERVER "\r\n";
    stream << "Connection: close\r\n";
    stream << "\r\n";

    stream << "{\"interfaceUUID\":\"352da5cf-7e92-45ca-88a5-639e5dc2f592\",\"status\":\"BATTERY_MODE\",\"measurementList\": [{\"name\":\"0\", \"type\":\"BATTERY_VOLTAGE\", \"value\":\"6.14\"},{\"name\":\"1\", \"type\":\"BATTERY_VOLTAGE\", \"value\":\"2.16\"}]}";
    stream << "\r\n";

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
    return "Host: " TLS_CLIENT_SERVER "\r\n";
}

char* SimpleRequestBuilder::getConnectionBehaviur()
{
    return "Connection: close\r\n";
}
