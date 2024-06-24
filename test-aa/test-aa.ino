/* Blink
  Most Arduinos have an on-board LED you can control. On the simpleCH552
  it is attached to digital pin P3.3 */

/* *******************************************************************
   **********************                 ****************************
   **********************   w a n t e d   ****************************
   **********************                 ****************************
   **********************                 ****************************
   ******************************************************************* */

/*
 *    wanted:   wrappers.  Anything provided by the .ino
 *              that does not 'work' in interpreter.c in an
 *              obvious way.
 *
 *              Commented out, for example, all Serial.print()
 *              uses.  A simple wrapper may solve that neatly.
 *
 */

#define LED_BUILTIN 16

/* char __data buffer[64]; */

/* char *__data bufPtr; */
/* char *__data */


/*

  [ https://stackoverflow.com/questions/63003227/vscode-not-able-to-resue-env-variables-set-inside-terminal-integrated-env-wind ]

In your user settings (File > Preferences > Settings), add the following:

"terminal.integrated.allowWorkspaceConfiguration": true

fzf
*/

/* void putLine(char *buffer) { */
/* void putLine() { */
/* void putLine(__data char * buffer) { */
/* void putLine(__data char * buffer) { */

/* USBSerial_println(buffer); */


  /* USBSerial_print(buffer); */
  /* USBSerial_print("frank in sense"); */
  /* USBSerial_print(*buffer); */


/* void testPutLine() { */
  /*
      GOOD: bufPtr = * buffer;
            bufPtr = buffer;
            {
            bufPtr = (char *__data) buffer;
            USBSerial_println(buffer);
            }
  */

  /*
      BAD: bufPtr = & buffer;
            {
            bufPtr = (char *__data) buffer;
            USBSerial_println(bufPtr);
            }
  */

/*
  bufPtr = (char *__data) buffer;
*/
  /* compiles clean but cannot find how to use it here for this task */

/*
  USBSerial_println(buffer);
  */

  /* USBSerial_println(buffer); */
  /* USBSerial_println(bufPtr); */
  /* snprintf(buffer, sizeof buffer, "%s", "abcde"); */

/*
}
*/


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
  delay(2000);
  /* testPutLine(); */

  setupInterpreter();
  for (;;) { Interpreter(); }
  for (int index = 225; index >0; index--) {
    USBSerial_print("ardcli I am 8051 defgh.. ");
    USBSerial_println(index);
    USBSerial_flush();
    slower();
  }

  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}

/* end */
