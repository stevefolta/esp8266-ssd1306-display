SOURCES := user_main.cpp SSD1306Display.cpp I2CMaster.cpp WordParser.cpp
SOURCES += cpp.cpp
DRIVER_SOURCES := uart.c

# SDK header problems revealed by U8glib:
# SWITCHES += __have_long64=0 __int_fast64_t_defined=1

-include Makefile.local

SDK ?= /opt/Espressif/ESP8266_SDK
TOOLCHAIN ?= /opt/Espressif/crosstool-NG
OBJECTS_DIR ?= objects

default: all

VPATH += src

# Project specific:



#####


BOOT ?= none
APP ?= 0
SPI_SPEED ?= 40
SPI_MODE ?= DIO
SPI_SIZE_MAP ?= 0

ifeq "$(BOOT)" "none"
	APP = 0
endif

ifeq "$(SPI_SIZE_MAP)" "0"
	SPI_SIZE_SPEC = 512 512 true 1.2 0x41000
else ifeq "$(SPI_SIZE_MAP)" "2"
	SPI_SIZE_SPEC = 1024	1024 true 1.2 0x81000
else ifeq "$(SPI_SIZE_MAP)" "3"
	SPI_SIZE_SPEC = 2048	1024 true 1.2 0x81000
else ifeq "$(SPI_SIZE_MAP)" "4"
	SPI_SIZE_SPEC = 4096	1024 true 1.2 0x81000
else ifeq "$(SPI_SIZE_MAP)" "5"
	SPI_SIZE_SPEC = 2048	2048 false 1.4 0x101000
else ifeq "$(SPI_SIZE_MAP)" "6"
	SPI_SIZE_SPEC = 4096	2048 false 1.4 0x101000
else
	foo := $(error Unknow SPI_SIZE_MAP: $(SPI_SIZE_MAP).)
endif
FLASH_SIZE = $(shell echo $(SPI_SIZE_SPEC) | cut '-d ' -f 1)
IMAGE_SIZE = $(shell echo $(SPI_SIZE_SPEC) | cut '-d ' -f 2)
LD_INCLUDES_APP = $(shell echo $(SPI_SIZE_SPEC) | cut '-d ' -f 3)
SUPPORTED_BOOTLOADER = $(shell echo $(SPI_SIZE_SPEC) | cut '-d ' -f 4)
BIN_FLASH_ADDR := 0x01000
ifeq "$(APP)" "2"
	BIN_FLASH_ADDR := $(shell echo $(SPI_SIZE_SPEC) | cut '-d ' -f 5)
endif

ifeq "$(SPI_SPEED)" "26.7"
	FREQ_DIV = 1
else ifeq "$(SPI_SPEED)" "20"
	FREQ_DIV = 2
else ifeq "$(SPI_SPEED)" "80"
	FREQ_DIV = 15
else
	FREQ_DIV = 0
endif

ifeq "$(SPI_MODE)" "QOUT"
	SPI_MODE_NUM = 1
else ifeq "$(SPI_MODE)" "DIO"
	SPI_MODE_NUM = 2
else ifeq "$(SPI_MODE)" "DOUT"
	SPI_MODE_NUM = 3
else
	SPI_MODE_NUM = 0
endif

LD_DIR = $(SDK)/ld
ifeq "$(APP)" "0"
	LD_FILE = $(LD_DIR)/eagle.app.v6.ld
else ifeq "$(LD_INCLUDES_APP)" "true"
	LD_FILE = $(LD_DIR)/eagle.app.v6.$(BOOT).$(IMAGE_SIZE).app$(APP).ld
else
	LD_FILE = $(LD_DIR)/eagle.app.v6.$(BOOT).$(IMAGE_SIZE).ld
endif


export PATH := $(TOOLCHAIN)/builds/xtensa-lx106-elf/bin:$(PATH)

AR = xtensa-lx106-elf-ar
CC = xtensa-lx106-elf-gcc
CXX = xtensa-lx106-elf-g++
NM = xtensa-lx106-elf-nm
CPP = xtensa-lx106-elf-cpp
OBJCOPY = xtensa-lx106-elf-objcopy
OBJDUMP = xtensa-lx106-elf-objdump

DRIVER_OBJECTS_DIR = $(OBJECTS_DIR)/driver

OBJECTS = $(foreach object,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SOURCES))),$(OBJECTS_DIR)/$(object))
DRIVER_OBJECTS = $(foreach source,$(DRIVER_SOURCES),$(DRIVER_OBJECTS_DIR)/$(source:.c=.o))

ifndef VERBOSE_MAKE
	QUIET := @
endif

SWITCHES += ICACHE_FLASH

CFLAGS += -Isrc/
CFLAGS += -I$(SDK)/include
CFLAGS += -I$(SDK)/examples/driver_lib/include
CFLAGS += $(foreach dir,$(INCLUDE_DIRS),-I$(dir))

CFLAGS += -MMD
CFLAGS += -Os
CFLAGS += -g
CFLAGS += -Wpointer-arith -Wundef -Werror
CFLAGS += -nostdlib
CFLAGS += -fno-inline-functions -ffunction-sections -fdata-sections
CFLAGS += -mlongcalls -mtext-section-literals
CFLAGS += $(foreach switch,$(SWITCHES),-D$(switch))

CXXFLAGS += -fno-exceptions -fno-rtti

EARLY_LINK_FLAGS = 	\
	-L$(SDK)/lib 	\
	-nostdlib 	\
	-T$(LD_FILE) 	\
	-Wl,--no-check-sections 	\
	-u call_user_start 	\
	-Wl,-static 	\
	-Wl,--start-group 	\
	-lc -lgcc -lhal -lphy -lpp -lnet80211 	\
	-llwip -lwpa -lmain -ljson -lupgrade\
	-lssl -lpwm -lsmartconfig
LATE_LINK_FLAGS = 	\
	-Wl,--end-group


BIN_NAME = user$(APP).$(FLASH_SIZE).$(BOOT).$(SPI_SIZE_MAP)
IMAGE_FILE_NAME := image$(APP).$(BOOT).$(SPI_SIZE_MAP).out
IMAGE_FILE := $(OBJECTS_DIR)/$(IMAGE_FILE_NAME)
ifeq "$(APP)" "0"
	FLASH_BIN := eagle.flash.bin
else
	FLASH_BIN := $(BIN_NAME).bin
endif

all: $(EARLY_ALL_TARGETS) $(FLASH_BIN)

.PHONY: ota
ota: $(EARLY_ALL_TARGETS)
	$(QUIET) make APP=1
	$(QUIET) make APP=2

-include $(OBJECTS_DIR)/*.d

$(OBJECTS_DIR)/libuser.a: $(OBJECTS)
	@echo Building $@...
	$(QUIET) $(AR) ru $@ $^

$(OBJECTS_DIR)/libdriver.a: $(DRIVER_OBJECTS)
	@echo Building $@...
	$(QUIET) $(AR) ru $@ $^

$(OBJECTS_DIR)/%.o: %.cpp
	@echo Compiling $(notdir $<)...
	@mkdir -p $(OBJECTS_DIR)
	$(QUIET) $(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ -c $<

$(OBJECTS_DIR)/%.o: %.c
	@echo Compiling $(notdir $<)...
	@mkdir -p $(OBJECTS_DIR)
	$(QUIET) $(CC) $(CFLAGS) -o $@ -c $<

$(DRIVER_OBJECTS_DIR)/%.o: $(SDK)/examples/driver_lib/driver/%.c
	@echo Compiling $(notdir $<)...
	@mkdir -p $(DRIVER_OBJECTS_DIR)
	$(QUIET) $(CC) $(CFLAGS) -o $@ -c $<

$(IMAGE_FILE): $(OBJECTS_DIR)/libuser.a $(OBJECTS_DIR)/libdriver.a
	@echo Linking $@...
	$(QUIET) $(CXX) $(EARLY_LINK_FLAGS) $^ $(LATE_LINK_FLAGS) -o $@

$(FLASH_BIN): $(IMAGE_FILE)
	@echo Building $@...

	@# Debugging info.
ifdef DUMP_INFO
ifeq "$(APP)" "0"
	@rm -f eagle.S eagle.dump
	@$(OBJDUMP) -x -s $< > eagle.dump
	@$(OBJDUMP) -S $< > eagle.S
else
	@rm -f $(BIN_NAME).S $(BIN_NAME).dump
	@$(OBJDUMP) -x -s $< > $(BIN_NAME).dump
	@$(OBJDUMP) -S $< > $(BIN_NAME).S
endif
endif

	@# Prepare for the Python script.
	@$(OBJCOPY) --only-section .text -O binary $< $(OBJECTS_DIR)/eagle.app.v6.text.bin
	@$(OBJCOPY) --only-section .data -O binary $< $(OBJECTS_DIR)/eagle.app.v6.data.bin
	@$(OBJCOPY) --only-section .rodata -O binary $< $(OBJECTS_DIR)/eagle.app.v6.rodata.bin
	@$(OBJCOPY) --only-section .irom0.text -O binary $< $(OBJECTS_DIR)/eagle.app.v6.irom0text.bin

	@# Call gen_appbin.py and deal with its output.
ifeq "$(APP)" "0"
	$(QUIET) cd $(OBJECTS_DIR) && COMPILE=gcc python $(SDK)/tools/gen_appbin.py $(IMAGE_FILE_NAME) 0 $(SPI_MODE_NUM) $(FREQ_DIV) $(SPI_SIZE_MAP)
	@mv $(OBJECTS_DIR)/eagle.app.flash.bin eagle.flash.bin
	@mv $(OBJECTS_DIR)/eagle.app.v6.irom0text.bin eagle.irom0text.bin
	@rm $(OBJECTS_DIR)/eagle.app.v6.*
	@echo "No separate bootloader needed."
	@echo "Flash eagle.flash.bin to 0x00000."
	@echo "Flash eagle.irom0text.bin to 0x40000."
else
ifneq "$(BOOT)" "new"
		$(QUIET) cd $(OBJECTS_DIR) && COMPILE=gcc python $(SDK)/tools/gen_appbin.py $(IMAGE_FILE_NAME) 1 $(SPI_MODE_NUM) $(FREQ_DIV) $(SPI_SIZE_MAP)
		@echo "Supported by boot_v1.1 and later."
else
		$(QUIET) cd $(OBJECTS_DIR) && COMPILE=gcc python $(SDK)/tools/gen_appbin.py $(IMAGE_FILE_NAME) 2 $(SPI_MODE_NUM) $(FREQ_DIV) $(SPI_SIZE_MAP)
		@echo "Supported by boot_v$(SUPPORTED_BOOTLOADER) and later."
endif
	@mv $(OBJECTS_DIR)/eagle.app.flash.bin $(BIN_NAME).bin
	@rm $(OBJECTS_DIR)/eagle.app.v6.*
	@echo "Flash boot.bin to 0x00000."
	@echo "Flash $(BIN_NAME).bin to $(BIN_FLASH_ADDR)."
endif


.PHONY: clean
clean:
	rm -rf objects *.bin *.S *.dump

.PHONY: flash
flash: $(FLASH_BIN)
	flash-esp 0x00000 eagle.flash.bin 0x40000 eagle.irom0text.bin


.PHONY: test
test:
	@echo FLASH_SIZE: $(FLASH_SIZE), IMAGE_SIZE: $(IMAGE_SIZE)

.PHONY: show-stuff
show-stuff:
	@echo VPATH: $(VPATH)
	@echo OBJECTS: $(OBJECTS)
	

