#include "mbed.h"
#include "log.hpp"
#include "ethernet_wrapper.hpp"
#include "mqtt_wrapper.hpp"

DigitalOut led(LED1);
int main()
{
    logif("boot begin\n");
    EthernetWrapper::getInstance().init();
    EthernetWrapper::getInstance().connect();
    EthernetWrapper::getInstance().printInfo();
    MqttWrapper::getInstance().init();
    logif("boot end\n");
    while (true) {
        led = !led;
        ThisThread::sleep_for(1000ms);
    }
}