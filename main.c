#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "connect_and_connack.h"
#include "PUBACK.h"
#include "PUBLISH.h"

#define SERVER_IP "117.78.5.125"
#define SERVER_PORT 1883

#define ClientID "67604637ef99673c8ad65ca8_stm32_0_1_2024122114"
#define Username "67604637ef99673c8ad65ca8_stm32"
#define Password \
    "57a9b6cebdf0310af3adffcd9c7bdd84ec0c060f6ad492526223bcce7ac6dd3f"

#define SET_TOPIC "$oc/devices/67604637ef99673c8ad65ca8_stm32/sys/messages/down"
#define POST_TOPIC \
    "$oc/devices/67604637ef99673c8ad65ca8_stm32/sys/properties/report"

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

const unsigned char parket_connectAck[] = {0x20, 0x02, 0x00, 0x00};
const unsigned char parket_disconnect[] = {0xe0, 0x00};
const unsigned char parket_heart[] = {0xc0, 0x00};
const unsigned char parket_heart_reply[] = {0xc0, 0x00};
const unsigned char parket_subAck[] = {0x90, 0x03};

void MQTT_SendBuf(unsigned char *buf, size_t len);
int Client_GetData(unsigned char *buffer);
int Client_SendData(unsigned char *buf, size_t len);
unsigned char MQTT_Connect(
    const char *clientID, const char *username, const char *password);
unsigned char MQTT_PublishData(char *topic, char *message, unsigned char qos);
unsigned char SubscribeTopic(
    char *topic, unsigned char qos, unsigned char whether);
void MQTT_Init(void);

int main() {
    // 创建socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket creation failed");
        return 1;
    }

    // 配置服务器地址
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(1883);
    sin.sin_addr.s_addr = inet_addr("117.78.5.125");

    // 连接服务器
    if (connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("socket连接失败");
        close(sockfd);
        return -1;
    }
    printf("Socket连接成功\n");

    // 初始化字符串长度
    MQTT_Init();
    while (1) {
        // 执行MQTT连接
        if (MQTT_Connect(ClientID, Username, Password) == 0) {
            printf("MQTT连接成功\n");
            break;  // 在成功连接后跳出循环
        } else {
            printf("MQTT连接失败\n");
            close(sockfd);
            return -1;
        }
    }
    // 测试MQTT订阅函数
    if (SubscribeTopic((char *)SET_TOPIC, 1, 1) == 1) {
        perror("MQTT订阅失败");
        close(sockfd);
        return 1;
    } else {
        printf("MQTT订阅成功\n");
    }
    while (1) {
        sprintf(mqtt_message,
            "{\"services\": [{\"service_id\": "
            "\"stm32\",\"properties\":{\"TEMP\":%.1f}}]}",
            (double)(TEMP += 0.2));  // 温度
        // 发布主题
        MQTT_PublishData((char *)POST_TOPIC, mqtt_message, 0);
        printf("发布消息成功\n");
        sleep(5);
        // 发送心跳包
        printf("发送心跳...\n");
        MQTT_SendBuf((unsigned char *)parket_heart, sizeof(parket_heart));
        sleep(10);  // 每10秒发送一次心跳包
    }

    // 断开连接
    MQTT_SendBuf((unsigned char *)parket_disconnect, sizeof(parket_disconnect));
    close(sockfd);
    return 0;
}

void MQTT_Init(void) {
    // 缓冲区赋值
    mqtt_rxlen = sizeof(mqtt_rxbuf);
    mqtt_txlen = sizeof(mqtt_txbuf);
    memset(mqtt_rxbuf, 0, mqtt_rxlen);
    memset(mqtt_txbuf, 0, mqtt_txlen);
}