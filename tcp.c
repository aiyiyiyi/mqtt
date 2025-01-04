#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "globals.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>



int tcp_socket() {
    // 创建socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket creation failed");
        return 1;
    }

    // 配置服务器地址
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SERVER_PORT);
    sin.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接服务器
    if (connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("socket连接失败");
        close(sockfd);
        return -1;
    }
    printf("Socket连接成功\n");
    return 0;
}