/* Blink
  Most Arduinos have an on-board LED you can control. On the simpleCH552
  it is attached to digital pin P3.3 */

/* *******************************************************************
 **********************                 ****************************
 **********************   w a n t e d   ****************************
 **********************                 ****************************
 ******************************************************************* */

/*
 *
 */

#define LED_BUILTIN 16

/*

  [
https://stackoverflow.com/questions/63003227/vscode-not-able-to-resue-env-variables-set-inside-terminal-integrated-env-wind
]

In your user settings (File > Preferences > Settings), add the following:

"terminal.integrated.allowWorkspaceConfiguration": true

fzf
*/

/*
 *  refs
 *    Stefan Wagner
 *    https://easyeda.com/wagiminator
 *    using:  http://creativecommons.org/licenses/by-sa/3.0/
 *    for:
 *      https://github.com/wagiminator/CH552-USB-OLED/blob/main/software/cdc_oled_terminal/src/oled_term.c#L161-L197
 *    Thank you.
 */

void ard_delay(int ms) { delay(ms); }

uint8_t serUSB_available() {
    uint8_t avail = USBSerial_available();
    return avail;
}

char serUSB_read() {
    char c = USBSerial_read();
    return c;
}

void serUSB_print_int(int i) { USBSerial_print(i); }

void serUSB_print_hex_int(int i) { USBSerial_print(i, HEX); }

void serUSB_flush() { USBSerial_flush(); }

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

// serUSB print string
void serUSB_print(char *str) {
    while (*str)
        serUSB_write(*str++);
}

// serUSB print string with newline
void serUSB_println(char *str) {
    serUSB_print(str);
    serUSB_write('\n');
}

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

void msg_longstars() { serUSB_println("   * * * * * * * *"); }

void msg_starframed() { serUSB_println("   * * *       * * *"); }

void print_cool_things(int count) {
    serUSB_println("darmok and gilad at tenagra eyes open 8051 pbrsgng.. ");
    serUSB_println("");
    USBSerial_print(count);
    serUSB_println("");

    msg_longstars();

    msg_starframed();

    serUSB_println("   * * *                     s t a t e                     "
                   "      * * *");
    msg_starframed();
    serUSB_println("   * * *         four eighty-seven Mars Std Time           "
                   "      * * *");
    msg_starframed();
    serUSB_println("   * * *           Sat 29 Jun 17:35:18 UTC 2024            "
                   "      * * *");
    msg_starframed();
    msg_longstars();

    /****
     *
     *
     */

    msg_starframed();
    msg_longstars();
    serUSB_flush();
    slower();
}

void do_cool_things() {
    for (int index = 225; index > 0; index--) {
        int count = index;
        print_cool_things(count);
        delay(1400);
    }
    pinMode(LED_BUILTIN, OUTPUT);
}

void dutyBlink() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}

extern void setupInterpreter();
extern void Interpreter();

void setup() {
    delay(4000);
    delay(1000);
    setupInterpreter();
}

void loop() { Interpreter(); }

/* end */
