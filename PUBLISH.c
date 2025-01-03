#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "connect_and_connack.h"
#include "globals.h"  // 包含 globals.h 以使用全局变量

#define SERVER_IP "117.78.5.125"
#define SERVER_PORT 1883

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