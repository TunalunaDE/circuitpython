// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2026 Limor Fried for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "bindings/espidf/__init__.h"
#include "common-hal/microcontroller/Pin.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "shared-bindings/usb_host/Port.h"
#include "supervisor/usb.h"
#include "supervisor/shared/tick.h"
#include "esp_private/usb_phy.h"
#include "tusb.h"

#if defined(CONFIG_IDF_TARGET_ESP32P4)
#if CIRCUITPY_USB_HOST_INSTANCE != 1
#error "USB host requires the ESP32-P4 high-speed controller"
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32S31)
// The S31 has only the high-speed controller, as TinyUSB rhport 0.
#if CIRCUITPY_USB_HOST_INSTANCE != 0
#error "USB host requires the ESP32-S31 high-speed controller"
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
// The S2 and S3 have one full-speed OTG controller (TinyUSB rhport 0) on the
// internal PHY, on GPIO19 (D-) and GPIO20 (D+). That controller and PHY are the
// same ones USB device and USB-Serial/JTAG use, so a host build must turn both off.
#if CIRCUITPY_USB_HOST_INSTANCE != 0
#error "USB host requires the full-speed controller (CIRCUITPY_USB_HOST_INSTANCE 0)"
#endif
#if CIRCUITPY_USB_DEVICE
#error "USB host and USB device cannot both use the only USB controller on this chip"
#endif
#if CIRCUITPY_ESP_USB_SERIAL_JTAG
#error "USB host cannot share the USB PHY with USB-Serial/JTAG"
#endif
#else
#error "USB host is not supported on this chip"
#endif
#if CIRCUITPY_USB_DEVICE && CIRCUITPY_USB_DEVICE_INSTANCE == CIRCUITPY_USB_HOST_INSTANCE
#error "USB host and device must use different controllers"
#endif

#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
#define USB_HOST_DP (&pin_GPIO20)
#define USB_HOST_DM (&pin_GPIO19)
#define USB_HOST_PHY_TARGET USB_PHY_TARGET_INT
#define USB_HOST_PHY_SPEED USB_PHY_SPEED_FULL
#else
// The high-speed PHY has dedicated pads, not GPIOs.
#define USB_HOST_DP (&pin_USB_HS_DP)
#define USB_HOST_DM (&pin_USB_HS_DM)
#define USB_HOST_PHY_TARGET USB_PHY_TARGET_UTMI
#define USB_HOST_PHY_SPEED USB_PHY_SPEED_HIGH
#endif

static usb_host_port_obj_t usb_host_instance;
static usb_phy_handle_t host_phy;

usb_host_port_obj_t *common_hal_usb_host_port_construct(const mcu_pin_obj_t *dp, const mcu_pin_obj_t *dm) {
    if (dp != USB_HOST_DP || dm != USB_HOST_DM) {
        raise_ValueError_invalid_pins();
    }
    usb_host_port_obj_t *self = &usb_host_instance;
    // On the S2/S3 the pads are real GPIOs, but Pin.c never resets them and marks
    // them in use at every reset (as for USB device), so Python cannot take them.
    if (self->dp != NULL) {
        return self;
    }

    // usb_new_phy() in host mode also connects the D+/D- pull-downs and
    // forces the A-side / VBUS-valid signals, so the controller runs as host.
    const usb_phy_config_t config = {
        .controller = USB_PHY_CTRL_OTG,
        .target = USB_HOST_PHY_TARGET,
        .otg_mode = USB_OTG_MODE_HOST,
        .otg_speed = USB_HOST_PHY_SPEED,
    };
    CHECK_ESP_RESULT(usb_new_phy(&config, &host_phy));
    if (!tuh_init(TUH_OPT_RHPORT)) {
        usb_del_phy(host_phy);
        host_phy = NULL;
        CHECK_ESP_RESULT(ESP_FAIL);
    }
    self->base.type = &usb_host_port_type;
    self->dp = dp;
    self->dm = dm;
    // Keep polling enumeration timers even without an active display.
    supervisor_enable_tick();
    return self;
}

// TinyUSB installs the controller IRQ. Process its events on the VM task,
// where the Python USB device and keyboard callbacks are safe to run.
void tuh_event_hook_cb(uint8_t rhport, uint32_t eventid, bool in_isr) {
    // Don't wait for the background tick: an isochronous stream must queue its
    // next transfer within the frame.
    usb_background_schedule();
}
