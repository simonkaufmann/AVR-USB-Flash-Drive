#ifndef USBDATA_H
#define USBDATA_H

void usb_send_next_data(void);
void usb_send_data(uint8_t *data, uint8_t length);
uint8_t usb_is_send_ready(void);
void usb_buffer_free(void);


#endif
