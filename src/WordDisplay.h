#ifndef WordDisplay_h
#define WordDisplay_h

class SSD1306Display;

class WordDisplay {
	public:
		WordDisplay(SSD1306Display* oled_display_in)
			: oled_display(oled_display_in) {}

		void	show(const char* word);

	protected:
		SSD1306Display*	oled_display;
	};


#endif	// !WordDisplay_h

