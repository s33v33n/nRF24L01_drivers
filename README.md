1. Connect the nRF modules to the pins specified in the `.dts` file.
2. Compile and load the DeviceTree overlay into the system using the `make dtbo_load` command.
3. Build and load the driver by running the `make` and `make load` commands.
4. Verify that the driver was loaded successfully by:
    - checking the logs with `make dmesg`.
    - checking major number of the device files in `/proc/devices` (cat /proc/devices | grep nrf24l01).
    - checking device class in `/sys/class` (ls -ld /sys/class/nrf24l01).
    - checking files created in /dev/ (ls -ld /dev/nrf24l01*).
    - checking writing operation works correctly(sudo chmod 666 /dev/nrf24l01_0; echo "hello" > /dev/nrf24l01_0; make dmesg)
5. Test radio transmission between modules:
    - for receiver open another terminal and run `sudo cat /dev/nrf24l01_0`
    - for transmitter `echo "Hi" | sudo tee /dev/nrf24l01_1`
    - then check logs `make dmesg`, you may be required to resend the message because of transmission medium
6. The poll() mechnism is supported