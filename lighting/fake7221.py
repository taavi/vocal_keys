NOP = 0x0
DIG0 = 0x1
DIG1 = 0x2
DIG2 = 0x3
DIG3 = 0x4
DIG4 = 0x5
DIG5 = 0x6
DIG6 = 0x7
DIG7 = 0x8
DECODE = 0x9
INTENSITY = 0xA
SCAN_LIMIT = 0xB
SHUTDOWN = 0xC
TEST = 0xF

from collections import namedtuple


STATE = namedtuple(
    "STATE",
    "nop dig0 dig1 dig2 dig3 dig4 dig5 dig6 dig7 decode intensity scan_limit shutdown xd xe test".split())


def nums2str(*l):
    return ''.join(map(chr, l))


class Fake7221(object):
    """
    >>> ic = Fake7221()
    >>> print ic
    SHUTDOWN
    >>> ic.write(nums2str(SHUTDOWN, 1))
    2
    >>> print ic
    dABCDEFG (legend)
    00000000
    Brightness 1/16
    >>> ic.write(nums2str(DIG0, 0xA5))
    2
    >>> ic.write(nums2str(DECODE, 0xF0))
    2
    >>> ic.write(nums2str(SCAN_LIMIT, 5))
    2
    >>> print ic
    dABCDEFG (legend)
    10100101
    00000000
    00000000
    00000000
    0
    0
    Brightness 1/16
    """

    def __init__(self):
        self._state = STATE(*([0] * 16))

    def close(self):
        pass

    def write(self, data):
        if len(data) < 2:
            return len(data)
        register = ord(data[-2]) & 0x0F
        value = ord(data[-1])
        update = {STATE._fields[register]: value}
        self._state = self._state._replace(**update)
        return len(data)

    def _decode_digit(self, n):
        """
        >>> ic = Fake7221()
        >>> ic._state = STATE(0, 0, 1, 2, 3, 4, 5, 6, 7, 0xA4, 5, 5, 0, 0, 0, 0)
        >>> ic._decode_digit(0)
        '00000000'
        >>> ic._decode_digit(1)
        '00000001'
        >>> ic._decode_digit(2)
        '2'
        >>> ic._decode_digit(3)
        '00000011'
        >>> ic._decode_digit(4)
        '00000100'
        >>> ic._decode_digit(5)
        '5'
        >>> ic._decode_digit(6)
        '00000110'
        >>> ic._decode_digit(7)
        '7'
        """
        digit = self._state[n + 1]
        if self._state.decode & (1 << n):
            dp = digit & 0x80
            return "0123456789-EHLP,"[digit & 0x0F] + ("." if dp else "")
        else:
            return bin(digit)[2:].zfill(8)

    @property
    def intensity(self):
        return "%d/16" % min(15, ((self._state.intensity & 0x0F) + 1))

    @property
    def is_shutdown(self):
        return not (self._state.shutdown & 1)

    @property
    def scan_limit(self):
        return (self._state.scan_limit & 0x07) + 1

    def __str__(self):
        if self.is_shutdown:
            return "SHUTDOWN"
        out = ["dABCDEFG (legend)"]
        out.extend(self._decode_digit(n) for n in range(self.scan_limit))
        out.append("Brightness " + self.intensity)
        return '\n'.join(out)

