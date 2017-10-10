#include <avr/io.h>

#include "usbdrv.h"
#include "main.h"
#include "utils.h"
#include "usbdata.h"

#define BUFFER_SIZE	100

uint8_t *packets[BUFFER_SIZE];
uint8_t packet_length[BUFFER_SIZE];
uint8_t packet_index_in = 0; /* points to next free space in buffer */
uint8_t packet_index_out = 0; /* points to element sending at the moment in buffer or next empty element if nothing ist to send */

uint8_t during_send = FALSE;
uint8_t index_during_send = 0;

/*
 * internal functions 
 */

/*
 * public functions
 */

void usb_send_next_data()
{
	if (during_send == FALSE)	{
		if (packet_index_in != packet_index_out)	{
			/* there is a packet to transmit */
			during_send = TRUE;
			index_during_send = 0;
		}
	}

	if (during_send == TRUE)	{
		if (packet_length[packet_index_out] - index_during_send > 8)	{
			usbSetInterrupt(packets[packet_index_out] + index_during_send, 8);
			index_during_send += 8;
		} else {
			/* if it are the last 8 or less bytes */
			usbSetInterrupt(packets[packet_index_out] + index_during_send, packet_length[packet_index_out] - index_during_send);
			index_during_send += packet_length[packet_index_out] - index_during_send;
			during_send = FALSE;
			packet_index_out++;
			if (packet_index_out >= BUFFER_SIZE)	{
				packet_index_out = 0;
			}
		}
	}
}

void usb_send_data(uint8_t *data, uint8_t length)
{
	packets[packet_index_in] = data;
	packet_length[packet_index_in] = length;
	packet_index_in++;
	if (packet_index_in >= BUFFER_SIZE)	{
		packet_index_in = 0;
	}
}

//uint8_t usb_is_send_ready()
//{
//	if (packet_index_in == packet_index_out)	{
//		return TRUE;
//	}
//	return FALSE;
//}
//
//uint8_t usb_is_buffer_free()
//{
//	uint8_t packet_index_in_temp = packet_index_in + 1;
//	if (packet_index_in_temp > BUFFER_SIZE)	{
//		packet_index_in_temp = 0;
//	}
//	if (packet_index_in_temp != packet_index_out)	{
//		return TRUE;
//	}
//	return FALSE;
//}
