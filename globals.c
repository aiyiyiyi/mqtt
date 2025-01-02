#include "globals.h"

size_t mqtt_txlen = 0;
size_t mqtt_rxlen = 0;
int ClientIDLen = sizeof(ClientID) - 1;
int UsernameLen = sizeof(Username) - 1;
int PasswordLen = sizeof(Password) - 1;
size_t Size = 0;
size_t GlobalDataLen = 0;  // 定义 GlobalDataLen
double TEMP = 10.0;

unsigned char mqtt_txbuf[256];
unsigned char mqtt_rxbuf[1024 * 1024];
unsigned char Buff[256];  // 定义 Buff
int sockfd;
char mqtt_message[1024];

const unsigned char parket_connectAck[] = {0x20, 0x02, 0x00, 0x00};
const unsigned char parket_disconnect[] = {0xe0, 0x00};
const unsigned char parket_heart[] = {0xc0, 0x00};
const unsigned char parket_heart_reply[] = {0xc0, 0x00};
const unsigned char parket_subAck[] = {0x90, 0x03};
