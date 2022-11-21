# name of your application
APPLICATION = engine_driver_pbl5

# If no BOARD is found in the environment, use this default:
BOARD ?= b-l072z-lrwan1

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Use a peripheral timer for the delay, if available
USEMODULE += xtimer
USEMODULE += periph_adc
USEMODULE += periph_gpio_irq
USEMODULE += core_msg


include $(RIOTBASE)/Makefile.include
