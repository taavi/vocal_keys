#!/usr/bin/env python

DEBUG_SPI = False
DEBUG_7221 = False

import array
import base64
import fcntl
import sys
import time
import threading

import rtmidi
import fake7221
import mq

SPI_IOC_WR_MODE = 0x40016b01
SPI_IOC_RD_MODE = 0x80016b01
SPI_IOC_WR_BITS_PER_WORD = 0x40016b03
SPI_IOC_RD_BITS_PER_WORD = 0x80016b03
SPI_IOC_WR_MAX_SPEED_HZ = 0x40046b04
SPI_IOC_RD_MAX_SPEED_HZ = 0x80046b04


USE_FAKE_SPI = False
if USE_FAKE_SPI:
    spi = fake7221.Fake7221()
else:
    spi = open('/dev/spidev0.0', 'w')
    for opt, arg in [
        (SPI_IOC_WR_MODE,          array.array('b', [0])),
        (SPI_IOC_RD_MODE,          array.array('b', [0])),
        (SPI_IOC_WR_BITS_PER_WORD, array.array('b', [8])),
        (SPI_IOC_RD_BITS_PER_WORD, array.array('b', [8])),
        (SPI_IOC_WR_MAX_SPEED_HZ,  array.array('l', [200000])),
        (SPI_IOC_RD_MAX_SPEED_HZ,  array.array('l', [200000])),
    ]:
        ret = fcntl.ioctl(spi.fileno(), opt, arg)
        if ret == -1:
            print "Failed to set opt", opt, "to", arg
            time.sleep(1)
            sys.exit(1)

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
        0: 6,
        1: 5,
        2: 4,
        3: 3,
        4: 2,
        5: 1,
        6: 0,
        7: 7,
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
    if DEBUG_7221:
        print spi


midi = rtmidi.MidiIn()
best_port = None
while best_port is None:
    ports = enumerate(midi.get_ports())
    for i, port_name in ports:
        if "Midi Through" not in port_name:
            best_port = i
            print "Found", i, port_name
            break
    else:
        print "No MIDI device found yet, only saw", ports
        time.sleep(1)

midi.open_port(best_port)
midi.set_callback(callback)


while True:
    data = q.read()
    if DEBUG_SPI:
        print base64.b16encode(data)
    spi.write(data)
    spi.flush()
    if DEBUG_7221:
        print spi
