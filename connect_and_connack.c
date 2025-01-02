#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ClientID "67604637ef99673c8ad65ca8_stm32_0_1_2024122114"
#define Username "67604637ef99673c8ad65ca8_stm32"
#define Password "57a9b6cebdf0310af3adffcd9c7bdd84ec0c060f6ad492526223bcce7ac6dd3f"

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
unsigned char MQTT_Connect(const char *clientID, const char *username, const char *password);

unsigned char MQTT_Connect(const char *clientID, const char *username, const char *password) {
    mqtt_txlen = 0;
    mqtt_txbuf[mqtt_txlen++] = 0x10;
    GlobalDataLen = 10 + (ClientIDLen + 2) + (UsernameLen + 2) + (PasswordLen + 2);
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
        memset(mqtt_rxbuf, 0, sizeof(mqtt_rxbuf));
        MQTT_SendBuf(mqtt_txbuf, mqtt_txlen);
        Size = Client_GetData(Buff);
        if (Size <= 0) {
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

void MQTT_SendBuf(unsigned char *buf, size_t len) { Client_SendData(buf, len); }

int Client_SendData(unsigned char *buf, size_t len) {
    if (send(sockfd, buf, len, 0) == -1) {
        perror("Client数据发送失败");
        close(sockfd);
        return -1;
    }
    return 0;
}

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