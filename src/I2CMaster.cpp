#include "I2CMaster.h"
#include "FlashData.h"
extern "C" {
#include "ets_sys.h"
#include "gpio.h"
#include "sdk_missing.h"
}


static const char ICACHE_RODATA_ATTR pin_function_reg_offsets[] = {
	0x34, 0x18, 0x38, 0x14, 0x3C, 0x40, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x04, 0x08, 0x0C, 0x10
	};
inline void make_pin_gpio(int pin) {
	int function = 0;
	if (pin == 16)
		function = 1;
	else if ((0xFFCA >> pin) & 0x01)
		function = 3;
	PIN_FUNC_SELECT(
		PERIPHS_IO_MUX + pgm_read_byte(pin_function_reg_offsets + pin),
		function);
	}

I2CMaster::I2CMaster(int sda_pin_in, int scl_pin_in)
	: sda_pin(sda_pin_in), scl_pin(scl_pin_in)
{
	// Set up the pins.
	ETS_GPIO_INTR_DISABLE();
	gpio_pin_intr_state_set(GPIO_ID_PIN(sda_pin), GPIO_PIN_INTR_DISABLE);
	gpio_pin_intr_state_set(GPIO_ID_PIN(scl_pin), GPIO_PIN_INTR_DISABLE);
	// Set pin functions to GPIO.
	make_pin_gpio(sda_pin);
	make_pin_gpio(scl_pin);
	// Set to "open drain".
	GPIO_REG_WRITE(
		GPIO_PIN_ADDR(GPIO_ID_PIN(sda_pin)),
		GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(sda_pin))) |
		GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE));
	GPIO_REG_WRITE(
		GPIO_ENABLE_ADDRESS,
		GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << sda_pin));
	GPIO_REG_WRITE(
		GPIO_PIN_ADDR(GPIO_ID_PIN(scl_pin)),
		GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(scl_pin))) |
		GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE));
	GPIO_REG_WRITE(
		GPIO_ENABLE_ADDRESS,
		GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << scl_pin));
	ETS_GPIO_INTR_ENABLE();
}


bool I2CMaster::start_transmission(unsigned char i2c_addr)
{
	// Start bit.
	setDC(true, true);
	wait();
	setDC(false, true);
	wait();

	// Send the address.
	return send_byte(i2c_addr << 1);
}


void I2CMaster::end_transmission()
{
	setDC(false, false);
	wait();
	setDC(false, true);
	// TODO: Handle clock stretching.
	wait();
	setDC(true, true);
	wait();
}


bool I2CMaster::send_byte(unsigned char byte)
{
	setDC(last_sda, false);
	wait();

	for (int which_bit = 7; which_bit >= 0; --which_bit) {
		bool bit = (byte >> which_bit) & 0x01;
		setDC(bit, false);
		wait();
		setDC(bit, true);
		wait();
		setDC(bit, false);
		wait();
		}

	// Read the ACK bit.
	setDC(last_sda, false);
	wait();
	setDC(true, false);
	wait();
	setDC(true, true);
	wait();
	bool nack = getSDA();
	wait();
	setDC(true, false);
	return !nack;
}


void I2CMaster::setDC(bool sda, bool scl)
{
	uint32 sda_mask = 1 << sda_pin;
	uint32 scl_mask = 1 << scl_pin;
	uint32 set_mask = 0;
	uint32 clear_mask = 0;
	if (sda)
		set_mask |= sda_mask;
	else
		clear_mask |= sda_mask;
	if (scl)
		set_mask |= scl_mask;
	else
		clear_mask |= scl_mask;
	gpio_output_set(set_mask, clear_mask, sda_mask | scl_mask, 0);

	last_sda = sda;
}


bool I2CMaster::getSDA()
{
	return GPIO_INPUT_GET(GPIO_ID_PIN(sda_pin));
}



