#ifndef __MQTT_WRAPPER_HPP__
#define __MQTT_WRAPPER_HPP__

#include "mbed.h"
#include "log.hpp"

class MqttWrapper {
private:
    MqttWrapper() {
        logif("\n");
    }

public:
    static MqttWrapper &getInstance() {
        static MqttWrapper instance;
        return instance;
    }

    void init() {
        logif("\n");
    }

};

#endif