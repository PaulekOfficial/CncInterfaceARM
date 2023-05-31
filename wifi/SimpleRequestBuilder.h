//
// Created by PaulekOfficial on 23/05/2023.
//

#ifndef INTERFACE_PICO_SIMPLEREQUESTBUILDER_H
#define INTERFACE_PICO_SIMPLEREQUESTBUILDER_H
#include <string>
#include <cstring>

class SimpleRequestBuilder {
private:
    double battery0;
    double battery1;
    char* status;
    char* host;

public:
    SimpleRequestBuilder(char* host_, double battery0_, double battery1_, char* status_);

    char* getUrl();

    char* getConnectionBehaviur();

    char* getHost();
};


#endif //INTERFACE_PICO_SIMPLEREQUESTBUILDER_H
