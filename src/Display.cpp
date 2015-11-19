#include "Display.h"
extern "C" {
#include "ets_sys.h"
#include "osapi.h"
#include "sdk_missing.h"
#include "mem.h"
#include "driver/i2c_master.h"
}
#include "log.h"

enum {
	i2c_address = 0x3c,
	};


ICACHE_FLASH_ATTR Display::Display()
{
	log("- Creating Display.\n");
	buffer = (char*) os_zalloc(buffer_size);

	i2c_master_gpio_init();
	initialize();
}


ICACHE_FLASH_ATTR Display::~Display()
{
	os_free(buffer);
}


void ICACHE_FLASH_ATTR Display::clear()
{
	os_memset(buffer, 0, buffer_size);
}


void ICACHE_FLASH_ATTR Display::fill()
{
	os_memset(buffer, 0xFF, buffer_size);
}


void ICACHE_FLASH_ATTR Display::display()
{
	const char* p = buffer;
	const char* stopper = buffer + buffer_size;
	while (p < stopper) {
		i2c_start();
		i2c_master_writeByte(0x40);
		if (!i2c_master_checkAck())
			log("No ack from display for data start!\n");
		for (int x = 0; x < 16; ++x) {
			i2c_master_writeByte(*p++);
			if (!i2c_master_checkAck())
				log("No ack from display for data byte!\n");
			}
		i2c_master_stop();
		}
}


void ICACHE_FLASH_ATTR Display::turn_on()
{
	send_command(0xAF);
}


void ICACHE_FLASH_ATTR Display::turn_off()
{
	send_command(0xAE);
}


void ICACHE_FLASH_ATTR Display::initialize()
{
	log("- Display::initialize().\n");
	static const char init_sequence[] = {
		// This init sequence comes from
		// <https://github.com/squix78/esp8266-oled-ssd1306/blob/master/ssd1306_i2c.cpp>,
		// which in turn got it from
		// <https://github.com/adafruit/Adafruit_SSD1306/blob/master/Adafruit_SSD1306.cpp>.
		// Init sequence for 128x64 OLED module
		0xae,		        // display off
		0xa6,            // Set Normal Display (default)
		0xAE,            // DISPLAYOFF
		0xD5,            // SETDISPLAYCLOCKDIV
		0x80,            // the suggested ratio 0x80
		0xA8,            // SSD1306_SETMULTIPLEX
		0x3F,
		0xD3,            // SETDISPLAYOFFSET
		0x0,             // no offset
		0x40 | 0x0,      // SETSTARTLINE
		0x8D,            // CHARGEPUMP
		0x14,
		0x20,             // MEMORYMODE
		0x00,             // 0x0 act like ks0108

		//0xA0 | 0x1,      // SEGREMAP   // Rotate screen 180 deg
		0xA0,

		//0xC8,            // COMSCANDEC  Rotate screen 180 Deg
		0xC0,

		0xDA,            // 0xDA
		0x12,           // COMSCANDEC
		0x81,           // SETCONTRAS
		0xCF,           //
		0xd9,          // SETPRECHARGE 
		0xF1, 
		0xDB,        // SETVCOMDETECT                
		0x40,
		0xA4,        // DISPLAYALLON_RESUME        
		0xA6,        // NORMALDISPLAY             

		0x2e,            // stop scroll
		//----------------------------REVERSE comments----------------------------//
		//  0xa0,		//seg re-map 0->127(default)
		//  0xa1,		//seg re-map 127->0
		//  0xc8,
		//  delay(1000,
		//----------------------------REVERSE comments----------------------------//
		// 0xa7,  // Set Inverse Display  
		// 0xae,		// display off
		0x20,            // Set Memory Addressing Mode
		0x00,            // Set Memory Addressing Mode ab Horizontal addressing mode
		//0x02,         // Set Memory Addressing Mode ab Page addressing mode(RESET)  
		};

	const char* p = init_sequence;
	const char* stopper = p + sizeof(init_sequence);
	for (; p < stopper; ++p)
		send_command(*p);
}


void ICACHE_FLASH_ATTR Display::i2c_start()
{
	i2c_master_start();
	i2c_master_writeByte(i2c_address << 1);
	if (!i2c_master_checkAck()) {
		log("No ack from display!\n");
		}
}


void ICACHE_FLASH_ATTR Display::send_command(unsigned char command)
{
	log("- sending command 0x%02X.\n", command);
	i2c_start();
	i2c_master_writeByte(0x80);
	if (!i2c_master_checkAck())
		log("No ack for command start!\n");
	i2c_master_writeByte(command);
	if (!i2c_master_checkAck())
		log("No ack for command 0x%02X!\n", command);
	i2c_master_stop();
}



