#include "WordDisplay.h"
#include "SSD1306Display.h"
extern "C" {
#include "ets_sys.h"
}


void ICACHE_FLASH_ATTR WordDisplay::show(const char* word)
{
	oled_display->clear();
	oled_display->draw_string(
		word, (oled_display->width - oled_display->string_width(word)) / 2, 18);
	oled_display->display();
}



