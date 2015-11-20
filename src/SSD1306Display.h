#ifndef SSD1306Display_h
#define SSD1306Display_h

class I2CMaster;


class SSD1306Display {
	public:
		SSD1306Display();
		~SSD1306Display();

		void	clear();
		void	fill();
		int	draw_string(const char* str, int x, int y);
		int	string_width(const char* str);
		void	set_pixel(int x, int y);
		void	display();

		void	turn_on();
		void	turn_off();

		enum {
			width = 128,
			height = 64,
			buffer_size = (width * height) / 8,
			};

	protected:
		class Font {
			public:
				Font(const char* data_in);

				int	width(const char* str);
				int	render(const char* str, int x, int y, SSD1306Display* display);

			protected:
				enum {
					width_offset = 0,
					height_offset,
					first_char_offset,
					num_chars_offset,
					char_widths_offset,
					};

				const char*	data;
				int	first_char, num_chars, height;
			};

		I2CMaster* i2c;
		char*	buffer;
		Font	font;

		void	initialize();
		void	i2c_start();
		void	send_command(unsigned char command);
	};


#endif	// !SSD1306Display_h

