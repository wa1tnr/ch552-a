/* Blink /*

/*
 *  refs
 *    Stefan Wagner
 *    https://easyeda.com/wagiminator
 *    using:  http://creativecommons.org/licenses/by-sa/3.0/
 *    for:
 *
https://github.com/wagiminator/CH552-USB-OLED/blob/main/software/cdc_oled_terminal/src/oled_term.c#L161-L197
 *    Thank you.
 */

void ard_delay(int ms) { delay(ms); }

#if 0
void serUSB_print_hex(char c) {
    if (c > 254) // 255 always false thanks compiler guy
        return;
    USBSerial_print(c, HEX);
}

void serUSB_write(char c) {
    c = c & 0x7F; // ignore top bit
    if (c > 254)
        return;
    if ((c > 31) || (c == '\r') || (c == '\n') || (c == '\010')) {
        USBSerial_write(c);
    }
}
#endif

void slowerE() {
    for (int p = 250; p > 0; p--) {
        ;
    }
}

void slowerD() {
    for (int p = 250; p > 0; p--) {
        slowerE();
    }
}

void slowerC() {
    for (int p = 25; p > 0; p--) {
        ;
        /* slowerD(); */
    }
}

void slowerB() {
    for (int p = 200; p > 0; p--) {
        slowerC();
    }
}

void slower() {
    for (int p = 200; p > 0; p--) {
        slowerB();
    }
}

extern void setupInterpreter();
extern void Interpreter();

void setup() {
    delay(1000);
    setupInterpreter();
}

void loop() { Interpreter(); }

/* end */
