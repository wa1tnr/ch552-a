/* test-aa.ino */
/* Fri 21 Jun 17:05:42 UTC 2024 */

#define CS_PIN 14

void waitUSB() {
    while (!USBSerial()) {
        ; // wait for serial port to connect.
          // Needed for native USB port only
    }
}

void gpio_setup() {

    /*
     * upstream has dWrite before pinMode
     * perhaps to avoid electrical issues
     * that may arise if not done in this sequence.
     */

    digitalWrite(CS_PIN, HIGH);
    pinMode(CS_PIN, OUTPUT);
}

void setup() { USBSerial_println("ASCII Table ~ Character Map"); }

void loop() {}

// end.
