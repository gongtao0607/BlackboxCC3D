#ifndef _LOG_H_
#define _LOG_H_
#ifdef __cplusplus
extern void log_init();
extern void log_1(uint8_t);
extern void log_1(uint16_t);
extern void log_1(float);
extern void log_sync();
#endif
#ifdef __cplusplus
extern "C" {
#endif
void log_putchar(const char);
void log_putchar(const char);
void log_send(const unsigned char*, unsigned long);
#ifdef __cplusplus
}
#endif
#endif
