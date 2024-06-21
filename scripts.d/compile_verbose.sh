#!/bin/sh
# Fri 21 Jun 16:52:18 UTC 2024

export LOCAL_ACLI_DIRECTOR_Y_PROJ_ECT="./test-aa"

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

payload() {
    arduino-cli -v compile \
         --fqbn CH55xDuino:mcs51:ch552 \
         ${LOCAL_ACLI_DIRECTOR_Y_PROJ_ECT}
}

# warning level too high for this BSP toolchain CH55xDuino
# arduino-cli -v compile --warnings all \

prequel() { # pick and choose
    # prequel_a
    prequel_b
    # prequel_c
}

do_all() {
    prequel
    # quiet_payload
    # payload
    payload
}

do_all

exit 0

# --warnings string  Optional, can be: none, default, more, all.
# Used to tell gcc which warning level to use (-W flag). (default "none")

# end.
