# name of your application
APPLICATION = data_collector

# If no BOARD is found in the environment, use this default:
BOARD ?= b-l072z-lrwan1

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Variables
LORA_REGION = EU868

# Packages
USEPKG += semtech-loramac

# Modules
USEMODULE += xtimer
USEMODULE += sx1276
USEMODULE += auto_init_loramac
USEMODULE += bmp280_i2c
USEMODULE += bh1750fvi

# Features
FEATURES_REQUIRED += cpp
FEATURES_OPTIONAL += periph_rtc

# Flags
CPPFLAGS += -DBMX280_PARAM_I2C_DEV=I2C_DEV\(0\)
CPPFLAGS += -DBMX280_PARAM_I2C_ADDR=0x76
CXXEXFLAGS += -std=c++17

include $(RIOTBASE)/Makefile.include
