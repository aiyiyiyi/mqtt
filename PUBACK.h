#ifndef PUBACK_H
#define PUBACK_H

int Client_GetData(unsigned char *buffer);
int Client_SendData(unsigned char *buf, size_t len);
void MQTT_SendBuf(unsigned char *buf, size_t len);
unsigned char SubscribeTopic(
    char *topic, unsigned char qos, unsigned char whether);

#endif