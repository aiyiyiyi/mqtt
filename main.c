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
#include "globals.h"
#include "tcp.h"
#include "init.h"

int tcp_socket();
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
    /*多线程问题：如果程序使用多线程，可能在一个线程中关闭了套接字(connect_and_connack还有一个sock连接，导致错误)，而另一个线程仍在尝试使用它。*/
    // 创建socket
    if (tcp_socket() == -1) {
        return -1;
    }
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
        sleep(10);
        /**发送心跳包
        printf("发送心跳...\n");
        MQTT_SendBuf((unsigned char *)parket_heart, 2);
        sleep(5);  // 每10秒发送一次心跳包*/
    }

    // 断开连接
    MQTT_SendBuf((unsigned char *)parket_disconnect, 2);
    close(sockfd);
    return 0;
}

