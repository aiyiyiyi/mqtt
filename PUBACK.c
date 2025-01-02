#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "connect_and_connack.h"
#include "PUBACK.h"

// MQTT三元组
#define ClientID "67604637ef99673c8ad65ca8_stm32_0_1_2024122114"
#define Username "67604637ef99673c8ad65ca8_stm32"
#define Password \
    "57a9b6cebdf0310af3adffcd9c7bdd84ec0c060f6ad492526223bcce7ac6dd3f"

extern const unsigned char parket_subAck[];

#define SERVER_IP "117.78.5.125"
#define SERVER_PORT 1883

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

// 发布确认
unsigned char SubscribeTopic(
    char *topic, unsigned char qos, unsigned char whether) {
    int i, j;
    size_t localDataLen = 0;
    unsigned int topicLength = (unsigned int)strlen(topic);
    unsigned char buff[256] = {0};
    mqtt_txlen = 0;

    // 固定报头(包含剩余长度)
    if (whether) {
        mqtt_txbuf[mqtt_txlen++] = 0x82;  // 订阅
    } else {
        mqtt_txbuf[mqtt_txlen++] = 0xA2;  // 取消订阅
    }
    localDataLen = 2 + (topicLength + 2) + (whether ? 1 : 0);

    // 编码数据长度
    do {
        unsigned char encodedByte = localDataLen % 128;
        localDataLen = localDataLen / 128;
        if (localDataLen > 0) {
            encodedByte |= 0x80;
        }
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    } while (localDataLen > 0);

    // 可变报头
    mqtt_txbuf[mqtt_txlen++] = 0;  // 保持会话标志
    mqtt_txbuf[mqtt_txlen++] = 1;  // 订阅标识符

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
    for (i = 0; i < 100; i++) {
        memset(mqtt_rxbuf, 0, sizeof(mqtt_rxbuf));
        send(sockfd, mqtt_txbuf, mqtt_txlen, 0);  // 使用send函数发送数据
        int SIZE = recv(sockfd, buff, sizeof(buff), 0);  // 使用recv函数接收数据

        // 接收服务器响应并打印服务器响应
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
        printf("订阅应答\r\n");
        for (j = 0; j < SIZE; j++) {
            printf("%#X ", mqtt_rxbuf[j]);
        }
        printf("\r\n");

        // 检查是否收到正确的订阅应答
        if (SIZE >= 3 && mqtt_rxbuf[0] == parket_subAck[0] &&
            mqtt_rxbuf[1] == parket_subAck[1]) {
            return 0;
        }
        usleep(1000 * 1000);  // 休眠1秒
    }
    return 1;
}