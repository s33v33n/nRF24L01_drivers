#   make              - build the project (driver, overlay, applications)
#   make clean        - remove build artifacts
#
#   --- HARDWARE (Kernel driver & Device Tree) ---
#   make driver       - build kernel module (driver/nrf24l01.ko)
#   make dtbo         - compile device tree overlay (.dts -> .dtbo)
#   make dtbo_clean   - remove compiled overlay file
#   make install      - install module and overlay permanently (auto-load on boot)
#   make uninstall    - remove module and overlay from system
#
#   --- APPLICATIONS ---
#   make master       - compile master application (NODE_ID=0)
#   make run_master   - run master application
#   make slave        - compile slave application (default NODE_ID=1)
#   make run_slave    - run slave application

# Module name (without .ko extension)
MODULE_NAME := nrf24l01
DRIVER_DIR := driver

# Device Tree Overlay source
DTS_FILE := $(DRIVER_DIR)/$(MODULE_NAME).dts
DTBO_FILE := $(DRIVER_DIR)/$(MODULE_NAME).dtbo

# Boot paths for permanent overlay installation
BOOT_OVERLAYS := /boot/overlays
BOOT_CONFIG   := /boot/config.txt

# Applications configuration
CC := gcc
CFLAGS := -Wall -Wextra -O2
INCLUDES := -Iinclude -Iencryption -Iframework
FRAMEWORK_SRC := framework/nrf_framework.c framework/nrf_packet.c encryption/Chacha20-Poly1305.c

# Target node ID (0 for Master, 1..N for Slave)
NODE_ID ?= 0

# Select application target based on NODE_ID (0 is Master, >0 is Slave)
ifeq ($(NODE_ID),0)
APP_TARGET := master
else
APP_TARGET := slave
endif

# Default target: build the module and selected application
all: driver dtbo install $(APP_TARGET)

# Build the kernel module in driver directory
driver:
	$(MAKE) -C $(DRIVER_DIR)

# Compile the Device Tree overlay (.dts -> .dtbo)
dtbo:
	$(MAKE) -C $(DRIVER_DIR) dtbo

# Remove the compiled Device Tree overlay
dtbo_clean:
	$(MAKE) -C $(DRIVER_DIR) dtbo_clean

# Permanent installation: copy module to kernel drivers and overlay to boot, then load immediately
install: driver dtbo
	sudo install -d /lib/modules/$$(uname -r)/kernel/drivers/spi
	sudo install -m 644 $(DRIVER_DIR)/$(MODULE_NAME).ko /lib/modules/$$(uname -r)/kernel/drivers/spi/
	sudo depmod -a
	echo "$(MODULE_NAME)" | sudo tee /etc/modules-load.d/$(MODULE_NAME).conf
	sudo install -m 644 $(DTBO_FILE) $(BOOT_OVERLAYS)/
	@grep -q "^dtoverlay=$(MODULE_NAME)" $(BOOT_CONFIG) || echo "dtoverlay=$(MODULE_NAME)" | sudo tee -a $(BOOT_CONFIG)
	@echo "Module and overlay installed permanently. Will load automatically on boot."
	@echo "Loading overlay and module into running system now..."
	@if lsmod | grep -q "^$(MODULE_NAME)"; then sudo rmmod $(MODULE_NAME) 2>/dev/null || true; fi
	@if command -v dtoverlay >/dev/null 2>&1; then \
		if sudo dtoverlay -l 2>/dev/null | grep -q "$(MODULE_NAME)"; then sudo dtoverlay -r $(MODULE_NAME) 2>/dev/null || true; fi; \
		sudo dtoverlay $(DTBO_FILE) 2>/dev/null || true; \
	fi
	sudo modprobe $(MODULE_NAME) 2>/dev/null || sudo insmod $(DRIVER_DIR)/$(MODULE_NAME).ko
	@echo "Driver is active and ready."

# Remove permanent installation from system and unload from kernel
uninstall:
	@if lsmod | grep -q "^$(MODULE_NAME)"; then sudo rmmod $(MODULE_NAME) 2>/dev/null || true; fi
	@if command -v dtoverlay >/dev/null 2>&1; then \
		if sudo dtoverlay -l 2>/dev/null | grep -q "$(MODULE_NAME)"; then sudo dtoverlay -r $(MODULE_NAME) 2>/dev/null || true; fi; \
	fi
	sudo rm -f /etc/modules-load.d/$(MODULE_NAME).conf
	sudo rm -f /lib/modules/$$(uname -r)/kernel/drivers/spi/$(MODULE_NAME).ko
	sudo depmod -a
	sudo rm -f $(BOOT_OVERLAYS)/$(MODULE_NAME).dtbo
	@if [ -f $(BOOT_CONFIG) ]; then sudo sed -i '/^dtoverlay=$(MODULE_NAME)/d' $(BOOT_CONFIG); fi
	@echo "Module and overlay uninstalled and unloaded."

# Compile master application
master: examples/master.c $(FRAMEWORK_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -DNODE_ID=0 examples/master.c $(FRAMEWORK_SRC) -o master
	@echo "Compiled master (NODE_ID=0)"

# Run master application
run_master: master
	./master

# Compile slave application
slave: examples/slave.c $(FRAMEWORK_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -DNODE_ID=$(NODE_ID) examples/slave.c $(FRAMEWORK_SRC) -o slave
	@echo "Compiled slave (NODE_ID=$(NODE_ID))"

# Run slave application
run_slave: slave
	./slave

# Remove all build artifacts
clean:
	$(MAKE) -C $(DRIVER_DIR) clean
	$(MAKE) -C $(DRIVER_DIR) dtbo_clean
	rm -f master slave

# these commands are just information for make, not output files
.PHONY: all clean dtbo dtbo_clean driver install uninstall master run_master slave run_slave
