1. Connect the nRF modules to the pins specified in the `.dts` file.
2. Compile and load the DeviceTree overlay into the system using the `make dtbo_load` command.
3. Build and load the driver by running the `make` and `make load` commands.
4. Verify that the driver loaded successfully by:
    - checking the logs with `make dmesg`.
    - checking major number of the device files in `/proc/devices` (cat /proc/devices | grep nrf24l01).
    - checking device class in `/sys/class` (ls -ld /sys/class/nrf24l01).
