# CH552-a

Adafruit's new board.

Arduino IDE (sort-of).

Primarily for the C programming Language.

#### Sun 23 Jun 13:21:55 UTC 2024

 * imported shattuck's forth like interpreter code
   initially with a git submodule

 * modded it severely for clean compile non-functioning

 * omitted both submodule dependencies but kept the
   compiler toolchain submodule available here

   REPEAT: no `'git submodule update --init'` used at all,
   except *electively* for detailed *informational*
   purposes.  Nothing there is compiled-against.

#### Sun 23 Jun 09:48:49 UTC 2024
##### update: success

The board receives firmware and executes it.

Do use Arduino IDE 2.x to do the firmware upload - arduino-cli 
did not 'work' and is unknown (in this one git repository) to
'work' completely (it will kick out messages that indicate a
conversation has taken place but that's not enough).

Or .. something was overlooked.

 * blink LED on GPIO
 * print to the USB serial, messages in English
 * operate the NeoPixel

Each of those hit on success yesterday.  Did not focus on making
gains good (saving code).

#### Sat 22 Jun 15:52:23 UTC 2024

Shipment arrived Saturday.

#### Fri Jun 21 02:02:43 PM UTC 2024

initialize repository, create minimal application, compile (no testing).

#### end.
