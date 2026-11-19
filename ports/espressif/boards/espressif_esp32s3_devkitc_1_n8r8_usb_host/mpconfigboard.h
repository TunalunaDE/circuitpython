// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2019 Scott Shawcroft for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#pragma once

// Micropython setup

#define MICROPY_HW_BOARD_NAME       "ESP32-S3-DevKitC-1-N8R8 USB host (test)"
#define MICROPY_HW_MCU_NAME         "ESP32S3"

#define MICROPY_HW_NEOPIXEL         (&pin_GPIO48)

#define DEFAULT_UART_BUS_RX         (&pin_GPIO44)
#define DEFAULT_UART_BUS_TX         (&pin_GPIO43)

// No USB device on this build: the REPL runs on UART0 (CH340 USB-C port).
#define CIRCUITPY_CONSOLE_UART_TX   (&pin_GPIO43)
#define CIRCUITPY_CONSOLE_UART_RX   (&pin_GPIO44)

// USB host on the only (full-speed) controller: D- GPIO19, D+ GPIO20.
#define CIRCUITPY_USB_HOST_INSTANCE 0
