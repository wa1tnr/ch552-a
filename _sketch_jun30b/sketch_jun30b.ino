/* Mon  1 Jul 18:11:33 UTC 2024 */
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

/* TODO: remove ard_delay not needed */
void ard_delay(int ms) { delay(ms); }

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
