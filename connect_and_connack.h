#ifndef CONNECT_AND_CONNACK_H
#define CONNECT_AND_CONNACK_H

int tcp_socket();
void MQTT_SendBuf(unsigned char *buf, size_t len);
int Client_GetData(unsigned char *buffer);
int Client_SendData(unsigned char *buf, size_t len);
unsigned char MQTT_Connect(
    const char *clientID, const char *username, const char *password);

#endif