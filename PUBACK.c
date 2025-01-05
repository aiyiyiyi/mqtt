#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "connect_and_connack.h"
#include "PUBACK.h"
#include "globals.h"

#define SERVER_IP "117.78.5.125"
#define SERVER_PORT 1883

int Client_GetData(unsigned char *buffer);
int Client_SendData(unsigned char *buf, size_t len);
void MQTT_SendBuf(unsigned char *buf, size_t len);
unsigned char SubscribeTopic(
    char *topic, unsigned char qos, unsigned char whether);

// 发布确认
unsigned char SubscribeTopic(
    char *topic, unsigned char qos, unsigned char whether) {
    int i, j;
    size_t localDataLen = 0;
    unsigned int topicLength = (unsigned int)strlen(topic);
    unsigned char buff[256] = {0};
    // mqtt_txlen = 0;

    // 固定报头(包含剩余长度)
    if (whether) {
        mqtt_txbuf[mqtt_txlen++] = 0x82;  // 订阅
    } else {
        mqtt_txbuf[mqtt_txlen++] = 0xA2;  // 取消订阅
    }
    localDataLen = 2 + (topicLength + 2) + (whether ? 1 : 0);

    // 编码数据长度
    size_t len = localDataLen;
    do {
        unsigned char encodedByte = len % 128;
        len = len / 128;
        if (len > 0) {
            encodedByte |= 0x80;
        }
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    } while (len > 0);

    // 可变报头
    mqtt_txbuf[mqtt_txlen++] = 0;     // 保持会话标志
    mqtt_txbuf[mqtt_txlen++] = 0x01;  // 订阅标识符

    // 有效载荷
    mqtt_txbuf[mqtt_txlen++] = (topicLength >> 8) & 0xFF;
    mqtt_txbuf[mqtt_txlen++] = topicLength & 0xFF;
    memcpy(&mqtt_txbuf[mqtt_txlen], topic, topicLength);
    mqtt_txlen += topicLength;

    // 判断是否订阅还是取消订阅
    if (whether) {
        mqtt_txbuf[mqtt_txlen++] = qos;
    }

    // 循环连接
    for (i = 0; i < 10; i++) {
        /*memset(mqtt_rxbuf, 0, sizeof(mqtt_rxbuf));*/
        memset(mqtt_rxbuf, 0, mqtt_rxlen);
        MQTT_SendBuf(mqtt_txbuf, mqtt_txlen);  // 使用mqtt_sendBuf函数发送数据
        int SIZE = Client_GetData(buff);  // 使用recv函数接收数据

        // 接收服务器响应并打印服务器订阅响应
        if (SIZE <= 0) {
            if (SIZE == 0) {
                printf("未收到数据，重试...\n");
            } else {
                printf("接收数据失败，重试...\n");
            }
            continue;
        }
        if ((size_t)SIZE > sizeof(mqtt_rxbuf)) {
            printf("接收到的数据超过缓冲区大小，丢弃数据...\n");
            continue;
        }
        memcpy(mqtt_rxbuf, buff, SIZE);
        printf("订阅应答\n");
        for (j = 0; j < SIZE; j++) {
            printf("%#X ", mqtt_rxbuf[j]);
        }
        printf("\n");

        // 检查是否收到正确的订阅应答
        if (mqtt_rxbuf[0] == parket_subAck[0] &&
            mqtt_rxbuf[1] == parket_subAck[1]) {
          //  printf("设备成功订阅了主题，并且服务器确认订阅请求\n");
            return 0;
        }
        usleep(1000 * 1000);  // 休眠1秒
    }
    return 1;
}
/*int Client_SendData(unsigned char *buf, size_t len) {
    if (send(sockfd, buf, len, 0) == -1) {
        perror("Client数据发送失败");
        close(sockfd);
        return -1;
    }
    return 0;
}

void MQTT_SendBuf(unsigned char *buf, size_t len) { Client_SendData(buf, len); }

int Client_GetData(unsigned char *buffer) {
    int received = recv(sockfd, buffer, 200, 0);
    if (received < 0) {
        perror("接收数据失败");
        close(sockfd);
        return -1;
    } else if (received == 0) {
        printf("服务器关闭连接\n");
        close(sockfd);
        return -1;
    } else {
        printf("数据接收成功: %d 字节\n", received);
    }
    return received;
}*/