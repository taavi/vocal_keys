Vocal Keys
==========

Quick Start
-----------

1. Plug power wires into the Raspberry Pi ([see diagram](RasPiWiring.svg)).
1. Plug visual driver wires into the Raspberry Pi
1. Plug 2x3 connector onto visual driver
1. Connect the 7 light cables into J1-J7
1. Connect the blue clock driver cable with 8P8C connector into the middle sound driver
1. Use the two 8P8C patch cables to connect the left and right sound drivers to the pink sockets
1. Connect the USB MIDI keyboard.
1. Plug the power in!


System Diagram
--------------

There's a picture showing [how it all fits together](SystemDiagram.svg).


Visual system
-------------

A fancy way to turn a MIDI controller into 25 light switches.

The MIDI controller connects to a Raspberry Pi running [glue
code](lighting/glue.py) to control an LED driver to power the LEDs.

From the RasPi to the lights, it's this: https://www.raspberrypi.org/forums/viewtopic.php?t=41713

GPIO header image from http://elinux.org/images/2/2a/GPIOs.png
RasPi to MAX7221 image from https://www.raspberrypi.org/forums/download/file.php?id=3187

Once the RasPi finishes booting (about 30s, some lights should blink), it
should start the glue script (via `supervisord`) and start driving lights.
Keep the MIDI keyboard octave at 0.


PCBs:

* https://upverter.com/taavi.burns/5a45d18ccff58b8d/Vocal-Keys-Visual-Driver/


Audio driver
------------

The clock source (on the perfboard) has a silver knob attached which adjusts
the pitch. There's a delay between turning the knob and the pitch changing, so
don't be alarmed if the pitch jumps when you frob it quickly.

PCBs:

* Clock source: https://upverter.com/taavi.burns/643a069b6ed90288/Vocal-Keys-Clock-Source/
* 2-channel module: https://upverter.com/taavi.burns/83e2016f035f6c28/Vocal-Keys-Sound-Driver-2-channel/
* 10-channel full board: https://upverter.com/taavi.burns/dba4ff64ca8faec9/Vocal-Keys-Driver-10-channel/


Programming the ATtiny45s
-------------------------

The ATtinys come with no bootloader, so you have to program them with a
programmer. Thankfully almost any Arduino will do! I used my Arduino Leonardo.

* http://www.arduino.cc/en/Tutorial/ArduinoISP
* https://petervanhoyweghen.wordpress.com/2012/09/16/arduinoisp-on-the-leonardo/
* http://kevincuzner.com/2013/05/27/raspberry-pi-as-an-avr-programmer/

(I think these are the best links, offline just now)
