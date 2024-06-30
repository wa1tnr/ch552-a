/* interpreter.c - Shattuck's Forth-like interpreter - port to CH552 8051 MCU */
/* June, 2024 */

// #define __MULLONG_ASM_SMALL_AUTO__
// # defined(_MULLONG_ASM_SMALL_AUTO)

// might help sdcc-lib.h -- might select right variant
// or mcs51 specific header not yet included

// result - no help from sdcc-lib.h so far

// alligator
#include <sdcc-lib.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
/***
 *
 *
 * include <string.h>
 *
 *   strcpy
 *
 *
 * include <stdlib.h>
 *
 *   extern long int strtol(const char *nptr, char **endptr, int base);
 *
 */


#define LED_BUILTIN 33
const int ledPin = LED_BUILTIN; // the number of the LED pin

extern uint8_t serUSB_available();
extern char serUSB_read();

// extern void serUSB_print_long(long i);
extern void serUSB_print_int(int i);
extern void serUSB_print_hex_int(int i);
extern void serUSB_write(char c);
extern void serUSB_print_hex(char c);
extern void serUSB_print(char *str);
extern void serUSB_println(char *str);
extern void serUSB_flush();
extern void ard_delay(int ms);

#define LIMIT_OUTER_ONE_EACH_ 0x1E00

#define SEE_LINE()                                                             \
    serUSB_print("   see: interpreter.c  LINE ");                              \
    serUSB_print_int(__LINE__);                                                \
    serUSB_println("")

uint8_t slowerThan = 0;

__code int j = 0x7e;

// cannot do anything here - likely is boot area:
__code __at(0x0) char ORG;

// __code __at (0x0) char ORG[64] = "seasons don't fear the reaper .. Nor do the
// wind, the sun or the rain";

// __code char smallORG[72] =
//    "seasons don't fear the reaper .. Nor do the wind, the sun or the rain";
//  [72] =
//  "123456781234567812345678123456781234567812345678123456781234567812345678"
//  [72] = "       8      16      24      32      40      48      56      64 72"
//  [80] =
//  "12345678123456781234567812345678123456781234567812345678123456781234567812345678"
//  [80] = "       8      16      24      32      40      48      56      64 72
//  80"

int *ORG_ptr = &ORG; // okay for reading only
int *jaddr = &j;

/* Tiny interpreter,
   similar to myforth's Standalone Interpreter
   This example code is in the public domain */

/* Structure of a dictionary entry */
typedef struct {
    const char *name;
    void (*function)();
} entry;

/* Data stack for parameter passing
   This "stack" is circular,
   like a Green Arrays F18A data stack,
   so overflow and underflow are not possible
   Number of items must be a power of 2 */
const uint8_t STKSIZE = 8;
const uint8_t STKMASK = 7;
int stack[8];
int p = 0;

/* TOS is Top Of Stack */
#define TOS stack[p]
/* NAMED creates a string in flash */
#define NAMED(x, y) const char x[] = y

/* Terminal Input Buffer for interpreter */
/* const byte maxtib = 16; */ /* compiler complains */

const uint8_t maxtib = 16;
char tib[16];

/* buffer required for strings read from flash */
char namebuf[16];
uint8_t pos;

/* push n to top of data stack */
void push(int n) {
    p = (p + 1) & STKMASK;
    serUSB_print("  push(n)  has received: ");
    serUSB_print_hex_int(n); serUSB_println(""); serUSB_flush();
    TOS = n;
}

/* return top of stack */
int pop() {
    int n = TOS;
    serUSB_print("  pop harvested n: ");
    serUSB_print_int(n);
    serUSB_flush();
    p = (p - 1) & STKMASK;
    serUSB_println("  pop()  will return: ");
    serUSB_print_int(n);
    serUSB_flush();
    return n;
}

/* Global delay timer */
uint8_t spd = 15;

/* top of stack becomes current spd */
NAMED(_speed, "speed");
void speed() { spd = pop(); }

/* discard top of stack */
NAMED(_drop, "drop");
void drop() { pop(); }

/* recover dropped stack item */
NAMED(_back, "back");
void back() {
    for (uint8_t i = 1; i < STKSIZE; i++)
        drop();
}

/* copy top of stack */
NAMED(_dup, "dup");
void dup() { push(TOS); }

/* exchange top two stack items */
NAMED(_swap, "swap");
void swap() {
    uint8_t a;
    uint8_t b;
    a = pop();
    b = pop();
    push(a);
    push(b);
}

/* copy second on stack to top */
NAMED(_over, "over");
void over() {
    uint8_t a;
    uint8_t b;
    a = pop();
    b = pop();
    push(b);
    push(a);
    push(b);
}
/* add top two items */
NAMED(_add, "+");
void add() {
    uint8_t a = pop();
    TOS = a + TOS;
}

/* bitwise and top two items */
NAMED(_and, "and");
void and_() {
    uint8_t a = pop();
    TOS = a & TOS;
}

/* inclusive or top two items */
NAMED(_or, "or");
void or_() {
    uint8_t a = pop();
    TOS = a | TOS;
}

/* exclusive or top two items */
NAMED(_xor, "xor");
void xor_() {
    uint8_t a = pop();
    TOS = a ^ TOS;
}

/* invert all bits in top of stack */
NAMED(_invert, "invert");
void invert() { TOS = ~(TOS); }

/* negate top of stack */
NAMED(_negate, "negate");
void negate() { TOS = -(TOS); }

/* destructively display top of stack, decimal */
NAMED(_dot, ".");
void dot() {
    serUSB_print_int(pop());
    serUSB_write(' '); serUSB_flush();
    /* Serial.print(pop()); */
    /* Serial.print(" "); */
}

/* destructively display top of stack, hex */
NAMED(_dotHEX, ".h");
void dotHEX() {
    // serUSB_print_hex_int(0xffff & pop(), HEX);
    serUSB_print_hex_int(0xffff & pop());
    serUSB_write(' '); serUSB_flush();
    /* Serial.print(0xffff & pop(), HEX); */
    /* Serial.print(" "); */
}

/* display whole stack, hex */
NAMED(_dotShex, ".sh");
void dotShex() {
    for (uint8_t i = 0; i < STKSIZE; i++)
        dotHEX();
}

/* display whole stack, decimal */
NAMED(_dotS, ".s");
void dotS() {
    for (uint8_t i = 0; i < STKSIZE; i++)
        dot();
}

/* delay TOS # of milliseconds */
NAMED(_delay, "delay");
void del() { /* delay(pop()); */
}

/* Toggle pin at TOS and delay(spd), repeat... */
NAMED(_wiggle, "wiggle");
void wiggle() {
    uint8_t a = pop();
    /* pinMode(a, OUTPUT); */
    for (uint8_t i = 0; i < 20; i++) {
        /* digitalWrite(a, HIGH); */
        /* delay(100); */
        /* digitalWrite(a, LOW); */
        /* delay(100); */
    }
}

/* TOS is pin number, set it HIGH */
NAMED(_high, "high");
void high() { /* digitalWrite(pop(), HIGH); */
}

/* set TOS pin LOW */
NAMED(_low, "low");
void low() { /* digitalWrite(pop(), LOW); */
}

/* read TOS pin */
NAMED(_in, "in");
void in() { /* TOS = digitalRead(TOS); */
}

/* make TOS pin an input */
NAMED(_input, "input");
void input() { /* pinMode(pop(), INPUT); */
}

/* make TOS pin an output */
NAMED(_output, "output");
void output() { /* pinMode(pop(), OUTPUT); */
}

/* make TOS pin an input with weak pullup */
NAMED(_input_pullup, "input_pullup");
void input_pullup() { /* pinMode(pop(), INPUT_PULLUP); */
}

void printZeds(int pvr) {
    if (pvr > 0xFFF) {
        return;
    }
    serUSB_print("0"); // pad print job with leading zero
    serUSB_flush();
    if (pvr > 0xFF) {
        return;
    }
    if (pvr < 0x100) {
        serUSB_print("0");
        serUSB_flush();
        if (pvr > 0x0F) {
            return;
        }
    }
    serUSB_print("0");
    serUSB_flush();
}

/* dump 16 bytes of RAM in hex with ascii on the side */
void dumpRAM() {
    char buffer[5] = "";

    char *ram;
    // ram = (char *)jaddr;
    ram = (char *)ORG_ptr;
    serUSB_print("!");
    serUSB_print(" 0x");

    int pvr = (int)ram;

    printZeds(pvr);
    serUSB_print_hex_int(pvr);

    /***********************
     *                     *
     *    2 4 6 8 A C E    *
     *   1 3 5 7 9 B D F   *
     *                     *
     ***********************/

    serUSB_write(':');
    serUSB_write(' ');
    serUSB_print("   ");

    /* individual hex 2-digit groups l to r */
    for (uint8_t i = 0; i < 16; i++) {
        char c = *ram++;
        c = c & 0xff;
        if (c < 0x10) {
            serUSB_print("0"); // padding
        }
        serUSB_print_hex(c);
        serUSB_write(' ');
        serUSB_flush();
    }
    // ram = (char *)jaddr;
    ram = (char *)ORG_ptr;
    serUSB_print("   ");
    for (uint8_t i = 0; i < 16; i++) {
        buffer[0] = *ram++;
        if (buffer[0] > 0x7f || buffer[0] < ' ')
            buffer[0] = '.';
        buffer[1] = '\0';
        serUSB_print(buffer);
        serUSB_flush();
    }
    for (int iter = 8; iter > 0; iter--) {
        // jaddr++;
        ORG_ptr++;
    }

    if (slowerThan) {
        ard_delay(23);
    }

    int tested = (int)ram;

    // if (tested > 0xFF) {
    if (tested > 0x4000) {
        serUSB_println("");
        serUSB_print("    __code address 0x");
        serUSB_print_hex_int(tested);
        serUSB_println(" may be a DANGER  ");
        SEE_LINE();
        serUSB_flush();
        while (-1) {
            ard_delay(2000);
            serUSB_write('.');
            serUSB_flush();
        }
    }
}

/* dump 256 bytes of RAM */
NAMED(_dumpr, "dump");
void rdumps() {
    for (uint8_t i = 0; i < 16; i++) {
        serUSB_println("");
        dumpRAM();
    }
    serUSB_println("");
    serUSB_flush();
}

/* End of Forth interpreter words */
/* ******************************************** */
/* Beginning of application words */

/* End of application words */
/* ******************************************** */
/* Now build the dictionary */

/* empty words don't cause an error */

NAMED(_nopp, " ");
void nopp() {}

NAMED(_nop, " ");
void nop() {}

/* Forward declaration required here */
NAMED(_words, "words");
void words();

/* table of names and function addresses in flash */
const entry dictionary[] = {
    {_nopp, nopp},
    {_nop, nop},       {_words, words},   {_dup, dup},
    {_drop, drop},     {_back, back},     {_swap, swap},
    {_over, over},     {_add, add},       {_and, and_},
    {_or, or_},        {_xor, xor_},      {_invert, invert},
    {_negate, negate}, {_dotS, dotS},     {_dotShex, dotShex},
    {_dot, dot},       {_dotHEX, dotHEX}, {_delay, del},
    {_high, high},     {_low, low},       {_in, in},
    {_input, input},   {_output, output}, {_input_pullup, input_pullup},
    {_wiggle, wiggle}, {_dumpr, rdumps},  {_speed, speed}};

/* Number of words in the dictionary */
const int entries = sizeof dictionary / sizeof dictionary[0];

/* Display all words in dictionary */
void words() {
    for (int i = entries - 1; i >= 0; i--) {
        strcpy(namebuf, dictionary[i].name);
        serUSB_print(namebuf);
        serUSB_write(' ');
    }
}

/* Find a word in the dictionary, returning its position */
int locate() {
    for (int i = entries; i >= 0; i--) {
        strcpy(namebuf, dictionary[i].name);
        if (!strcmp(tib, namebuf)) {
            serUSB_print("  locate() returns: ");
            serUSB_print_int(i);
            serUSB_println("");
            serUSB_flush();
            return i;
        }
    }
    // serUSB_println("  locate() returns zero.");
    // serUSB_flush();
    return 0;
}

/* Is the word in tib a number? */
int isNumber() {
    char *endptr;
    strtol(tib, &endptr, 0);
    if (endptr == tib) return 0;
    if (*endptr != '\0') return 0;
    return 1;
}

int atoiLocal(char __xdata *str) {
	int i;
	int res = 0;

    for (i = 0; str[i] != 0; ++i)
        res = res * 10 + str[i] - '0';

    return res;
}

// extern int atoi (const char *nptr);

/* Convert number in tib */
int number() {
    char* ram;
    char* __xdata tibPtr = (char*) &tib;
    ram = (char *)tibPtr;
    int n = *ram;
    // const char *endptr; SEE_LINE();
    // char __xdata tibPtr = (char) &tib[0];

    // char* tibPtrPtr = (char*) tibPtr;
// int *ORG_ptr = &ORG; // okay for reading only
    // char *endptr; SEE_LINE();
    // endptr = &tib;
    // int nmbr = atoi(&tibPtr);
    // int nmbr =  (int) atoiLocal(&tibPtr);
    // int* nmbrPtr = (int *) atoiLocal(&tibPtr);
    // int* nmbrPtr = (int *) atoiLocal(*ram);
    // int nmbr = *nmbrPtr;
    // int nmbr = nmbrPtr;
    int nmbr=n;
    return nmbr;
    // return (int) strtol(tib, &endptr, 0);
}


#if 0
/* Is the word in tib a number? */
int isNumber() {
  char *endptr;
  strtol(tib, &endptr, 0);
  if (endptr == tib) return 0;
  if (*endptr != '\0') return 0;
  return 1;
}

/* Convert number in tib */
int number() {
  char *endptr;
  return (int) strtol(tib, &endptr, 0);
}
#endif

char ch;

void ok() {
    if (ch == '\r')
        return; /* NO RETURN it is this instead: Serial.println("ok"); */
}

#define BACKSPACE '\010'
#define RUBOUT '\177'

uint8_t ahua_flg = 0;
uint8_t two_ahua_flg = 0;

/* Incrementally read command line from serial port */
/* byte reading() { */
uint8_t reading() {
    if (!serUSB_available())
        return 1;

    ch = serUSB_read();
    // wrong: after a single backspace on an empty line

    if (ahua_flg) {
        int counted;
        counted = 6;

        for (int count = counted; count > 0; count--) {
            serUSB_write(' ');
            serUSB_flush();
            serUSB_write(BACKSPACE);
            serUSB_flush();
            serUSB_write(BACKSPACE);
            serUSB_flush();
        }
        serUSB_write(' ');
        serUSB_flush();
        ahua_flg = 0; // reset
    }

    if (ch == BACKSPACE) {
        serUSB_write(' '); // oblit - was already echo_d so cursor is in right
                           // spot for this
        serUSB_flush();
        serUSB_write(BACKSPACE); // move cursor
        serUSB_flush();
        if (pos > 0) {
            serUSB_write(BACKSPACE);
            serUSB_flush();
            serUSB_write(' ');
            serUSB_flush();
            serUSB_write(BACKSPACE);
            serUSB_flush();
            tib[--pos] = 0; // oblit captured char stored in tib
        }
        if (pos == 0) {
            serUSB_write(BACKSPACE);
            serUSB_flush();
            serUSB_write(' ');
            serUSB_flush();
            serUSB_write(BACKSPACE);
            serUSB_flush();
            serUSB_print(" ahua!");
            serUSB_flush();
            ahua_flg = 1; // or use a signed int
        }
        return 1;
    }

    serUSB_write(ch);
    serUSB_flush();

    if (ch == '\n')
        return 1;
    if (ch == '\r') {
        serUSB_write('\n'); // or no linefeed here
        serUSB_flush();
        return 0;
    }

    if (ch == ' ')
        return 0;

    if (pos < maxtib) {
        tib[pos++] = ch;
        tib[pos] = 0;
    }
    return 1;
}

/* Block on reading the command line from serial port */
/* then echo each word */
void readword() {
    serUSB_flush();
    pos = 0;
    tib[0] = 0;
    while (reading())
        ;

    if (0) {
        serUSB_println("");
        serUSB_flush();
        serUSB_println("doubleEcho testing for a conditional, LINE: 431");
        serUSB_println("");
        serUSB_flush();
    }

    uint8_t doubleEcho = 0; /* 0 normal - use -1  for the ACTIVE state */
    if (doubleEcho) {
        serUSB_flush();
        serUSB_print(" backspace_testing_tib: ");
        serUSB_flush();
        serUSB_print(tib);
        serUSB_flush();
        serUSB_print(" :end_tib ");
        serUSB_flush();
        serUSB_print(" ");
        serUSB_flush();
    }
}

/* Run a word via its name */
void runword() {
    serUSB_println("  - - - runword():  - - -");
    serUSB_flush();
    int place = locate();
    if (place != 0) {
        dictionary[place].function();
        ok();
        return;
    }
    if (isNumber()) {
        int nbr = number();

        // SEE_LINE();
        serUSB_print(" show what number() has for us: ");
        // serUSB_print_int(nbr);
        serUSB_print_hex_int(nbr);
        serUSB_print("  <-- nbr ");
        serUSB_println("");
        serUSB_flush();

        // push(number());
        push(nbr);
        ok();
        return;
    }
    /* serUSB_println("?"); */
}

void thing_bb() {

    int iterations = 2; // 32

    for (int i = iterations; i > 0; i--) {
        serUSB_flush();
        rdumps();
        serUSB_flush();
        ard_delay(60);
    }
}

void setupInterpreter() {
    ard_delay(500);
    // serUSB_println("");
    // serUSB_println("");
    // serUSB_println("");
    // serUSB_flush();
    serUSB_println("");
    // serUSB_println("  aye bee cee dee eee eff gee ach eye jay kay ell emm enn "
    //               "ohh pee que are ess tee you vee");
    // serUSB_println("");
    // serUSB_println("");
    serUSB_println("");
    serUSB_flush();
    ard_delay(500);

    char c = 'b';
    serUSB_write(c);
    c = ' ';
    serUSB_write(c);
    serUSB_println("Forth-like interpreter:");
    words();
    serUSB_println("");
    serUSB_flush();

    for (int offset = (0x11C0 + 0xB); offset > 0; offset--) {
        jaddr--; // re-align
        // ORG_ptr++; // re-align
        // ORG_ptr--;
    }

    /* ard_delay(30); */

    // uint8_t switchedOn = 0; /* -1  ACTIVE state */

    uint8_t switchedOn = 0; /* -1  ACTIVE state */

    if (switchedOn) {
        SEE_LINE();
        serUSB_println("   switchedOn active");
        serUSB_flush();

        for (int i = 2; i > 0; i--) {
            thing_bb();
        }
    }
}

void Interpreter() {
    serUSB_flush();
    readword();
    runword();
}

/**
 *
 * Sat 29 Jun 17:35:18 UTC 2024
 * Fri 28 Jun 16:46:02 UTC 2024
 * Thu 27 Jun 22:18:40 UTC 2024
 * Sun 23 Jun 11:30:54 UTC 2024
 *
 *
 */

/***
 *
 * Cannot yet do these things:
 *
 * strcpy(namebuf, dictionary[i].name);
 *
 * TODO: strcpy in the sdcc PDF
 */

/* end. */
