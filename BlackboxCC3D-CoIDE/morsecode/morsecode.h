#ifndef _MORSECODE_H_
#define _MORSECODE_H_

#define DOT_DURATION 120 //(120ms=20WPM)
#define MORSE_STRING_SIZE 12

#define ERROR_NOERROR 0
#define ERROR_RECEIVER_RECEIPTION 1


extern char morse_string[MORSE_STRING_SIZE];
extern void morse_send();
extern void morse_error_string(const char*,size_t);
extern void morse_error_code(uint8_t);
#endif
