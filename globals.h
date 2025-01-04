/*在 globals.h 中声明所有全局变量*/

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stddef.h>


#define SERVER_IP "117.78.5.125"
#define SERVER_PORT 1883

#define ClientID "67604637ef99673c8ad65ca8_stm32_0_1_2024122114"
#define Username "67604637ef99673c8ad65ca8_stm32"
#define Password \
    "57a9b6cebdf0310af3adffcd9c7bdd84ec0c060f6ad492526223bcce7ac6dd3f"

#define SET_TOPIC "$oc/devices/67604637ef99673c8ad65ca8_stm32/sys/messages/down"
#define POST_TOPIC \
    "$oc/devices/67604637ef99673c8ad65ca8_stm32/sys/properties/report"

extern size_t mqtt_txlen;
extern size_t mqtt_rxlen;
extern int ClientIDLen;
extern int UsernameLen;
extern int PasswordLen;
extern size_t Size;
extern double TEMP;
extern size_t GlobalDataLen;  // 添加 GlobalDataLen 的声明

extern unsigned char mqtt_txbuf[256];
extern unsigned char mqtt_rxbuf[1024 * 1024];
extern unsigned char Buff[256];  // 添加 Buff 的声明
extern int sockfd;
extern char mqtt_message[1024];


extern const unsigned char parket_connectAck[];
extern const unsigned char parket_disconnect[];
extern const unsigned char parket_heart[];
extern const unsigned char parket_heart_reply[];
extern const unsigned char parket_subAck[];

#endif  // GLOBALS_H