#!/bin/bash
echo "=== Script doesn't check wiring - check it by yourself ==="

# Optional default configuration generation
read -p "Do you want to generate default network configuration? [y/N]: " gen_conf
if [[ "$gen_conf" =~ ^[Yy]$ ]]; then
    tools/auto_config.sh
fi

# user's data validation
gcc -Wall -Werror tools/data_validator.c -o tools/validator
if ! ./tools/validator; then
    echo "Check your input data"
    exit 1
fi

# ============= DeviceTree =============

# ---- Hardware Configuration File ----
HW_CONFIG_FILE="user_config/hardware_config.h"

# ---- SPI0 ----
ENABLE_SPI0=$(awk '/^[ \t]*#define RADIO_SPI0_ENABLE/ {print $3}' "$HW_CONFIG_FILE")
if [ "$ENABLE_SPI0" = "1" ]; then STATUS_SPI0="okay"; else STATUS_SPI0="disabled"; fi

DEVICE_NAME_SPI0=$(awk '/^[ \t]*#define DEV_NAME_SPI0/ {print $3}' "$HW_CONFIG_FILE" | tr -d '"')
CE_PIN_SPI0=$(awk '/^[ \t]*#define CE_PIN_SPI0/ {print $3}' "$HW_CONFIG_FILE")
IRQ_PIN_SPI0=$(awk '/^[ \t]*#define IRQ_PIN_SPI0/ {print $3}' "$HW_CONFIG_FILE")
SPEED_SPI0=$(awk '/^[ \t]*#define SPI0_SPEED/ {print $3}' "$HW_CONFIG_FILE")
CHIP_SELECT_PIN_SPI0=$(awk '/^[ \t]*#define CS_PIN_SPI0/ {print $3}' "$HW_CONFIG_FILE")

# ---- SPI1 ----
ENABLE_SPI1=$(awk '/^[ \t]*#define RADIO_SPI1_ENABLE/ {print $3}' "$HW_CONFIG_FILE")
if [ "$ENABLE_SPI1" = "1" ]; then STATUS_SPI1="okay"; else STATUS_SPI1="disabled"; fi

DEVICE_NAME_SPI1=$(awk '/^[ \t]*#define DEV_NAME_SPI1/ {print $3}' "$HW_CONFIG_FILE" | tr -d '"')
CE_PIN_SPI1=$(awk '/^[ \t]*#define CE_PIN_SPI1/ {print $3}' "$HW_CONFIG_FILE")
IRQ_PIN_SPI1=$(awk '/^[ \t]*#define IRQ_PIN_SPI1/ {print $3}' "$HW_CONFIG_FILE")
SPEED_SPI1=$(awk '/^[ \t]*#define SPI1_SPEED/ {print $3}' "$HW_CONFIG_FILE")
MISO_PIN_SPI1=$(awk '/^[ \t]*#define SPI1_MISO_PIN/ {print $3}' "$HW_CONFIG_FILE")
MOSI_PIN_SPI1=$(awk '/^[ \t]*#define SPI1_MOSI_PIN/ {print $3}' "$HW_CONFIG_FILE")
SCLK_PIN_SPI1=$(awk '/^[ \t]*#define SPI1_SCLK_PIN/ {print $3}' "$HW_CONFIG_FILE")
CHIP_SELECT_PIN_SPI1=$(awk '/^[ \t]*#define CS_PIN_SPI1/ {print $3}' "$HW_CONFIG_FILE")


sed -e "s/__STATUS_0__/${STATUS_SPI0}/g" \
    -e "s/__DEV_NAME_0__/${DEVICE_NAME_SPI0}/g" \
    -e "s/__CE_PIN_0__/${CE_PIN_SPI0}/g" \
    -e "s/__IRQ_PIN_0__/${IRQ_PIN_SPI0}/g" \
    -e "s/__SPI_SPEED_0__/${SPEED_SPI0}/g" \
    -e "s/__CS_PIN_SPI0__/${CHIP_SELECT_PIN_SPI0}/g" \
    -e "s/__STATUS_1__/${STATUS_SPI1}/g" \
    -e "s/__DEV_NAME_1__/${DEVICE_NAME_SPI1}/g" \
    -e "s/__CE_PIN_1__/${CE_PIN_SPI1}/g" \
    -e "s/__IRQ_PIN_1__/${IRQ_PIN_SPI1}/g" \
    -e "s/__SPI_SPEED_1__/${SPEED_SPI1}/g" \
    -e "s/__SPI1_MISO__/${MISO_PIN_SPI1}/g" \
    -e "s/__SPI1_MOSI__/${MOSI_PIN_SPI1}/g" \
    -e "s/__SPI1_SCLK__/${SCLK_PIN_SPI1}/g" \
    -e "s/__CS_PIN_SPI1__/${CHIP_SELECT_PIN_SPI1}/g" \
    driver/nrf24l01.dts.template > driver/nrf24l01.dts

# ============= DeviceTree =============

# ============= Compilation & Installation =============
if [ -n "$1" ]; then
    NODE_ID="$1"
else
    read -p "Enter Node ID to compile (0 for Master, 1..N for Slave) [default: 0]: " NODE_ID
    NODE_ID=${NODE_ID:-0}
fi

if ! ./tools/validator "$NODE_ID"; then
    echo "Invalid NODE_ID selected. Aborting build."
    exit 1
fi

echo "Building and installing for NODE_ID=${NODE_ID}..."
make all NODE_ID="${NODE_ID}"