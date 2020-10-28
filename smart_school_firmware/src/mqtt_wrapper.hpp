#ifndef __MQTT_WRAPPER_HPP__
#define __MQTT_WRAPPER_HPP__

#include "mbed.h"
#include "log.hpp"

#include "MQTTPacket.h"
#include "MQTTClient.h"
#include "MQTTmbed.h"

typedef void (*message_callback_t)(MQTT::MessageData &);

class MqttNetwork {
private:
    Socket *socket;

public:
    explicit MqttNetwork(Socket *socket) : socket(socket) { }

    int connect(const char *hostname, int port) {
        SocketAddress sockAddr(hostname, port);
        return socket->connect(sockAddr);
    }

    int disconnect() {
        return socket->close();
    }

    int write(unsigned char *buffer, int len, int timeout) {
        nsapi_size_or_error_t rc;
        socket->set_timeout(timeout);
        rc = socket->send(buffer, len);
        if (rc == NSAPI_ERROR_WOULD_BLOCK) {
            // time out and no data
            // MQTTClient.writePacket() requires 0 on time out and no data.
            return 0;
        }
        if (rc == 0) {
            // The socket is closed so indicate this to MQTTClient
            return -1;
        }
        return rc;
    }

    int read(unsigned char *buffer, int len, int timeout) {
        nsapi_size_or_error_t rc;
        socket->set_timeout(timeout);
        rc = socket->recv(buffer, len);
        if (rc == NSAPI_ERROR_WOULD_BLOCK) {
            // time out and no data
            // MQTTClient.readPacket() requires 0 on time out and no data.
            return 0;
        }
        if (rc == 0) {
            // A receive size of 0 indicates that the socket
            // was successfully closed so indicate this to MQTTClient
            return -1;
        }
        return rc;
    }
};

class MqttWrapper {
private:
    TCPSocket *socket = nullptr;
    MqttNetwork *mqttNetwork;
    MQTT::Client<MqttNetwork, Countdown, MBED_CONF_MQTT_WRAPPER_MAX_PACKET_SIZE, MBED_CONF_MQTT_WRAPPER_MAX_CONNECTIONS> *mqttClient;
    bool initialized = false;

    MqttWrapper() {
        logif("\n");
        mqttNetwork = new MqttNetwork(socket);
        mqttClient = new MQTT::Client<MqttNetwork, Countdown, MBED_CONF_MQTT_WRAPPER_MAX_PACKET_SIZE, MBED_CONF_MQTT_WRAPPER_MAX_CONNECTIONS> (*mqttNetwork);
    }

public:
    static MqttWrapper &getInstance() {
        static MqttWrapper instance;
        return instance;
    }

    void init(NetworkInterface *networkInterface, const char *hostname, uint16_t port) {
        logif("\n");
        // check if client exist
        if (mqttClient == nullptr) return NSAPI_ERROR_NO_CONNECTION;
        // check if initialized
        if (isInitialized()) return NSAPI_ERROR_IN_PROGRESS;

        socket = new TCPSocket;

        SocketAddress socketAddress;
        networkInterface->gethostbyname(hostname, &socketAddress);
        socketAddress.set_port(port);

        int openResult = socket->open(networkInterface);
        if (openResult != NSAPI_ERROR_OK) return openResult;

        int connectResult = socket->connect(socketAddress);
        if (connectResult != NSAPI_ERROR_OK) return connectResult;

        initialized = true;

        return NSAPI_ERROR_OK;
    }

    bool isInitialized() {
        if (mqttClient != nullptr) {
            return initialized;
        } else {
            return false;
        }
    }

    nsapi_error_t deinitialize() {
        // check if client exist
        if (mqttClient == nullptr) return NSAPI_ERROR_NO_CONNECTION;
        // check if connected
        if (isConnected()) return NSAPI_ERROR_IS_CONNECTED;
        // check if initialized
        if (isInitialized()) return NSAPI_ERROR_NO_SOCKET;

        //force socket recv to timeout
        socket->set_timeout(1);
        //wait for timeout
        ThisThread::sleep_for(10ms);
        //close the socket and deallocate memory
        socket->close();
        delete socket;
        socket = nullptr;
    }

    nsapi_error_t connect(const char* username, const char *password, const char *clientId) {
        // check if client exist
        if (mqttClient == nullptr) return NSAPI_ERROR_NO_CONNECTION;
        // check if connected
        if (isConnected()) return NSAPI_ERROR_IS_CONNECTED;
        // check if initialized
        if (isInitialized()) return NSAPI_ERROR_NO_SOCKET;

        // default data initializer
        MQTTPacket_connectData options = MQTTPacket_connectData_initializer;

        options.MQTTVersion = 3;
        options.username = {(char*)username, {0 , nullptr}};
        options.password = {(char*)password, {0 , nullptr}};
        options.clientID = {(char*)clientId, {0 , nullptr}};

        nsapi_error_t returnCode = mqttClient->connect(options);
        return returnCode < 0 ? NSAPI_ERROR_NO_CONNECTION : returnCode;
    }

    bool isConnected() {
        if (mqttClient != nullptr) {
            return mqttClient->isConnected();
        } else {
            return false;
        }
    }

    nsapi_error_t disconnect() {
        // check if client exist
        if (mqttClient == nullptr) return NSAPI_ERROR_NO_CONNECTION;
        // check if connected
        if (isConnected()) return NSAPI_ERROR_IS_CONNECTED;
        // check if initialized
        if (isInitialized()) return NSAPI_ERROR_NO_SOCKET;

        nsapi_error_t returnCode = mqttClient->disconnect();
        return returnCode < 0 ? NSAPI_ERROR_NO_CONNECTION : returnCode;
    }

    nsapi_error_t publish(const char *topic, void *payload, uint16_t payloadLen) {
        // check if client exist
        if (mqttClient == nullptr) return NSAPI_ERROR_NO_CONNECTION;
        // check if connected
        if (isConnected()) return NSAPI_ERROR_IS_CONNECTED;
        // check if initialized
        if (isInitialized()) return NSAPI_ERROR_NO_SOCKET;

        MQTT::Message message {
                .qos = MQTT::QOS0,
                .retained = false,
                .dup = false,
        };

        message.payload = payload;
        message.payloadlen = payloadLen;

        nsapi_error_t returnCode = mqttClient->publish(topic, message);
        return returnCode < 0 ? NSAPI_ERROR_NO_CONNECTION : returnCode;
    }

    nsapi_error_t subscribe(const char *topic, message_callback_t listener) {
        // check if client exist
        if (mqttClient == nullptr) return NSAPI_ERROR_NO_CONNECTION;
        // check if connected
        if (isConnected()) return NSAPI_ERROR_IS_CONNECTED;
        // check if initialized
        if (isInitialized()) return NSAPI_ERROR_NO_SOCKET;

        nsapi_error_t returnCode = mqttClient->subscribe(topic, MQTT::QOS0, listener);
        return returnCode < 0 ? NSAPI_ERROR_NO_CONNECTION : returnCode;
    }

    nsapi_error_t unsubscribe(const char *topic) {
        // check if client exist
        if (mqttClient == nullptr) return NSAPI_ERROR_NO_CONNECTION;
        // check if connected
        if (isConnected()) return NSAPI_ERROR_IS_CONNECTED;
        // check if initialized
        if (isInitialized()) return NSAPI_ERROR_NO_SOCKET;

        nsapi_error_t returnCode = mqttClient->unsubscribe(topic);
        return returnCode < 0 ? NSAPI_ERROR_NO_CONNECTION : returnCode;
    }

    void yield(uint64_t millis) {
        if (mqttClient == nullptr) return;

        mqttClient->yield(millis);
    }

    static const char *getNsapiErrorString(nsapi_error error) {
        switch (error) {
            case NSAPI_ERROR_OK: {
                return "NSAPI_ERROR_OK";
            }
            case NSAPI_ERROR_WOULD_BLOCK: {
                return "NSAPI_ERROR_WOULD_BLOCK";
            }
            case NSAPI_ERROR_UNSUPPORTED: {
                return "NSAPI_ERROR_UNSUPPORTED";
            }
            case NSAPI_ERROR_PARAMETER: {
                return "NSAPI_ERROR_PARAMETER";
            }
            case NSAPI_ERROR_NO_CONNECTION: {
                return "NSAPI_ERROR_NO_CONNECTION";
            }
            case NSAPI_ERROR_NO_SOCKET: {
                return "NSAPI_ERROR_NO_SOCKET";
            }
            case NSAPI_ERROR_NO_ADDRESS: {
                return "NSAPI_ERROR_NO_ADDRESS";
            }
            case NSAPI_ERROR_NO_MEMORY: {
                return "NSAPI_ERROR_NO_MEMORY";
            }
            case NSAPI_ERROR_NO_SSID: {
                return "NSAPI_ERROR_NO_SSID";
            }
            case NSAPI_ERROR_DNS_FAILURE: {
                return "NSAPI_ERROR_DNS_FAILURE";
            }
            case NSAPI_ERROR_DHCP_FAILURE: {
                return "NSAPI_ERROR_DHCP_FAILURE";
            }
            case NSAPI_ERROR_AUTH_FAILURE: {
                return "NSAPI_ERROR_AUTH_FAILURE";
            }
            case NSAPI_ERROR_DEVICE_ERROR: {
                return "NSAPI_ERROR_DEVICE_ERROR";
            }
            case NSAPI_ERROR_IN_PROGRESS: {
                return "NSAPI_ERROR_IN_PROGRESS";
            }
            case NSAPI_ERROR_ALREADY: {
                return "NSAPI_ERROR_ALREADY";
            }
            case NSAPI_ERROR_IS_CONNECTED: {
                return "NSAPI_ERROR_IS_CONNECTED";
            }
            case NSAPI_ERROR_CONNECTION_LOST: {
                return "NSAPI_ERROR_CONNECTION_LOST";
            }
            case NSAPI_ERROR_CONNECTION_TIMEOUT: {
                return "NSAPI_ERROR_CONNECTION_TIMEOUT";
            }
            case NSAPI_ERROR_ADDRESS_IN_USE: {
                return "NSAPI_ERROR_ADDRESS_IN_USE";
            }
            case NSAPI_ERROR_TIMEOUT: {
                return "NSAPI_ERROR_TIMEOUT";
            }
            case NSAPI_ERROR_BUSY: {
                return "NSAPI_ERROR_BUSY";
            }
        }
    }

};

#endif