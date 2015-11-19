#ifndef Display_h
#define Display_h


class Display {
	public:
		Display();
		~Display();

		void	clear();
		void	fill();
		void	display();

		enum {
			width = 128,
			height = 64,
			buffer_size = (width * height) / 8,
			};

	protected:
		char*	buffer;

		void	initialize();
		void	i2c_start();
		void	send_command(unsigned char command);
	};


#endif	// !Display_h

