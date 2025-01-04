#include "globals.h"
#include <string.h>

void MQTT_Init(void) {
    // 缓冲区赋值
    mqtt_rxlen = sizeof(mqtt_rxbuf);
    mqtt_txlen = sizeof(mqtt_txbuf);
    memset(mqtt_rxbuf, 0, mqtt_rxlen);
    memset(mqtt_txbuf, 0, mqtt_txlen);
}

