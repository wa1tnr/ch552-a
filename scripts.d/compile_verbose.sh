#!/bin/sh
# Fri 21 Jun 16:52:18 UTC 2024


# // #define __MULLONG_ASM_SMALL_AUTO__

export LOCAL_ACLI_DIRECTOR_Y_PROJ_ECT="./sketch_jun30b"

prequel_a() {
    echo
    echo " make clean does this:"
    echo
    echo " rm -rf /tmp/arduino/cores ; rm -rf /tmp/arduino/sketches  # elbee"
    echo
}

prequel_b() {
    cat << _EOF

    RUNNING:

        arduino-cli compile --fqbn CH55xDuino:mcs51:ch552 ${1}

_EOF
}
prequel_c() {
    cat << _EOF
        # arduino-cli compile --fqbn CH55xDuino:mcs51:ch552 ${1}
        # TODO ESP32 Dev Module     esp32:esp32:esp32
_EOF
}

quiet_payload() {
    arduino-cli    compile --fqbn CH55xDuino:mcs51:ch552 \
         ${LOCAL_ACLI_DIRECTOR_Y_PROJ_ECT}
}

# // #define __MULLONG_ASM_SMALL_AUTO__

payload_16() {
    arduino-cli -v compile \
         --fqbn CH55xDuino:mcs51:ch552 \
         --board-options "bootloader_pin=p36" \
         --board-options "clock=16internal" \
         --board-options "upload_method=usb" \
         --board-options "usb_settings=usbcdc" \
         ${LOCAL_ACLI_DIRECTOR_Y_PROJ_ECT}

# --build-property "compiler.cpp.extra_flags=\"-D__SDCC_MODEL_SMALL\"" \
# --build-property "compiler.cpp.extra_flags=\"-D_MULLONG_ASM_SMALL_AUTO\"" \

# arduino-cli compile --fqbn MiniCore:avr:328 --board-options "clock=8MHz_internal" --board-options "<menu ID>=<option ID>"

# --board-options "clock=8MHz_internal" --board-options "<menu ID>=<option ID>"

               # P3.6 (D+) pull-up:   bootloader_pin=p36
               # 16 MHz (internal), 3.3V or 5V:  clock=16internal
               # USB:  upload_method=usb
               # Default CDC:  usb_settings=usbcdc
}

payload() {
    arduino-cli -v compile \
         --fqbn CH55xDuino:mcs51:ch552 \
         ${LOCAL_ACLI_DIRECTOR_Y_PROJ_ECT}
}

# warning level too high for this BSP toolchain CH55xDuino
# arduino-cli -v compile --warnings all

prequel() { # pick and choose
    # prequel_a
    prequel_b
    # prequel_c
}

do_all() {
    prequel
    # quiet_payload
    # payload
    payload_16
}

do_all

exit 0

# --warnings string  Optional, can be: none, default, more, all.
# Used to tell gcc which warning level to use (-W flag). (default "none")

# enId,

cat << _EOF__

Option:        USB Settings                                               usb_settings
               Default CDC                                  ✔             usb_settings=usbcdc
               USER CODE w/ 148B USB ram                                  usb_settings=user148
               USER CODE w/ 0B USB ram                                    usb_settings=user0
               USER CODE w/ 266B USB ram                                  usb_settings=user266
Option:        Upload method                                              upload_method
               USB                                          ✔             upload_method=usb
               Serial                                                     upload_method=serial
Option:        Clock Source                                               clock
               24 MHz (internal), 5V                        ✔             clock=24internal
               16 MHz (internal), 3.3V or 5V                              clock=16internal
               12 MHz (internal), 3.3V or 5V                              clock=12internal
               24 MHz (external 24M osc), 5V                              clock=24external
               16 MHz (external 24M osc), 3.3V or 5V                      clock=16external
Option:        Bootloader pin                                             bootloader_pin
               P3.6 (D+) pull-up                            ✔             bootloader_pin=p36
               P1.5 pull-down                                             bootloader_pin=p15
Programmers:   ID                                           Name
_EOF__

# end.
