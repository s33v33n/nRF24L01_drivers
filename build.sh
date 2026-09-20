#!/bin/bash
echo "=== Script doesn't check wiring - check it by yourself ==="

# user's data validation
gcc -Wall -Werror tools/data_validator.c -o tools/validator
if ! ./tools/validator; then
    echo "Check your input data"
    exit 1
fi

# ============= DeviceTree =============

# ---- SPI0 ----
ENABLE_SPI0=$(awk '/^[ \t]*#define RADIO_SPI0_ENABLE/ {print $3}' network_config.h)
if [ "$ENABLE_SPI0" = "1" ]; then STATUS_SPI0="okay"; else STATUS_SPI0="disabled"; fi

DEVICE_NAME_SPI0=$(awk '/^[ \t]*#define DEV_NAME_SPI0/ {print $3}' network_config.h | tr -d '"')
CE_PIN_SPI0=$(awk '/^[ \t]*#define CE_PIN_SPI0/ {print $3}' network_config.h)
IRQ_PIN_SPI0=$(awk '/^[ \t]*#define IRQ_PIN_SPI0/ {print $3}' network_config.h)
SPEED_SPI0=$(awk '/^[ \t]*#define SPI0_SPEED/ {print $3}' network_config.h)
CHIP_SELECT_PIN_SPI0=$(awk '/^[ \t]*#define CS_PIN_SPI0/ {print $3}' network_config.h)

# ---- SPI1 ----
ENABLE_SPI1=$(awk '/^[ \t]*#define RADIO_SPI1_ENABLE/ {print $3}' network_config.h)
if [ "$ENABLE_SPI1" = "1" ]; then STATUS_SPI1="okay"; else STATUS_SPI1="disabled"; fi

DEVICE_NAME_SPI1=$(awk '/^[ \t]*#define DEV_NAME_SPI1/ {print $3}' network_config.h | tr -d '"')
CE_PIN_SPI1=$(awk '/^[ \t]*#define CE_PIN_SPI1/ {print $3}' network_config.h)
IRQ_PIN_SPI1=$(awk '/^[ \t]*#define IRQ_PIN_SPI1/ {print $3}' network_config.h)
SPEED_SPI1=$(awk '/^[ \t]*#define SPI1_SPEED/ {print $3}' network_config.h)
MISO_PIN_SPI1=$(awk '/^[ \t]*#define SPI1_MISO_PIN/ {print $3}' network_config.h)
MOSI_PIN_SPI1=$(awk '/^[ \t]*#define SPI1_MOSI_PIN/ {print $3}' network_config.h)
SCLK_PIN_SPI1=$(awk '/^[ \t]*#define SPI1_SCLK_PIN/ {print $3}' network_config.h)
CHIP_SELECT_PIN_SPI1=$(awk '/^[ \t]*#define CS_PIN_SPI1/ {print $3}' network_config.h)


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

echo "nrf24l01.dts successfully generated"

# ============= DeviceTree =============