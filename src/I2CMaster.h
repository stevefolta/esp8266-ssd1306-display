#ifndef I2CMaster_h
#define I2CMaster_h

class I2CMaster {
	public:
		I2CMaster(int sda_pin, int scl_pin);

		bool	start_transmission(unsigned char i2c_addr);
		void	end_transmission();
		bool	send_byte(unsigned char byte);

	protected:
		int	sda_pin, scl_pin;
		bool	last_sda;

		void	setDC(bool sda, bool scl);
		bool	getSDA();
		void	wait() {
			// TOP SPEED!  No waiting.
			}
	};



#endif	// !I2CMaster_h

