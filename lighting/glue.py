import time

import rtmidi
import fake7221

spi = fake7221.Fake7221()

class LinearDisplay(object):
    BIT_TRANSLATOR = {
        x: x
        for x in range(8)
    }

    def __init__(self, spi_7221):
        self._spi_7221 = spi_7221
        self._state = [0] * 4

    def __setitem__(self, key, do_set):
        if key >= (len(self._state) * 8):
            return
        digit, bit = divmod(key, 8)
        mask = 1 << self.BIT_TRANSLATOR[bit]
        if do_set:
            self._state[digit] |= mask
        else:
            self._state[digit] &= ~(mask)
        self._spi_7221.write(fake7221.nums2str(fake7221.DIG0 + digit, self._state[digit]))


display = LinearDisplay(spi)

KEYDOWN = 144
KEYUP = 128
MIDI_C2 = 48
MIDI_C4 = 72

def callback(event, data):
    ((event_type, note, velocity), time_delta) = event
    display[note - MIDI_C2] = True if event_type == KEYDOWN else False
    print spi


midi = rtmidi.MidiIn()
midi.open_port(name="Some Name")
midi.set_callback(callback)

while True:
    time.sleep(1)
    spi.write(fake7221.nums2str(fake7221.SHUTDOWN, 1))
