#ifndef WordDisplay_h
#define WordDisplay_h

extern "C" {
#include "os_type.h"
}

class SSD1306Display;
class WordParser;

class WordDisplay {
	public:
		WordDisplay(SSD1306Display* oled_display_in);
		~WordDisplay();

		void	show(const char* word);
		void	show_text(const char* text, int text_length, void (*done_fn)(void*), void* done_arg);
		void	show_text(const char* text, void (*done_fn)(void*), void* done_arg)
		{
			show_text(text, -1, done_fn, done_arg);
		}

	protected:
		SSD1306Display*	oled_display;
		WordParser*	words;
		bool shown_para_break;
		void	(*text_done_fn)(void* arg);
		void*	text_done_arg;

		void	next_word();
		static void	next_word_fn(void* arg);
		os_timer_t text_timer;
	};


#endif	// !WordDisplay_h

