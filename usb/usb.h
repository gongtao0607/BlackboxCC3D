#ifndef _USB_H_
#define _USB_H_
#ifdef __cplusplus
extern "C" {
#endif
extern void usb_init();
uint32_t CDC_Send_DATA (uint8_t *, uint8_t);
#ifdef __cplusplus
}
#endif
#endif
