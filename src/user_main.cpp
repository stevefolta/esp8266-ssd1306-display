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
static WordParser* words = 0;
static os_timer_t word_timer;

struct Word {
	const char* word;
	int ms;
	};

enum {
	default_word_ms = 150,
	sentence_word_ms = 1000,
	text_end_ms = 2000,
	};

static const char text[] = "This is an ESP8266 running an OLED display.";
#include "wiki.h"

static void next_word(void* arg)
{
	if (words == NULL) {
		// Show blank, and set up the text (again).
		words = new WordParser(wiki_text);
		display->show("");
		os_timer_setfn(&word_timer, next_word, NULL);
		os_timer_arm(&word_timer, sentence_word_ms, false);
		return;
		}

	char word[32];
	static bool shown_para_break = false;
	if (words->at_paragraph_end() && !shown_para_break) {
		word[0] = 0;
		shown_para_break = true;
		}
	else {
		words->get_next_word(word, 32);
		shown_para_break = false;
		}

	// Show the word.
	display->show(word);

	// Rest the timer to show the next word.
	int ms = default_word_ms;
	if (words->at_text_end()) {
		ms = text_end_ms;
		delete words;
		words = 0;
		}
	else if (words->at_sentence_end())
		ms = sentence_word_ms;
	os_timer_setfn(&word_timer, next_word, NULL);
	os_timer_arm(&word_timer, ms, false);
}


static void start_display(void* arg)
{
	const char* word = "Hello!";
	display->show(word);
	oled_display->turn_on();

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

	oled_display = new SSD1306Display();
	display = new WordDisplay(oled_display);

	// Start the display.
	os_timer_disarm(&word_timer);
#ifdef DELAY_DISPLAY_START
	os_timer_setfn(&word_timer, start_display, NULL);
	os_timer_arm(&word_timer, 1000, false);
#else
	start_display(NULL);
#endif
}


