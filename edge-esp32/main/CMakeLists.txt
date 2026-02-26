# Copyright (C) 2021 Intel Corporation and others.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        mqtt
        esp_wifi
        nvs_flash
        esp_driver_gpio
        esp_adc
        esp_event
        mdns
)

