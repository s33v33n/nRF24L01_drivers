#   make              - build the kernel module
#   make clean        - remove build artifacts
#
#   --- TEMPORARY (lost after reboot) ---
#   make load         - load module directly from file (insmod, temporary)
#   make unload       - remove module from the running kernel
#   make reload       - unload + load again (useful during development)
#   make dtbo_load    - compile and load overlay at runtime (temporary)
#   make dtbo_unload  - unload overlay from the running system
#
#   --- UTILS ---
#   make dtbo         - compile device tree overlay (.dts -> .dtbo)
#   make dtbo_clean   - remove compiled overlay file
#   make dmesg        - show last 30 kernel log lines
#
#   --- IOCTL ---
#   make make_ioctl   - compile IOCTL program
#   make run_ioctl    - compile & run IOCTL program
#
#   --- My app ---
#   make make_app 	  - compile my app
#   make run_app_n    - compile & run my app , n is device number
#   make make_sniffer - compile sniffer
#   make run_sniffer  - compile & run sniffer

# Module name (without .ko extension)
MODULE_NAME := nrf24l01

# Source file(s)
obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-y := nrf24l01_driver.o nrf_hal.o

# Kernel source directory (auto-detect running kernel)
KDIR ?= /lib/modules/$(shell uname -r)/build

# Current working directory
PWD := $(shell pwd)

# Device Tree Overlay source
DTS_FILE := $(MODULE_NAME).dts
DTBO_FILE := $(MODULE_NAME).dtbo

# Default target: build the module
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Remove all build artifacts
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f test_ioctl
	rm -f secure_chat
	rm -f sniffer	

# Load module directly from .ko file (temporary - lost after reboot)
load:
	sudo insmod $(MODULE_NAME).ko
	@echo "Module loaded (temporary - will be gone after reboot)."

# Remove module from the running kernel
unload:
	sudo rmmod $(MODULE_NAME)
	@echo "Module unloaded."

# Reload: unload and load again (useful during development)
reload: unload load

# Compile the Device Tree overlay (.dts -> .dtbo)
dtbo:
	dtc -@ -I dts -O dtb -o $(DTBO_FILE) $(DTS_FILE)

# Remove the compiled Device Tree overlay
dtbo_clean:
	rm -f $(DTBO_FILE)

# Load overlay into running system (temporary - lost after reboot)
dtbo_load: dtbo
	sudo dtoverlay $(DTBO_FILE)
	@echo "Overlay loaded (temporary - will be gone after reboot)."

# Unload overlay from the running system
dtbo_unload:
	sudo dtoverlay -r $(MODULE_NAME)
	@echo "Overlay unloaded."

# Compile IOCTL 
make_ioctl:
	gcc test_ioctl.c -o test_ioctl

# Run IOCTL program
run_ioctl: make_ioctl
	sudo ./test_ioctl

# Compile my app - secure_chat 
make_app:
	gcc secure_chat.c -o secure_chat

# Run my app - device 0
run_app_0: make_app
	sudo ./secure_chat /dev/nrf24l01_0 1234 7777

# Run my app - device 1
run_app_1: make_app
	sudo ./secure_chat /dev/nrf24l01_1 1234 9999

# Compile my app - secure_chat 
make_sniffer:
	gcc sniffer.c -o sniffer

# Run my app - device 0
run_sniffer: make_sniffer
	sudo ./sniffer

# Show kernel logs 
dmesg:
	sudo dmesg | tail -30

# these commands are just information for make, not output files
.PHONY: all clean load unload reload dtbo dtbo_clean dtbo_load dtbo_unload dmesg make_ioctl run_ioctl make_app run_app_0 run_app_1 sniffer
