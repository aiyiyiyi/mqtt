#ifndef CONNECT_AND_CONNACK_H
#define CONNECT_AND_CONNACK_H

void MQTT_SendBuf(unsigned char *buf, unsigned int len);
int Client_GetData(unsigned char *buffer);
int Client_SendData(unsigned char *buf, unsigned int len);
unsigned char MQTT_Connect(
    const char *clientID, const char *username, const char *password);
#endif