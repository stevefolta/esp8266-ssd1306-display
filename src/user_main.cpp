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
static os_timer_t word_timer;

struct Word {
	const char* word;
	int ms;
	};

enum {
	default_word_ms = 150,
	sentence_word_ms = 1000,
	};

static Word words[] = {
	{ "This", default_word_ms },
	{ "is", default_word_ms },
	{ "an", default_word_ms },
	{ "ESP8266", default_word_ms },
	{ "running", default_word_ms },
	{ "an", default_word_ms },
	{ "OLED", default_word_ms },
	{ "display.", sentence_word_ms },
	};

static void next_word(void* arg)
{
	static int which_word = 0;
	static const int num_words = sizeof(words) / sizeof(words[0]);
	const Word* word = &words[which_word];

	// Show the word.
	display->clear();
	display->draw_string(
		word->word,
		(display->width - display->string_width(word->word)) / 2, 18);
	display->display();

	// Go to the next word.
	which_word += 1;
	if (which_word >= num_words)
		which_word = 0;

	// Rest the timer to show the next word.
	os_timer_setfn(&word_timer, next_word, NULL);
	os_timer_arm(&word_timer, word->ms, false);
}


static void start_display(void* arg)
{
	display->clear();
	const char* word = "Hello!";
	display->draw_string(
		word, (display->width - display->string_width(word)) / 2, 18);
	display->display();
	display->turn_on();

	os_timer_setfn(&word_timer, next_word, NULL);
	os_timer_arm(&word_timer, 1000, false);
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

	// Start the display.
	os_timer_disarm(&word_timer);
#ifdef DELAY_DISPLAY_START
	os_timer_setfn(&word_timer, start_display, NULL);
	os_timer_arm(&word_timer, 1000, false);
#else
	start_display(NULL);
#endif
}


