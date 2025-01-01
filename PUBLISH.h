#ifndef PUBLISH_H
#define PUBLISH_H

unsigned char MQTT_PublishData(char* topic, char* message, unsigned char qos);

#endif