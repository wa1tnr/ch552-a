/* interpreter.c - Shattuck's Forth-like interpreter - port to CH552 8051 MCU */
/* June, 2024 */
#include <Arduino.h>

const char signature[] = "   This is what a const char array is like and where "
                         "it is stored fedcba9876543210 back quiet   ";

#define LED_BUILTIN 33
const int ledPin = LED_BUILTIN; // the number of the LED pin

extern void ard_delay(int ms);

#define SEE_LINE()                                                             \
    USBSerial_print("   see: interpreter.c  LINE ");                           \
    USBSerial_print(__LINE__);                                                 \
    USBSerial_println("")

uint8_t baseRadix = 16;
uint8_t slowerThan = 0;

__code int j = 0x7e;

// cannot do anything here - likely is boot area:
__code __at(0x0) char  ORG;

int *ORG_ptr        = &ORG; // okay for reading only
int *ORG_ptrBu      = &ORG;
int *ORG_ptrBuBu    = &ORG;
int *jaddr = &j;

// xdata is abt 1 kb
// data is abt 128 bytes perhaps - check datasheet and reexamine dump
__data __at(0x0) char k;
__data __at(0x0) char ORGXdata;
int *ORG_XPtr       = &ORGXdata; // unknown okay
int *ORG_XPtrBu     = &ORGXdata;
int *ORG_XPtrBuBu   = &ORGXdata;

int *ORG_kaddrPtr      = &k;
int *ORG_kaddrPtrBu    = &k;
int *ORG_kaddrPtrBuBu  = &k;

int *kaddr = &k;

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

NAMED(_clear, "clr");
void clrstack() {
    for (uint8_t count = 8; count > 0; count--) {
        push(0);
    }
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

/* multiply top two items */
NAMED(_multply, "*");
void multply() {
    int a = pop();
    TOS = a * TOS;
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

// void baser(uint8_t radix) { baseRadix = radix; }
NAMED(_base, "base");
void base() { baseRadix = (uint8_t)pop(); }

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
void del() {delay(pop()); }

/* Toggle pin at TOS and delay(spd), repeat... */
NAMED(_wiggle, "wiggle");
void wiggle() {
    uint8_t a = pop();
    pinMode(a, OUTPUT);
    for (uint8_t i = 0; i < 20; i++) {
        digitalWrite(a, HIGH);
        delay(100);
        digitalWrite(a, LOW);
        delay(100);
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
    if (pvr > 0xFF) {
        return;
    }
    if (pvr < 0x100) {
        USBSerial_print("0");
        if (pvr > 0x0F) {
            return;
        }
    }
    USBSerial_print("0");
}

void spaces(int qty) {
    for (int quantity = qty; quantity > 0; quantity--) {
        USBSerial_write(' ');
    }
}

void throw(int error) {
    SEE_LINE();
    USBSerial_print("   * * *     ERROR ");
    USBSerial_print(error);
    USBSerial_print("     * * *");
    spaces(7);
    USBSerial_print(" PRESS  ~ RESET ~");
    USBSerial_flush();
    while (-1)
        ;
}

/* dump 16 bytes of RAM in hex with ascii on the side */
void dumpRAM() {
    char buffer[5] = "";
    char *ram;

    const uint8_t xdataP = 0; // 0 code; 1 for data iram or xdata xram

    // ram = 0;

    if (!xdataP) {
        ram = (char *)ORG_ptr;
    }

    if (xdataP) {
        // ram = (char *)ORG_XPtr;
        ram = (char *)ORG_kaddrPtr;
    }

    USBSerial_print("!");
    USBSerial_print(" 0x");
    int pvr = (int)ram;

    if (pvr > 0x3FF0) {
        int error = -13;
        throw(error);
    }

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
    }

    if (!xdataP) {
        ram = (char *)ORG_ptr;
    }

    if (xdataP) {
        // ram = (char *)ORG_XPtr;
        ram = (char *)ORG_kaddrPtr;
    }

    USBSerial_print("   ");
    for (uint8_t i = 0; i < 16; i++) {
        buffer[0] = *ram++;
        if (buffer[0] > 0x7f || buffer[0] < ' ')
            buffer[0] = '.';
        buffer[1] = '\0';
        USBSerial_print(buffer);
    }

    for (int iter = 8; iter > 0; iter--) {
        ORG_XPtr++;
        ORG_ptr++;
        ORG_kaddrPtr++;
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

void dumping() {
    for (uint8_t i = 0; i < 16; i++) {
        USBSerial_println("");
        dumpRAM();
    }
    USBSerial_println("");
    USBSerial_flush();
}

void resetOrgPtr() {
    ORG_ptrBu      = ORG_ptrBuBu;
    ORG_XPtrBu     = ORG_XPtrBuBu;
    ORG_kaddrPtrBu = ORG_kaddrPtrBuBu;
}

/* dump 256 bytes of RAM */
NAMED(_dumpr, "dump");
void rdumps() {
    int ORGOffset = pop();
    drop();
    back();
    push(0);

    USBSerial_println("");

    dotShex();

    USBSerial_println("");
    USBSerial_flush();

    uint8_t isOffset = (ORGOffset > 0);

    if (isOffset) {
        resetOrgPtr();
        ORG_ptr   = ORG_ptrBu;
        ORG_ptr   = ORG_ptr + (ORGOffset / 2);
        ORG_ptrBu = ORG_ptrBu + 128;

        ORG_XPtr  = ORG_XPtrBu;
        ORG_XPtr  = ORG_XPtr + (ORGOffset / 2); // assume same case as above
        ORG_XPtrBu = ORG_XPtrBu + 128;

        ORG_kaddrPtr = ORG_kaddrPtrBu;
        ORG_kaddrPtr = ORG_kaddrPtr + (ORGOffset / 2);
        ORG_kaddrPtrBu = ORG_kaddrPtrBu + 128;
    }

    ORG_ptr++;
    ORG_ptr--; // align

    ORG_XPtr++;
    ORG_XPtr--;

    ORG_kaddrPtr++;
    ORG_kaddrPtr--;


    /*
     *            pointer alignment in flashROM memory
     *
     * The alignment consequences are:
     *
     *     '1  dump'  dumps from  0x0000   .. but
     *     '2  dump'  dumps from  0x0002   .. and that
     */
    dumping();
}

/* End of Forth interpreter words */
/* ******************************************** */
/* Beginning of application words */

/* End of application words */
/* ******************************************** */
/* Now build the dictionary */

/* empty words don't cause an error */

NAMED(_nopp, "nopp");
void nopp() {}

NAMED(_nop, "nop");
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
    {_clear, clrstack},
    {_over, over},     {_add, add},       {_multply, multply},
    {_and, and_},
    {_or, or_},        {_xor, xor_},      {_invert, invert},
    {_negate, negate}, {_dotS, dotS},     {_dotShex, dotShex},
    {_base, base},
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
    strtol(tib, &endptr, baseRadix); // use zero for automatic or what have you
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

/***
 *
 * colonel Kurtz ;)
 *
 */

const char hexDigits[] = "0123456789abcdef";

int atoiLocal(char __xdata *str) {
    int iter;
    int res = 0;
    uint8_t value = 0;

#if 0
    for (uint8_t j = 0; hexDigits[j] != 0; j++) {
        char ptr = hexDigits[j];
        uint8_t dec = xlate_hex_to_dec(ptr);
        if (dec != noMatch) {
        }
    }
#endif

    for (iter = 0; str[iter] != 0; ++iter) {
        int interim;
        interim = str[iter] - '0' - ((baseRadix == 16) * 7);

        if (interim < 10) {
            res = res * baseRadix + str[iter] - '0';
        }

        if (interim > 9) {
            res = res * baseRadix + str[iter] - '0' - ((baseRadix == 16) * 7);
        }
    }
    return res;
}

/* Convert number in tib */
int number() {
    char *ram;
    char *__xdata tibPtr = (char *)&tib;
    ram = (char *)tibPtr;
    int n = *ram;
    int nmbr = atoiLocal((char __xdata *)tib);
    return nmbr;
}

char ch;

void ok() {
    if (ch == '\r')
        USBSerial_print(" ok ");
}

#define BACKSPACE '\010'
#define RUBOUT '\177'

uint8_t ahua_flg = 0;
uint8_t two_ahua_flg = 0;

/* Incrementally read command line from serial port */
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
    const uint8_t switchedOn = 0;
    if (isNumber()) {
        int nbr = number();

        if (switchedOn) {
            USBSerial_print(" hex: ");
            USBSerial_print(nbr, HEX);
            USBSerial_print("  <-- number, HEX");
            USBSerial_println("");
            USBSerial_flush();
        }

        push(nbr);
        ok();
        return;
    }
    /* USBSerial_println("?"); */
}

void setupInterpreter() {
    delay(800);
    delay(800);
    delay(800);
    delay(800);
    USBSerial_println("");
    USBSerial_flush();
    USBSerial_println(signature);
    USBSerial_println("");
    USBSerial_flush();
    USBSerial_println(" kansas nb Forth-like interpreter:");
    USBSerial_flush();
    words();
    USBSerial_println("");
    USBSerial_flush();

    for (int offset = (0x11C0 + 0xB); offset > 0; offset--) {
        jaddr--; // re-align
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

// quite good.

/* end. */
