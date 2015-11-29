# esp8266-ssd1306-display

This is a little program that lets an ESP8266 fire words at the viewer via an
OLED display with an SSD1306 driver chip.  The default text is from Wikipedia's
ESP8266 entry, but it also presents a web page that lets you change the text.
The web page is at the IP address assigned by the Wifi network, which is
displayed at startup.


## Building

The only dependencies are the ESP8266 SDK and "crosstool-NG"
(<https://github.com/jcmvbkbc/crosstool-NG>) built to support C++ (from the
`lx106-g++` branch).

Create a `config.h` file in the `src` directory and #define `WIFI_SSID` and
`WIFI_PASSWORD` to be strings for your Wifi network's SSID and password.  You
can also set `DISPLAY_SDA_PIN` and `DISPLAY_SCL_PIN` to define the pins for the
I2C interface to the OLED display (they default to pins 2 and 14 respectively).

Example config.h:

```cpp
#define WIFI_SSID "my-wifi-network"
#define WIFI_PASSWORD "its-password"

#define DISPLAY_SDA_PIN 5
```

You can also create a `Makefile.local`.  `SPI_SIZE_MAP` is a good thing to set
there; eg. use this for an ESP-12 module:

```make
SPI_SIZE_MAP := 6
```

Also set `SDK` and `TOOLCHAIN` there if they are not at `/opt/Espressif/ESP8266_SDK`
and `/opt/Espressif/crosstool-NG` respectively.

Then a simple `make` should build it.  If you have a `flash-esp` script in your
$PATH, you can `make flash`.


## License

The code is licensed under the MIT License.  See the `License` file.


## Related projects

This was partially inspired by
<https://github.com/squix78/esp8266-oled-ssd1306>, but that project requires
the Arduino framework.



