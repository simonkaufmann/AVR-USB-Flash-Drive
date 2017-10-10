#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <avr/pgmspace.h>
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "requests.h"       /* The custom request numbers we use */
#include "utils.h"
#include "main.h"
#include "commands.h"
#include "usbdata.h"
#include "twi.h"
#include "ext_eeprom.h"
#include "ram.h"

#define GET_MAX_LUN	0xfe

const uint8_t cbw_signature[4] = {0x55, 0x53, 0x42, 0x43};
const uint8_t csw_signature[4] = {0x55, 0x53, 0x42, 0x53};
static uint8_t cbw_progress = FALSE;
static uint8_t cbw_finished = FALSE;

uint8_t int_data[8] = {0,1,2,3,4,5,6,7};
uint8_t int_len = 8;

volatile uint8_t write_eeprom = FALSE;

uint8_t is_writing = FALSE;
uint8_t is_writing_beginning = FALSE;
uint16_t write = 0;
static uint16_t count = 0;
uint8_t writing_finished = FALSE;
uint8_t write_page = 0;
uint8_t transfer_length_write;
uint8_t transfer_length_write_beginning;

#define CBW_SIZE 31
uint8_t cbw[CBW_SIZE];
uint8_t index = 0;
uint8_t index_write = 0;

uint8_t blocks_written[16];

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

/* good page to describe descriptors
 * http://www.beyondlogic.org/usbnutshell/usb5.shtml
 */

/* to use this descriptor:
 *  the define USB_CFG_DESCR_PROPS_CONFIGURATION in usbconfig.h must be set
 *  to USB_PROP_IS_DYNAMIC and the function usbFunctionDescriptor has to be
 *  defined
 */
static const PROGMEM char configDescrCDC[] = {   /* USB configuration descriptor */
    9,          /* sizeof(usbDescrConfig): length of descriptor in bytes */
    USBDESCR_CONFIG,    /* descriptor type */
    32,
    0,          /* total length of data returned (including inlined descriptors) */
    1,          /* number of interfaces in this configuration */
    1,          /* index of this configuration */
    0,          /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    (1 << 7) | USBATTR_SELFPOWER,       /* attributes */
#else
    (1 << 7),                           /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER/2,    /* max USB current in 2mA units */

    /* interface descriptor follows inline: */
    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,          /* index of this interface */
    0,          /* alternate setting for this interface */
    2,   /* endpoints excl 0: number of endpoint descriptors to follow */
    8,//8, //mass storage     //USB_CFG_INTERFACE_CLASS,
    6, //SCSI command set //USB_CFG_INTERFACE_SUBCLASS,
    80, //Bulk Only        //USB_CFG_INTERFACE_PROTOCOL,
    0,          /* string index for interface */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x80|USB_CFG_EP3_NUMBER,        /* IN endpoint number 1 */
    0x02,        /* attrib: Bulk endpoint */
	///what to do with maximum packet size????? 
    8, 0,        /* maximum packet size */
    0,  //USB_CFG_INTR_POLL_INTERVAL,        /* in ms */

	/* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x02,        /* OUT endpoint number 2 */
    0x02,        /* attrib: Bulk endpoint */
	///what to do with maximum packet size????? 
    8, 0,        /* maximum packet size */
    0,  //USB_CFG_INTR_POLL_INTERVAL,        /* in ms */

};

uchar usbFunctionDescriptor(usbRequest_t *rq)
{

    if(rq->wValue.bytes[1] == USBDESCR_DEVICE){
        usbMsgPtr = (uchar *)usbDescriptorDevice;
        return usbDescriptorDevice[0];
    }else{  /* must be config descriptor */
        usbMsgPtr = (uchar *)configDescrCDC;
        return sizeof(configDescrCDC);
    }
}


usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;
static uchar    dataBuffer[4];  /* buffer must stay valid when usbFunctionSetup returns */

	/* is this needed? */
    if(rq->bRequest == CUSTOM_RQ_ECHO){ /* echo -- used for reliability tests */
        dataBuffer[0] = rq->wValue.bytes[0];
        dataBuffer[1] = rq->wValue.bytes[1];
        dataBuffer[2] = rq->wIndex.bytes[0];
        dataBuffer[3] = rq->wIndex.bytes[1];
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 4;
    }else if(rq->bRequest == CUSTOM_RQ_SET_STATUS){
        if(rq->wValue.bytes[0] & 1){    /* set LED */
         //   LED_PORT_OUTPUT |= _BV(LED_BIT);
        }else{                          /* clear LED */
          //  LED_PORT_OUTPUT &= ~_BV(LED_BIT);
        }
    }else if(rq->bRequest == CUSTOM_RQ_GET_STATUS){
        dataBuffer[0] = 0x00;//((LED_PORT_OUTPUT & _BV(LED_BIT)) != 0);
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 1;                       /* tell the driver to send 1 byte */
	/* end is this needed? */
    } else if(rq->bRequest == GET_MAX_LUN)	{
		dataBuffer[0] = 0; /* 0 different LUN devices */
		usbMsgPtr = dataBuffer;
		return 1; /* one byte to send! */
	}
    return 0;   /* default for not implemented requests: return no data back to host */
}

/* ------------------------------------------------------------------------- */

int __attribute__((noreturn)) main(void)
{


uchar   i;

	ram_init();

	LED_DDR |= (1 << LED);

	twi_init();

	/* read saved data from eeprom */
	LED_ON();
	uint16_t j = 0;
	for (j = 0; j < 2048; j++)	{
		uint8_t data[32];
		ext_eeprom_read_block(data, j * 32, 32);
		for (i = 0; i < 32; i++)	{
			ram_write_byte(j * 32 + i, data[i]);
		}
	}
	LED_OFF();

    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    odDebugInit();
    DBG1(0x00, 0, 0);       /* debug output: main starts */
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    DBG1(0x01, 0, 0);       /* debug output: main loop starts */
    for(;;){                /* main event loop */
        DBG1(0x02, 0, 0);   /* debug output: main loop iterates */
        wdt_reset();
        usbPoll();
		if (usbInterruptIsReady())	{
			usb_send_next_data();
			execute_read();
		}
		if (cbw_finished == TRUE)	{
			cbw_finished = FALSE;
			execute_cbw(cbw);
		}
		uint8_t t;
		static int8_t write_block = -1;
		if (write_block < 0)	{
			for (t = 0; t < 16; t++)	{
				if (blocks_written[t] > 0)	{
					uint8_t j, suc = FALSE;
					for (j = 0; j < 8; j++)	{
						if ((blocks_written[t] & (1 << j)) > 0)	{
							write_block = t * 8 +  j;
							blocks_written[t] &= ~(1 << j);
							suc = TRUE;
							LED_ON();
							break;
						}
					}
					if (suc == TRUE)	{
						break;
					}
				}
			}
			if (write_block < 0)	{
				LED_OFF();
			}
		} else	{
			uint8_t i;
			static uint8_t count = 0;
			for (i = 0; i < PAGE_SIZE; i++)	{
				data_rw[i] = ram_read_byte(BLOCK_SIZE * write_block + count * PAGE_SIZE + i);
			}
			ext_eeprom_write_block(data_rw, BLOCK_SIZE * write_block + count * PAGE_SIZE, PAGE_SIZE);
			count++;
			if (count >= 4)	{
				count = 0;
				write_block = -1;
			}
		}
	}
}

/* ------------------------------------------------------------------------- */

void set_writing(uint16_t write_int, uint16_t transfer_length)
{
	is_writing = TRUE;
	write = write_int;
	count = 0;
	transfer_length_write = transfer_length;
	transfer_length_write_beginning = transfer_length;
}

void set_writing_beginning(void)
{
	is_writing_beginning = TRUE;
	write_page = 0;
}

void usbFunctionWriteOut(uchar *data, uchar len)
{
	if (is_writing == TRUE)	{
		if (count < write)	{
			uint8_t i;
			for (i = 0; i < 8; i++)	{
				ram_write_byte(logical_block * BLOCK_SIZE + count * 8 + i + write_page * BLOCK_SIZE, data[i]);
			}
			count++;
		}
		if (count == write)	{
			uint8_t block = logical_block + write_page;
			blocks_written[block / 8] |= (1 << (block % 8));
			write_page++;
			count = 0;
			if (write_page == transfer_length_write)	{
				writing_finished = TRUE;
				is_writing = FALSE;
				is_writing_beginning = FALSE;
				write_eeprom = TRUE;
				execute_writing_finished();
			}
		}
	} else {
		if (cbw_progress == FALSE && cbw_finished == FALSE)	{
			if (data_cmp(data, cbw_signature, 4) == TRUE)	{
				index_write = len;
				data_cpy(cbw, data, len);
				cbw_progress = TRUE;
			}
		} else {
			uint8_t temp = index_write + len;
			if (temp >= CBW_SIZE)	{
				temp = CBW_SIZE;
				cbw_progress = FALSE;
				cbw_finished = TRUE;
			}
			data_cpy(cbw + index_write, data, temp - index_write);
			index_write = temp;
		}
	}
}

