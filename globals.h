/*在 globals.h 中声明所有全局变量*/

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stddef.h>

extern size_t mqtt_txlen;
extern size_t mqtt_rxlen;
extern int ClientIDLen;
extern int UsernameLen;
extern int PasswordLen;
extern size_t Size;
extern double TEMP;

extern const unsigned char parket_connectAck[];
extern const unsigned char parket_disconnect[];
extern const unsigned char parket_heart[];
extern const unsigned char parket_heart_reply[];
extern const unsigned char parket_subAck[];

#endif  // GLOBALS_H