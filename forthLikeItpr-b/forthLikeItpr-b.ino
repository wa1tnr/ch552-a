/* Wed  3 Jul 14:29:46 UTC 2024 */

/* Shattuck's Forth-like interpreter */

/*
 *  refs
 *    Stefan Wagner
 *    https://easyeda.com/wagiminator
 *    using:  http://creativecommons.org/licenses/by-sa/3.0/
 *    for:
 * https://github.com/wagiminator/CH552-USB-OLED/blob/main/software/cdc_oled_terminal/src/oled_term.c#L161-L197
 *    Thank you.
 */

void slowerC() {
    for (int p = 25; p > 0; p--) {
        ;
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