#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>      // DNS解析
#include <arpa/inet.h>  // IP地址处理
#include <unistd.h>
#include "connect_and_connack.h"
#include "PUBACK.h"
#include "PUBLISH.h"

#define SERVER_IP "117.78.5.125"
#define SERVER_PORT 1883

// MQTT三元组
#define ClientID "67604637ef99673c8ad65ca8_stm32_0_1_2024122114"
#define Username "67604637ef99673c8ad65ca8_stm32"
#define Password \
    "57a9b6cebdf0310af3adffcd9c7bdd84ec0c060f6ad492526223bcce7ac6dd3f"

// 订阅主题:
#define SET_TOPIC \
    "$oc/devices/67604637ef99673c8ad65ca8_stm32/sys/messages/down"  // 订阅
// 发布主题:
#define POST_TOPIC \
    "$oc/devices/67604637ef99673c8ad65ca8_stm32/sys/properties/report"  // 发布

void MQTT_SendBuf(unsigned char *buf, size_t len);
int Client_GetData(unsigned char *buffer);
int Client_SendData(unsigned char *buf, size_t len);

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

typedef enum {
    // 名字 	    值 			报文流动方向 	描述
    M_RESERVED1 = 0,  //	禁止	保留
    M_CONNECT,        //	客户端到服务端	客户端请求连接服务端
    M_CONNACK,        //	服务端到客户端	连接报文确认
    M_PUBLISH,        //	两个方向都允许	发布消息
    M_PUBACK,         //	两个方向都允许	QoS 1消息发布收到确认
    M_PUBREC,  //	两个方向都允许	发布收到（保证交付第一步）
    M_PUBREL,  //	两个方向都允许	发布释放（保证交付第二步）
    M_PUBCOMP,  //	两个方向都允许	QoS 2消息发布完成（保证交互第三步）
    M_SUBSCRIBE,    //	客户端到服务端	客户端订阅请求
    M_SUBACK,       //	服务端到客户端	订阅请求报文确认
    M_UNSUBSCRIBE,  //	客户端到服务端	客户端取消订阅请求
    M_UNSUBACK,     //	服务端到客户端	取消订阅报文确认
    M_PINGREQ,      //	客户端到服务端	心跳请求
    M_PINGRESP,     //	服务端到客户端	心跳响应
    M_DISCONNECT,   //	客户端到服务端	客户端断开连接
    M_RESERVED2,    //	禁止	保留
} _typdef_mqtt_message;

// 连接成功服务器回应 20 02 00 00
// 客户端主动断开连接 e0 00
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

/*unsigned char MQTT_Connect(
    const char *clientID, const char *username, const char *password) {
    mqtt_txlen = 0;  // 重置缓冲区长度
    mqtt_txbuf[mqtt_txlen++] = 0x10;
    GlobalDataLen =
        10 + (ClientIDLen + 2) + (UsernameLen + 2) + (PasswordLen + 2);
    size_t remainingLength = GlobalDataLen;
    do {
        unsigned char encodeByte = remainingLength % 128;
        remainingLength = remainingLength / 128;
        if (remainingLength > 0) {
            encodeByte = encodeByte | 128;
        }
        mqtt_txbuf[mqtt_txlen++] = encodeByte;
    } while (remainingLength > 0);

    mqtt_txbuf[mqtt_txlen++] = 0;
    mqtt_txbuf[mqtt_txlen++] = 4;
    mqtt_txbuf[mqtt_txlen++] = 'M';
    mqtt_txbuf[mqtt_txlen++] = 'Q';
    mqtt_txbuf[mqtt_txlen++] = 'T';
    mqtt_txbuf[mqtt_txlen++] = 'T';
    mqtt_txbuf[mqtt_txlen++] = 4;
    mqtt_txbuf[mqtt_txlen++] = 0xc2;
    mqtt_txbuf[mqtt_txlen++] = 0;
    mqtt_txbuf[mqtt_txlen++] = 120;

    mqtt_txbuf[mqtt_txlen++] = (ClientIDLen >> 8) & 0xFF;
    mqtt_txbuf[mqtt_txlen++] = ClientIDLen & 0xFF;
    memcpy(&mqtt_txbuf[mqtt_txlen], clientID, ClientIDLen);
    mqtt_txlen += ClientIDLen;

    if (UsernameLen > 0) {
        mqtt_txbuf[mqtt_txlen++] = (UsernameLen >> 8) & 0xFF;
        mqtt_txbuf[mqtt_txlen++] = UsernameLen & 0xFF;
        memcpy(&mqtt_txbuf[mqtt_txlen], username, UsernameLen);
        mqtt_txlen += UsernameLen;
    }

    if (PasswordLen > 0) {
        mqtt_txbuf[mqtt_txlen++] = (PasswordLen >> 8) & 0xFF;
        mqtt_txbuf[mqtt_txlen++] = PasswordLen & 0xFF;
        memcpy(&mqtt_txbuf[mqtt_txlen], password, PasswordLen);
        mqtt_txlen += PasswordLen;
    }

    for (int i = 0; i < 5; i++) {
        // 清空接收缓冲区
        memset(mqtt_rxbuf, 0, sizeof(mqtt_rxbuf));
        // 发送连接请求
        MQTT_SendBuf(mqtt_txbuf, mqtt_txlen);
        // 获取服务器返回数据
        Size = Client_GetData(Buff);
        if (Size <= 0) {  // 添加检查
            if (Size == 0) {
                printf("未收到数据，重试...\n");
            } else {
                printf("接收数据失败，重试...\n");
            }
            continue;
        }
        if ((size_t)Size > sizeof(mqtt_rxbuf)) {
            printf("接收到的数据超过缓冲区大小，丢弃数据...\n");
            continue;
        }
        memcpy(mqtt_rxbuf, Buff, Size);
        printf("登录响应: \n");
        for (size_t j = 0; j < Size; j++) {
            printf("%#X ", Buff[j]);
        }
        printf("\n");
        if (Size >= 4 && mqtt_rxbuf[0] == parket_connectAck[0] &&
            mqtt_rxbuf[1] == parket_connectAck[1]) {
            return 0;
        }
    }
    return -1;
}

// 发送数据到服务器
void MQTT_SendBuf(unsigned char *buf, size_t len) { Client_SendData(buf, len); }

int Client_SendData(unsigned char *buf, size_t len) {
    if (send(sockfd, buf, len, 0) == -1) {
        perror("Client数据发送失败");
        close(sockfd);
        return -1;
    }
    return 0;
}

// 接收数据从服务器
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
}

// 模拟 MQTT 发布数据函数
unsigned char MQTT_PublishData(char *topic, char *message, unsigned char qos) {
    unsigned int topicLength = (unsigned int)strlen(topic);      // 主题长度
    unsigned int messageLength = (unsigned int)strlen(message);  // 消息长度
    size_t localDataLen;  // 保存最终长度
    unsigned short id = 0;

    printf("上报JSON消息长度为:%d\n", messageLength);
    printf("message:%s\r\n", message);

    // 固定报头（包括剩余长度）
    mqtt_txlen = 0;
    mqtt_txbuf[mqtt_txlen++] =
        0x30;  // 这里应该是0x30，表示PUBLISH报文类型和标志位

    if (qos) {
        localDataLen = (topicLength + 2) + 2 +
                       messageLength;  // QoS 1或2时需要额外的2字节报文标识符
    } else {
        localDataLen = (topicLength + 2) + messageLength;
    }

    // 编码数据长度
    do {
        unsigned char encodedByte = (localDataLen % 128);
        localDataLen = localDataLen / 128;
        if (localDataLen > 0) {
            encodedByte |= 128;  // 注意这里是或运算符，不是赋值运算符
        }
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    } while (localDataLen > 0);

    // 可变报头
    mqtt_txbuf[mqtt_txlen++] = 0;                      // DUP标志位
    mqtt_txbuf[mqtt_txlen++] = (qos << 1) | (1 << 3);  // QoS和RETAIN标志位
    mqtt_txbuf[mqtt_txlen++] = (topicLength >> 8) & 0xFF;  // 主题长度高字节
    mqtt_txbuf[mqtt_txlen++] = topicLength & 0xFF;  // 主题长度低字节
    memcpy(&mqtt_txbuf[mqtt_txlen], topic,
        topicLength);           // 将主题消息拷贝到缓冲区中
    mqtt_txlen += topicLength;  // 更新缓冲区长度

    // 报文标识符
    if (qos) {
        mqtt_txbuf[mqtt_txlen++] = (id >> 8) & 0xFF;  // 报文标识符高字节
        mqtt_txbuf[mqtt_txlen++] = id & 0xFF;  // 报文标识符低字节
        id++;  //
将报文标识符id递增，以确保每个发送的PUBLISH报文都有一个唯一的标识符
    }

    // 有效载荷
    memcpy(&mqtt_txbuf[mqtt_txlen], message, messageLength);
    mqtt_txlen += messageLength;

    if (send(sockfd, mqtt_txbuf, mqtt_txlen, 0) < 0) {
        perror("send");
        return 0;
    }

    return mqtt_txlen;
}

unsigned char SubscribeTopic(
    char *topic, unsigned char qos, unsigned char whether) {
    int i, j;
    size_t localDataLen = 0;
    unsigned int topicLength = (unsigned int)strlen(topic);
    int SIZE = 0;  // 在函数开头声明
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
        //将接收缓冲区 mqtt_rxbuf 的前 mqtt_rxlen 个字节全部设置为
         //* 0，即清空该缓冲区
        memset(mqtt_rxbuf, 0, sizeof(mqtt_rxbuf));
        send(sockfd, mqtt_txbuf, mqtt_txlen, 0);  // 使用send函数发送数据
        SIZE = recv(sockfd, buff, sizeof(buff), 0);  // 使用recv函数接收数据

        // 接收服务器响应并打印服务器响应
        if (SIZE <= 0) {  // 添加检查
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
}*/