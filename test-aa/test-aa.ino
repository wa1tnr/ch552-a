/* test-aa.ino */
/* Fri 21 Jun 17:05:42 UTC 2024 */

void waitUSB() {
    while (!USBSerial()) {
        ; // wait for serial port to connect.
          // Needed for native USB port only
    }
}

void setup() { USBSerial_println("ASCII Table ~ Character Map"); }

void loop() {}

// end.
