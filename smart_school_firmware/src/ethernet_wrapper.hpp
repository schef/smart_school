#ifndef __ETHERNET_WRAPPER_HPP__
#define __ETHERNET_WRAPPER_HPP__

#include "mbed.h"
#include "log.hpp"
#include "EthernetInterface.h"

class EthernetWrapper {
private:
    EthernetInterface eth;

    EthernetWrapper() {
        logif("\n");
    }

    void statusCallback(nsapi_event_t nsapi_event, intptr_t intptr) {
        nsapi_connection_status_t nsapi_connection_status = eth.get_connection_status();
        logif("nsapi_event[%d], intptr[%d], nsapi_connection_status[%d]\n", nsapi_event, intptr, nsapi_connection_status);
    }
public:
    static EthernetWrapper &getInstance() {
        static EthernetWrapper instance;
        return instance;
    }

    void init() {
        logif("\n");
        eth.attach(callback(this, &EthernetWrapper::statusCallback));
    }

    void connect() {
        nsapi_error_t status = eth.connect();
        if (status != NSAPI_ERROR_OK) {
            logif("error[%d]\n", status);
        }
    }

    void printInfo() {
        SocketAddress address;
        eth.get_ip_address(&address);
        logif("ip[%s]\n", address.get_ip_address());
    }

    NetworkInterface *getInterface() {
        return eth.get_default_instance();
    }
};

#endif