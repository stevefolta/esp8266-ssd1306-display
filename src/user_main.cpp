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

#include "Display.h"
#include "log.h"

#include "config.h"

static Display* display;
static os_timer_t tick_timer;


static void tick(void* arg)
{
	log("tick()...\n");

	static int num_ticks = 0;
	static const int display_start_tick = 2;
	if (num_ticks++ == display_start_tick) {
		display->clear();
		const char* word = "Hello!";
		display->draw_string(
			word, (display->width - display->string_width(word)) / 2, 18);
		display->display();
		display->turn_on();
		}
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

	display = new Display();

	// Run the tick.
	os_timer_disarm(&tick_timer);
	os_timer_setfn(&tick_timer, tick, NULL);
	os_timer_arm(&tick_timer, 1000, true);
}


