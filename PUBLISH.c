#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>  // 用于 send 函数
#include <arpa/inet.h>   // 用于套接字操作

#define SERVER_IP "117.78.5.125"  // 服务器IP地址
#define SERVER_PORT 1883          // 服务器端口

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
        id++;  // 将报文标识符id递增，以确保每个发送的PUBLISH报文都有一个唯一的标识符
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


/*int main() {
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in sin = {0}; // 初始化结构体
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SERVER_PORT);
    sin.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sockfd, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }
    printf("连接成功\n");

    // 测试MQTT发布函数
    char* topic = "test/topic";
    char* message = "{\"key\":\"value\"}";
    unsigned char qos = 0;
    MQTT_PublishData(topic, message, qos, sockfd);

    close(sockfd);
    return 0;
}*/