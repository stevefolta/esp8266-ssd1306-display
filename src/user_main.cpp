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
#include "WebServer.h"
#include "log.h"

#include "config.h"

static SSD1306Display* oled_display = 0;
static WordDisplay* display = 0;
static WebServer* web_server = 0;
static char* message = 0;
static int message_length = 0;
static os_timer_t timer;

static const char text[] = "This is an ESP8266 running an OLED display.";
#include "wiki.h"

static void between_text(void* arg);
static void between_message(void* arg);

static void ICACHE_FLASH_ATTR schedule(void (*fn)(void*), int ms)
{
	os_timer_setfn(&timer, fn, NULL);
	os_timer_arm(&timer, ms, false);
}

static void ICACHE_FLASH_ATTR show_text(void* arg)
{
	display->show_text(wiki_text, between_text, NULL);
}


static void ICACHE_FLASH_ATTR between_text(void* arg)
{
	display->set_font_size(24);
	display->show("");
	schedule(show_text, 1000);
}


static void ICACHE_FLASH_ATTR check_connection(void* arg)
{
	display->set_font_size(16);

	static int retries = 0;

	uint8 status = wifi_station_get_connect_status();
	if (status == STATION_GOT_IP) {
		char msg[32];
		struct ip_info ip_info;
		bool ok = wifi_get_ip_info(0, &ip_info);
		if (ok) {
			os_sprintf(
				msg, "%d.%d.%d.%d", IP2STR(&ip_info.ip));
			}
		else
			os_sprintf(msg, "Problem.");
		display->show(msg);
		schedule(between_text, 3000);
		web_server = new WebServer();
		}
	else if (status == STATION_CONNECTING) {
		retries = 0;
		display->show("Connecting...");
		schedule(check_connection, 500);
		}
	else if (status == STATION_WRONG_PASSWORD && ++retries < 10) {
		// It seems to report STATION_WRONG_PASSWORD while it's sending the
		// password.  So we give it a few seconds to exit that state before we
		// believe that the password is actually bad.
		schedule(check_connection, 500);
		}
	else {
		const char* msg = "Unknown";
		if (status == STATION_WRONG_PASSWORD)
			msg = "Bad password.";
		else if (status == STATION_NO_AP_FOUND)
			msg = "No access point.";
		else if (status == STATION_CONNECT_FAIL)
			msg = "Failed.";
		display->show(msg);
		schedule(between_text, 3000);
		}
}


static void ICACHE_FLASH_ATTR start_display(void* arg)
{
	const char* word = "Hello!";
	display->show(word);
	oled_display->turn_on();

	schedule(check_connection, 1000);
}


void ICACHE_FLASH_ATTR show_message(void* arg)
{
	display->show_text(message, message_length, between_message, NULL);
}


void ICACHE_FLASH_ATTR between_message(void* arg)
{
	display->set_font_size(24);
	display->show("");
	schedule(show_message, 1000);
}


void ICACHE_FLASH_ATTR display_message(const char* new_message, int length)
{
	os_timer_disarm(&timer);
	display->set_font_size(24);
	display->show("");
	if (message)
		os_free(message);
	if (length < 0)
		length = strlen(new_message);
	message = (char*) os_zalloc(length + 1);
	memcpy(message, new_message, length);
	message[length] = 0;
	message_length = length;
	show_message(NULL);
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
	schedule(start_display, 1000);
#else
	start_display(NULL);
#endif
}


