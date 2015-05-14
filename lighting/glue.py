DEBUG_SPI = False

import base64
import time
import threading

import rtmidi
import fake7221
import mq

spi = fake7221.Fake7221()

q = mq.MQ()


def setup7221(queue):
    setup = [
        (fake7221.DECODE, 0),
        (fake7221.INTENSITY, 0x0F),
        (fake7221.SCAN_LIMIT, 3),
        (fake7221.SHUTDOWN, 1),
    ]
    while True:
        for command in setup:
            queue.write(fake7221.nums2str(*command))
        time.sleep(1)


setup_thread = threading.Thread(target=setup7221, args=(q,))
setup_thread.daemon = True
setup_thread.start()


class LinearDisplay(object):
    BIT_TRANSLATOR = {
        x: x
        for x in range(8)
    }

    def __init__(self, queue):
        self._queue = queue
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
        self._queue.write(fake7221.nums2str(fake7221.DIG0 + digit, self._state[digit]))


display = LinearDisplay(q)

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
    data = q.read()
    if DEBUG_SPI:
        print base64.b16encode(data)
    spi.write(data)
