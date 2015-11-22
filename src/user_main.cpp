extern "C" {
#include "ets_sys.h"
#include "user_interface.h"
#include "string.h"
#include "osapi.h"
#include "espconn.h"
#include "mem.h"
#include "driver/uart.h"
#include "sdk_missing.h"
}

#include "SSD1306Display.h"
#include "WordDisplay.h"
#include "WordParser.h"
#include "log.h"

#include "config.h"

static SSD1306Display* oled_display = 0;
static WordDisplay* display = 0;
static os_timer_t timer;

static const char text[] = "This is an ESP8266 running an OLED display.";
#include "wiki.h"

static void between_text(void* arg);

static void ICACHE_FLASH_ATTR show_text(void* arg)
{
	display->show_text(wiki_text, between_text, NULL);
}


static void ICACHE_FLASH_ATTR between_text(void* arg)
{
	display->show("");

	os_timer_setfn(&timer, show_text, NULL);
	os_timer_arm(&timer, 1000, false);
}


static void ICACHE_FLASH_ATTR start_display(void* arg)
{
	const char* word = "Hello!";
	display->show(word);
	oled_display->turn_on();

	os_timer_setfn(&timer, between_text, NULL);
	os_timer_arm(&timer, 1000, false);
}


extern "C"
void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	// Connect to wifi.
	os_printf("Connection to wifi...\n");
	wifi_set_opmode(STATION_MODE);
	struct station_config config;
	strcpy((char*) config.ssid, WIFI_SSID);
	strcpy((char*) config.password, WIFI_PASSWORD);
	config.bssid_set = 0;
	bool ok = wifi_station_set_config(&config);

	oled_display = new SSD1306Display();
	display = new WordDisplay(oled_display);

	// Start the display.
	os_timer_disarm(&timer);
#ifdef DELAY_DISPLAY_START
	os_timer_setfn(&timer, start_display, NULL);
	os_timer_arm(&timer, 1000, false);
#else
	start_display(NULL);
#endif
}


