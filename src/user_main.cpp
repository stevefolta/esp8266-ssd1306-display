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
	if (num_ticks++ == 10) {
		display = new Display();
		display->fill();
		display->display();
		display->turn_on();
		}
	else if (num_ticks > 10) {
		static bool phase = false;
		if (phase)
			display->fill();
		else
			display->clear();
		display->display();
		phase = !phase;
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

	// Run the tick.
	os_timer_disarm(&tick_timer);
	os_timer_setfn(&tick_timer, tick, NULL);
	os_timer_arm(&tick_timer, 1000, true);
}


