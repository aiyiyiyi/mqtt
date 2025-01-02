#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "connect_and_connack.h"

#define SERVER_IP "117.78.5.125"
#define SERVER_PORT 1883

extern const unsigned char parket_subAck[];

unsigned char Buff[256];
int sockfd;
size_t mqtt_txlen = 0;
unsigned char mqtt_txbuf[256];
size_t mqtt_rxlen = 0;
unsigned char mqtt_rxbuf[1024 * 1024];
int ClientIDLen = sizeof(ClientID) - 1;
int UsernameLen = sizeof(Username) - 1;
int PasswordLen = sizeof(Password) - 1;
size_t GlobalDataLen;
size_t Size = 0;
char mqtt_message[1024];
double TEMP = 10.0;

// 模拟 MQTT 发布数据函数
unsigned char MQTT_PublishData(char *topic, char *message, unsigned char qos) {
    unsigned int topicLength = (unsigned int)strlen(topic);
    unsigned int messageLength = (unsigned int)strlen(message);
    size_t localDataLen;
    unsigned short id = 0;

    printf("上报JSON消息长度为:%d\n", messageLength);
    printf("message:%s\r\n", message);

    mqtt_txlen = 0;
    mqtt_txbuf[mqtt_txlen++] = 0x30;

    if (qos) {
        localDataLen = (topicLength + 2) + 2 + messageLength;
    } else {
        localDataLen = (topicLength + 2) + messageLength;
    }

    do {
        unsigned char encodedByte = (localDataLen % 128);
        localDataLen = localDataLen / 128;
        if (localDataLen > 0) {
            encodedByte |= 128;
        }
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    } while (localDataLen > 0);

    mqtt_txbuf[mqtt_txlen++] = 0;
    mqtt_txbuf[mqtt_txlen++] = (qos << 1) | (1 << 3);
    mqtt_txbuf[mqtt_txlen++] = (topicLength >> 8) & 0xFF;
    mqtt_txbuf[mqtt_txlen++] = topicLength & 0xFF;
    memcpy(&mqtt_txbuf[mqtt_txlen], topic, topicLength);
    mqtt_txlen += topicLength;

    if (qos) {
        mqtt_txbuf[mqtt_txlen++] = (id >> 8) & 0xFF;
        mqtt_txbuf[mqtt_txlen++] = id & 0xFF;
        id++;
    }

    memcpy(&mqtt_txbuf[mqtt_txlen], message, messageLength);
    mqtt_txlen += messageLength;

    if (send(sockfd, mqtt_txbuf, mqtt_txlen, 0) < 0) {
        perror("send");
        return 0;
    }

    return mqtt_txlen;
}