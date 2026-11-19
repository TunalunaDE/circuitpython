# TEST BOARD (exploration fork only, not for upstream): ESP32-S3-DevKitC-1 N8R8 as a USB *host*.
# The chip has one USB controller and one PHY, so USB device (CIRCUITPY drive, USB REPL) is off.
# The REPL is on UART0 (the CH340 "UART" USB-C port of the DevKitC-1).
CIRCUITPY_CREATOR_ID = 0x000C303A
CIRCUITPY_CREATION_ID = 0x00531F01

IDF_TARGET = esp32s3

CIRCUITPY_ESP_FLASH_SIZE = 8MB
CIRCUITPY_ESP_FLASH_MODE = qio
CIRCUITPY_ESP_FLASH_FREQ = 80m

CIRCUITPY_ESP_PSRAM_SIZE = 8MB
CIRCUITPY_ESP_PSRAM_MODE = opi
CIRCUITPY_ESP_PSRAM_FREQ = 80m

CIRCUITPY_USB_DEVICE = 0
CIRCUITPY_USB_HOST = 1

# Native host and the MAX3421E SPI host are mutually exclusive in TinyUSB (CFG_TUH_MAX3421 replaces the DWC2 host driver).
CIRCUITPY_MAX3421E = 0
