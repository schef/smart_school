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
    MqttWrapper::getInstance().init(EthernetWrapper::getInstance().getInterface(), "broker.emqx.io", 1883);
    MqttWrapper::getInstance().connect("exrkeina", "2zUqpbUBMZGY", "MQTT_TEST");
    MqttWrapper::getInstance().subscribe("test", onMessageReceived);
    logif("boot end\n");
    while (true) {
        led = !led;
        ThisThread::sleep_for(1000ms);
    }
}