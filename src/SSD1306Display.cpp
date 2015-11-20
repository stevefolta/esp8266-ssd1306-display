#include "SSD1306Display.h"
#include "I2CMaster.h"
#include "FlashData.h"
extern "C" {
#include "ets_sys.h"
#include "osapi.h"
#include "sdk_missing.h"
#include "mem.h"
}
#include "log.h"
#include "config.h"

#define PROGMEM ICACHE_RODATA_ATTR
#include "third-party/SSD1306Fonts.h"

enum {
	i2c_address = 0x3c,
	};

#ifndef DISPLAY_SDA_PIN
	#define DISPLAY_SDA_PIN	2
#endif
#ifndef DISPLAY_SCL_PIN
	#define DISPLAY_SCL_PIN	14
#endif


ICACHE_FLASH_ATTR SSD1306Display::SSD1306Display()
	: font(ArialMT_Plain_24)
{
	log("- Creating Display.\n");
	buffer = (char*) os_zalloc(buffer_size);

	i2c = new I2CMaster(DISPLAY_SDA_PIN, DISPLAY_SCL_PIN);
	initialize();
}


ICACHE_FLASH_ATTR SSD1306Display::~SSD1306Display()
{
	os_free(buffer);
	delete(i2c);
}


void ICACHE_FLASH_ATTR SSD1306Display::clear()
{
	os_memset(buffer, 0, buffer_size);
}


void ICACHE_FLASH_ATTR SSD1306Display::fill()
{
	os_memset(buffer, 0xFF, buffer_size);
}


int ICACHE_FLASH_ATTR SSD1306Display::draw_string(const char* str, int x, int y)
{
	return font.render(str, x, y, this);
}


int ICACHE_FLASH_ATTR SSD1306Display::string_width(const char* str)
{
	return font.width(str);
}


void ICACHE_FLASH_ATTR SSD1306Display::set_pixel(int x, int y)
{
	if (x < 0 || x >= width || y < 0 || y >= height)
		return;

	// Apparently, the buffer is laid out sideways.
	buffer[(y / 8) * width + x] |= 1 << (y & 0x07);
}


void ICACHE_FLASH_ATTR SSD1306Display::display()
{
	const char* p = buffer;
	const char* stopper = buffer + buffer_size;
	while (p < stopper) {
		i2c_start();
		if (!i2c->send_byte(0x40))
			log("No ack from display for data start!\n");
		for (int x = 0; x < 16; ++x) {
			if (!i2c->send_byte(*p++))
				log("No ack from display for data byte!\n");
			}
		i2c->end_transmission();
		}
}


void ICACHE_FLASH_ATTR SSD1306Display::turn_on()
{
	send_command(0xAF);
}


void ICACHE_FLASH_ATTR SSD1306Display::turn_off()
{
	send_command(0xAE);
}


void ICACHE_FLASH_ATTR SSD1306Display::initialize()
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

		0xA0 | 0x1,      // SEGREMAP   // Rotate screen 180 deg
		// 0xA0,

		0xC8,            // COMSCANDEC  Rotate screen 180 Deg
		// 0xC0,

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
		0xa7,  // Set Inverse Display  
		// 0xae,		// display off
		0x20,            // Set Memory Addressing Mode
		0x00,            // Set Memory Addressing Mode ab Horizontal addressing mode
		//0x02,         // Set Memory Addressing Mode ab Page addressing mode(RESET)  

		0x00, 0x10, 0xB0, 	// Clear the start address, in case we reset in the middle of sending data.
		};

	const char* p = init_sequence;
	const char* stopper = p + sizeof(init_sequence);
	for (; p < stopper; ++p)
		send_command(*p);
}


void ICACHE_FLASH_ATTR SSD1306Display::i2c_start()
{
	if (!i2c->start_transmission(i2c_address))
		log("No ack from display!\n");
}


void ICACHE_FLASH_ATTR SSD1306Display::send_command(unsigned char command)
{
	log("- sending command 0x%02X.\n", command);
	i2c_start();
	if (!i2c->send_byte(0x80))
		log("No ack for command start!\n");
	if (!i2c->send_byte(command))
		log("No ack for command 0x%02X!\n", command);
	i2c->end_transmission();
}



SSD1306Display::Font::Font(const char* data_in)
	: data(data_in)
{
	first_char = pgm_read_byte(data + first_char_offset);
	num_chars = pgm_read_byte(data + num_chars_offset);
	height = pgm_read_byte(data + height_offset);
}

int SSD1306Display::Font::width(const char* str)
{
	int width = 0;
	for (; *str; ++str) {
		int char_index = *str - first_char;
		if (char_index < num_chars)
			width += pgm_read_byte(data + char_widths_offset + char_index);
		}
	return width;
}


int SSD1306Display::Font::render(const char* str, int x, int y, SSD1306Display* display)
{
	int total_width = 0;
	for (; *str; ++str) {
		int char_index = *str - first_char;
		if (char_index >= num_chars)
			continue;

		// Find the start of the character's data.
		const char* char_data = data + char_widths_offset + num_chars;
		const char* width_ptr = data + char_widths_offset;
		const char* stopper = width_ptr + char_index;
		for (; width_ptr < stopper; ++width_ptr)
			char_data += pgm_read_byte(width_ptr) * height / 8 + 1;

		// Render the character.
		int char_width = pgm_read_byte(data + char_widths_offset + char_index);
		int row_y = y;
		int row_y_stopper = y + height;
		const char* bitmap = char_data;
		char cur_byte = 0;
		char mask = 0;
		for (; row_y < row_y_stopper; ++row_y) {
			for (int col = 0; col < char_width; ++col) {
				if (mask == 0) {
					mask = 0x01;
					cur_byte = pgm_read_byte(bitmap);
					bitmap += 1;
					}
				if (cur_byte & mask)
					display->set_pixel(x + total_width + col, row_y);
				mask <<= 1;
				}
			}
		total_width += char_width;
		}
	return total_width;
}



