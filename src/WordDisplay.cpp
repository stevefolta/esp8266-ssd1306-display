#include "WordDisplay.h"
#include "SSD1306Display.h"
#include "WordParser.h"
#include "log.h"
extern "C" {
#include "osapi.h"
#include "sdk_missing.h"
}


enum {
	word_ms = 150,
	sentence_ms = 1000,
	text_end_ms = 2000,
	};

ICACHE_FLASH_ATTR WordDisplay::WordDisplay(SSD1306Display* oled_display_in)
	: oled_display(oled_display_in), words(0)
{
	os_timer_disarm(&text_timer);
}


ICACHE_FLASH_ATTR WordDisplay::~WordDisplay()
{
	os_timer_disarm(&text_timer);
	delete words;
}


void ICACHE_FLASH_ATTR WordDisplay::show(const char* word)
{
	oled_display->clear();
	oled_display->draw_string(
		word, (oled_display->width - oled_display->string_width(word)) / 2, 18);
	oled_display->display();
}


void ICACHE_FLASH_ATTR WordDisplay::show_text(
	const char* text, int text_length, void (*done_fn)(void*), void* done_arg)
{
	os_timer_disarm(&text_timer);
	text_done_fn = done_fn;
	text_done_arg = done_arg;
	delete words;
	words = new WordParser(text, text_length);
	shown_para_break = false;
	next_word();
}


void ICACHE_FLASH_ATTR WordDisplay::next_word()
{
	if (!words) {
		// We've reached the end of the text.
		text_done_fn(text_done_arg);
		return;
		}

	char word[32];
	if (words->at_paragraph_end() && !shown_para_break) {
		word[0] = 0;
		shown_para_break = true;
		}
	else {
		words->get_next_word(word, 32);
		shown_para_break = false;
		}

	// Show the word.
	show(word);

	// Reset the timer to show the next word.
	int ms = word_ms;
	if (words->at_text_end()) {
		ms = text_end_ms;
		delete words;
		words = 0;
		}
	else if (words->at_sentence_end())
		ms = sentence_ms;
	os_timer_setfn(&text_timer, next_word_fn, this);
	os_timer_arm(&text_timer, ms, false);
}


void ICACHE_FLASH_ATTR WordDisplay::next_word_fn(void* arg)
{
	((WordDisplay*) arg)->next_word();
}



