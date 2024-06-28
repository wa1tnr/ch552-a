#include <stdint.h>

#define LED_BUILTIN 33
const int ledPin = LED_BUILTIN; // the number of the LED pin

extern uint8_t serUSB_available();
extern char serUSB_read();

extern void serUSB_print_int(int i);
extern void serUSB_print_hex_int(int i);
extern void serUSB_write(char c);
extern void serUSB_print_hex(char c);
extern void serUSB_print(char *str);
extern void serUSB_println(char *str);
extern void serUSB_flush();
extern void ard_delay(int ms);

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
uint8_t stack[8];
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
    /* Serial.print(pop()); */
    /* Serial.print(" "); */
}

/* destructively display top of stack, hex */
NAMED(_dotHEX, ".h");
void dotHEX() {
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
void del() { /* delay(pop()); */ }

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
void high() { /* digitalWrite(pop(), HIGH); */ }

/* set TOS pin LOW */
NAMED(_low, "low");
void low() { /* digitalWrite(pop(), LOW); */ }

/* read TOS pin */
NAMED(_in, "in");
void in() { /* TOS = digitalRead(TOS); */ }

/* make TOS pin an input */
NAMED(_input, "input");
void input() { /* pinMode(pop(), INPUT); */ }

/* make TOS pin an output */
NAMED(_output, "output");
void output() { /* pinMode(pop(), OUTPUT); */ }

/* make TOS pin an input with weak pullup */
NAMED(_input_pullup, "input_pullup");
void input_pullup() { /* pinMode(pop(), INPUT_PULLUP); */ }

#define LIMIT_OUTER_ONE_EACH_ 0x1E00

__code int j = 0x7e;

int *jaddr = &j;

void printZeds(int pvr) {
    if (pvr > 0xFFF) {
        return;
    }
    serUSB_flush();
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
    serUSB_flush();
    serUSB_print("0");
    serUSB_flush();
}

/* dump 16 bytes of RAM in hex with ascii on the side */
void dumpRAM() {
    char buffer[5] = "";

    char *ram;
    ram = (char *)jaddr;
    serUSB_print("!");
    serUSB_flush();
    serUSB_print(" 0x");
    serUSB_flush();

    int pvr = (int)ram;

    // vuln bulk for (int count = 9; count > 0; count--) { pvr++; }

    printZeds(pvr);
    serUSB_print_hex_int(pvr);

    /***********************
     *                     *
     *    2 4 6 8 A C E    *
     *   1 3 5 7 9 B D F   *
     *                     *
     ***********************/

    serUSB_flush();
    serUSB_write(':');
    serUSB_write(' ');
    serUSB_flush();
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
    }
    ram = (char *)jaddr;
    serUSB_print("   ");
    for (uint8_t i = 0; i < 16; i++) {
        buffer[0] = *ram++;
        if (buffer[0] > 0x7f || buffer[0] < ' ')
            buffer[0] = '.';
        buffer[1] = '\0';
        serUSB_print(buffer);
    }
    for (int iter = 8; iter > 0; iter--) {
        jaddr++;
    }

    ard_delay(23);

    /* old int tested = (int)jaddr; */

    /* trial 2210z */
    int tested = (int)ram;

    if (tested > 0x3480) {
        serUSB_println("   may be a DANGER  ");
        serUSB_flush();
        serUSB_println("");
        serUSB_flush();
        serUSB_println("   see: interpreter.c  LINE 295");
        serUSB_flush();
        serUSB_println("");
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
}

/* End of Forth interpreter words */
/* ******************************************** */
/* Beginning of application words */

/* End of application words */
/* ******************************************** */
/* Now build the dictionary */

/* empty words don't cause an error */
NAMED(_nop, " ");
void nop() {}

/* Forward declaration required here */
NAMED(_words, "words");
void words();

/* table of names and function addresses in flash */
const entry dictionary[] = {
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
        /* strcpy(namebuf, dictionary[i].name); */
        /* Serial.print(namebuf); */
        /* Serial.print(" "); */
    }
}

/* Find a word in the dictionary, returning its position */
int locate() {
    for (int i = entries; i >= 0; i--) {
        /* strcpy(namebuf, dictionary[i].name); */
        /* if (!strcmp(tib, namebuf)) return i; */
    }
    return 0;
}

/* Is the word in tib a number? */
int isNumber() {
    /* char *endptr; */
    /* strtol(tib, &endptr, 0); */
    /* if (endptr == tib) return 0; */
    /* if (*endptr != '\0') return 0; */
    return 1;
}

/* Convert number in tib */
int number() {
    /* char *endptr; */
    /* return (int) strtol(tib, &endptr, 0); */
}

char ch;

void ok() {
    if (ch == '\r')
        return; /* NO RETURN it is this instead: Serial.println("ok"); */
}

/* Incrementally read command line from serial port */
/* byte reading() { */
uint8_t reading() {
    /* if (!Serial.available()) return 1; */
    if (!serUSB_available()) return 1;
    ch = serUSB_read();
    ch = 'a'; /* no constant it is this instead: Serial.read(); */
    if (ch == '\n')
        return 1;
    if (ch == '\r')
        return 0;
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
    pos = 0;
    tib[0] = 0;
    while (reading())
        ;
    serUSB_print(tib);
    serUSB_print(" ");
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
        push(number());
        ok();
        return;
    }
    /* serUSB_println("?"); */
}

void setupInterpreter() {
    ard_delay(3000);
    serUSB_println("");
    serUSB_println("");
    serUSB_println("");
    serUSB_println("seen: setupInterpreter();");
    serUSB_println("");
    serUSB_println("  aye bee cee dee eee eff gee ach eye jay kay ell emm enn "
                   "ohh pee que are ess tee you vee");
    serUSB_println("");
    serUSB_println("");
    serUSB_println("");
    serUSB_flush();
    ard_delay(1000);

    char c = 'b';
    serUSB_write(c);
    c = ' ';
    serUSB_write(c);
    serUSB_println("Forth-like interpreter:");
    words();
    serUSB_println("");
    serUSB_flush();

    for (int offset = (0x11C0 + 0xB); offset > 0; offset--) {
        jaddr--;  // re-align
    }


    for (int offset = (0x10F + 0x4); offset > 0; offset--) {
        jaddr--;  // re-align
    }


    // ard_delay(30000);

    for (int i = 48; i > 0; i--) {
        serUSB_flush();
        rdumps();
        serUSB_flush();
        ard_delay(60);
    }
}

void Interpreter() {
    readword();
    runword();
}

/*
 * Fri 28 Jun 16:46:02 UTC 2024
 * Thu 27 Jun 22:18:40 UTC 2024
 * Sun 23 Jun 11:30:54 UTC 2024
 */

/* end. */
