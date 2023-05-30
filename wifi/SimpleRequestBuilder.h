//
// Created by PaulekOfficial on 23/05/2023.
//

#ifndef INTERFACE_PICO_SIMPLEREQUESTBUILDER_H
#define INTERFACE_PICO_SIMPLEREQUESTBUILDER_H
#include <string>
#include <cstring>

#define TLS_CLIENT_SERVER        "api.pauleklab.com"

class SimpleRequestBuilder {
private:
    double battery0;
    double battery1;
    std::string status;

public:
    SimpleRequestBuilder(double battery0_, double battery1_, std::string status_);

    char* getUrl();

    char* getConnectionBehaviur();

    char* getHost();
};


#endif //INTERFACE_PICO_SIMPLEREQUESTBUILDER_H
