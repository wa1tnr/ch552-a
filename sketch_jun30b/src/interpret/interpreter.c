/* interpreter.c - Shattuck's Forth-like interpreter - port to CH552 8051 MCU */
/* June, 2024 */
#include <Arduino.h>
#include <sdcc-lib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/***
 *
 *
 */

#define LED_BUILTIN 33
const int ledPin = LED_BUILTIN; // the number of the LED pin

#if 0
extern uint8_t USBSerial_available();
extern char USBSerial_read();
extern void USBSerial_print_int(int i);
extern void USBSerial_print_hex_int(int i);
extern void USBSerial_write(char c);
extern void USBSerial_print_hex(char c);
extern void USBSerial_print(char *str);
extern void USBSerial_println(char *str);
extern void USBSerial_flush();
#endif
extern void ard_delay(int ms);

#define LIMIT_OUTER_ONE_EACH_ 0x1E00

#define SEE_LINE()                                                             \
    USBSerial_print("   see: interpreter.c  LINE ");                              \
    USBSerial_print(__LINE__);                                                \
    USBSerial_println("")


uint8_t baseRadix = 10;
uint8_t slowerThan = 0;

__code int j = 0x7e;

// cannot do anything here - likely is boot area:
__code __at(0x0) char ORG;

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
    TOS = n;
}

/* return top of stack */
int pop() {
    int n = TOS;
    p = (p - 1) & STKMASK;
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
    int a;
    int b;
    a = pop();
    b = pop();
    push(a);
    push(b);
}

/* copy second on stack to top */
NAMED(_over, "over");
void over() {
    int a;
    int b;
    a = pop();
    b = pop();
    push(b);
    push(a);
    push(b);
}
/* add top two items */
NAMED(_add, "+");
void add() {
    int a = pop();
    TOS = a + TOS;
}

/* bitwise and top two items */
NAMED(_and, "and");
void and_() {
    int a = pop();
    TOS = a & TOS;
}

/* inclusive or top two items */
NAMED(_or, "or");
void or_() {
    int a = pop();
    TOS = a | TOS;
}

/* exclusive or top two items */
NAMED(_xor, "xor");
void xor_() {
    int a = pop();
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
    USBSerial_print(pop());
    USBSerial_write(' ');
    USBSerial_flush();
}

/* destructively display top of stack, hex */
NAMED(_dotHEX, ".h");
void dotHEX() {
    USBSerial_print(0xffff & pop(), HEX);
    USBSerial_write(' ');
    USBSerial_flush();
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
void del() { ard_delay(pop()); }

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
    USBSerial_print("0"); // pad print job with leading zero
    USBSerial_flush();
    if (pvr > 0xFF) {
        return;
    }
    if (pvr < 0x100) {
        USBSerial_print("0");
        USBSerial_flush();
        if (pvr > 0x0F) {
            return;
        }
    }
    USBSerial_print("0");
    USBSerial_flush();
}

/* dump 16 bytes of RAM in hex with ascii on the side */
void dumpRAM() {
    char buffer[5] = "";
    char *ram;
    ram = (char *)ORG_ptr;
    USBSerial_print("!");
    USBSerial_print(" 0x");
    int pvr = (int)ram;
    printZeds(pvr);
    USBSerial_print(pvr, HEX);

    /***********************
     *                     *
     *    2 4 6 8 A C E    *
     *   1 3 5 7 9 B D F   *
     *                     *
     ***********************/

    USBSerial_write(':');
    USBSerial_write(' ');
    USBSerial_print("   ");

    /* individual hex 2-digit groups l to r */
    for (uint8_t i = 0; i < 16; i++) {
        char c = *ram++;
        c = c & 0xff;
        if (c < 0x10) {
            USBSerial_print("0"); // padding
        }
        USBSerial_print(c, HEX);
        USBSerial_write(' ');
        USBSerial_flush();
    }
    ram = (char *)ORG_ptr;
    USBSerial_print("   ");
    for (uint8_t i = 0; i < 16; i++) {
        buffer[0] = *ram++;
        if (buffer[0] > 0x7f || buffer[0] < ' ')
            buffer[0] = '.';
        buffer[1] = '\0';
        USBSerial_print(buffer);
        USBSerial_flush();
    }
    for (int iter = 8; iter > 0; iter--) {
        ORG_ptr++;
    }

    if (slowerThan) {
        ard_delay(23);
    }

    int tested = (int)ram;

    if (tested > 0x4000) {
        USBSerial_println("");
        USBSerial_print("    __code address 0x");
        USBSerial_print(tested, HEX);
        USBSerial_println(" may be a DANGER  ");
        SEE_LINE();
        USBSerial_flush();
        while (-1) {
            ard_delay(2000);
            USBSerial_write('.');
            USBSerial_flush();
        }
    }
}

/* dump 256 bytes of RAM */
NAMED(_dumpr, "dump");
void rdumps() {
    for (uint8_t i = 0; i < 16; i++) {
        USBSerial_println("");
        dumpRAM();
    }
    USBSerial_println("");
    USBSerial_flush();
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

// clang-format off
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
// clang-format on

/* Number of words in the dictionary */
const int entries = sizeof dictionary / sizeof dictionary[0];

/* Display all words in dictionary */
void words() {
    for (int i = entries - 1; i >= 0; i--) {
        strcpy(namebuf, dictionary[i].name);
        USBSerial_print(namebuf);
        USBSerial_write(' ');
    }
}

/* Find a word in the dictionary, returning its position */
int locate() {
    for (int i = entries; i >= 0; i--) {
        strcpy(namebuf, dictionary[i].name);
        if (!strcmp(tib, namebuf)) {
            return i;
        }
    }
    return 0;
}

/* Is the word in tib a number? */
int isNumber() {
    char *endptr;
    strtol(tib, &endptr, 0);
    if (endptr == tib)
        return 0;
    if (*endptr != '\0')
        return 0;
    return 1;
}








const uint8_t noMatch = 77;

uint8_t xlate_hex_to_dec(char ptr) {
    if (ptr == 'a') {
        // USBSerial_write('Q');
        uint8_t value = 10;
        return value;
    }
    return noMatch; // what else might it return?
}


const char hexDigits[] = "0123456789abcdef";

int atoiLocal(char __xdata *str) {
    SEE_LINE();
    int i;
    int res = 0;
    int resh = 0;
    uint8_t value = 0;

    const char* p = (const char *) hexDigits;

    USBSerial_println(" table hexDigits array: ");
    USBSerial_print("     ");
    USBSerial_println_s(p); // prints entire 'table' (const char hexDigits array)
    USBSerial_println(" <- table");
    USBSerial_flush();

    for (uint8_t j = 0; hexDigits[j] != 0; j++) {
        USBSerial_print("  n: ");
        USBSerial_print(j); // but why - it is an index into an arbitrary character set
        USBSerial_print(" >");
        USBSerial_flush();
        char ptr = hexDigits[j];
        uint8_t dec = xlate_hex_to_dec(ptr);

        USBSerial_print(ptr);
        USBSerial_write('<');


        if (dec != noMatch) {
            USBSerial_println("");
            USBSerial_println("    ! noMatch  .. it is 'a' atm.. ");
            USBSerial_print("     ");
            USBSerial_flush();
        }



        USBSerial_flush();
    }

    // hexDigits[j] holds a character

        // str[i] gives left to right  -  code lifted outside source

        // str must be <thing> then

        //  1BC = 444

        //  256 + 176 + 12

        //     1     B     C
        //  0001  1011  1100
        //  0001  0000  0000    256
        //  0000  1011  0000    176
        //  0000  0000  1100     12

    for (i = 0; str[i] != 0; ++i)
        resh = resh * 16 + str[i] - '0'; // no of course  

        res = res * 10 + str[i] - '0';
    return res;
}

/* Convert number in tib */
int number() {
    char *ram;
    char *__xdata tibPtr = (char *)&tib;
    ram = (char *)tibPtr;
    int n = *ram;
    int nmbr = atoiLocal((char __xdata *)tib);
    // USBSerial_println("DARMOK and Gilad at Tenagra!");
    // USBSerial_flush();
    return nmbr;
}

char ch;

void ok() {
    if (ch == '\r')
        return;
}

#define BACKSPACE '\010'
#define RUBOUT '\177'

uint8_t ahua_flg = 0;
uint8_t two_ahua_flg = 0;

/* Incrementally read command line from serial port */
/* byte reading() { */
uint8_t reading() {
    if (!USBSerial_available())
        return 1;

    ch = USBSerial_read();
    // wrong: after a single backspace on an empty line

    if (ahua_flg) {
        int counted;
        counted = 6;

        for (int count = counted; count > 0; count--) {
            USBSerial_write(' ');
            USBSerial_flush();
            USBSerial_write(BACKSPACE);
            USBSerial_flush();
            USBSerial_write(BACKSPACE);
            USBSerial_flush();
        }
        USBSerial_write(' ');
        USBSerial_flush();
        ahua_flg = 0; // reset
    }

    if (ch == BACKSPACE) {
        USBSerial_write(' ');
        USBSerial_flush();
        USBSerial_write(BACKSPACE);
        USBSerial_flush();
        if (pos > 0) {
            USBSerial_write(BACKSPACE);
            USBSerial_flush();
            USBSerial_write(' ');
            USBSerial_flush();
            USBSerial_write(BACKSPACE);
            USBSerial_flush();
            tib[--pos] = 0; // obliterate
        }
        if (pos == 0) {
            USBSerial_write(BACKSPACE);
            USBSerial_flush();
            USBSerial_write(' ');
            USBSerial_flush();
            USBSerial_write(BACKSPACE);
            USBSerial_flush();
            USBSerial_print(" ahua!");
            USBSerial_flush();
            ahua_flg = 1;
        }
        return 1;
    }

    USBSerial_write(ch);
    USBSerial_flush();

    if (ch == '\n')
        return 1;
    if (ch == '\r') {
        USBSerial_write('\n');
        USBSerial_flush();
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
    USBSerial_flush();
    pos = 0;
    tib[0] = 0;
    while (reading())
        ;
}

/* Run a word via its name */
void runword() {
    int place = locate();
    if (place != 0) {
        dictionary[place].function();
        ok();
        return;
    }
    if (isNumber()) {
        int nbr = number();

        USBSerial_print(" hex: ");
        USBSerial_print(nbr, HEX);
        USBSerial_print("  <-- number, HEX");
        USBSerial_println("");
        USBSerial_flush();

        push(nbr);
        ok();
        return;
    }
    /* USBSerial_println("?"); */
}

void thing_bb() {

    int iterations = 2; // 32

    for (int i = iterations; i > 0; i--) {
        USBSerial_flush();
        rdumps();
        USBSerial_flush();
        ard_delay(60);
    }
}


void base(uint8_t radix) {
    baseRadix = radix;
}

void setupInterpreter() {
    SEE_LINE();
    USBSerial_println("I am inside setupInterpreter()");
    // conclusion: no need for USBSerial_foo() wrappers
    // however, must have this in a C (not C++ here)
    // source file (such as the present interpeter.c
    // file):
    // #include <Arduino.h>
    USBSerial_flush();
    ard_delay(500);
    USBSerial_println("");
    USBSerial_flush();
    ard_delay(500);

    char c = ' ';
    USBSerial_write(c);
    c = ' ';
    USBSerial_write(c);
    USBSerial_println(" jasper wy Forth-like interpreter:");
    words();
    USBSerial_println("");
    USBSerial_flush();

    for (int offset = (0x11C0 + 0xB); offset > 0; offset--) {
        jaddr--; // re-align
    }

    uint8_t switchedOn = 0; /* -1  ACTIVE state */

    if (switchedOn) {
        SEE_LINE();
        USBSerial_println("   switchedOn active");
        USBSerial_flush();

        for (int i = 2; i > 0; i--) {
            thing_bb();
        }
    }
}

void Interpreter() {
    USBSerial_flush();
    readword();
    runword();
}

/**
 *
 * Sun 30 Jun 03:45:04 UTC 2024
 * Sat 29 Jun 17:35:18 UTC 2024
 * Fri 28 Jun 16:46:02 UTC 2024
 * Thu 27 Jun 22:18:40 UTC 2024
 * Sun 23 Jun 11:30:54 UTC 2024
 *
 *
 */

/***
 *
 *
 *
 */

/* end. */

// uint8_t baseRadix = 10;