import math
from collections import namedtuple

Note = namedtuple('Note', 'name frequency'.split())

SEMI = 2 ** (1/12.)
A440 = 440

note_names = [n + o for o in '234' for n in 'A Bb B C Db D Eb E F Gb G Ab'.split()][3:-3]
notes = [Note(name, A440 * (SEMI ** n)) for (name, n) in zip(note_names, range(-21, 4))]
print '\n'.join(map(str, notes))

SAMPLE_RATE = 10000.
WAVE_RESOLUTION = 256

print '\n'.join(
    "#define NOTE_{} {}".format(note.name, int(note.frequency*WAVE_RESOLUTION/SAMPLE_RATE*256))
    for note in notes
)


sine = ["%3d" % (min(254, int((math.sin(x * math.pi / 64) + 1)*128)) + 1) for x in range(128)]
def group(x, n):
    return [x[i:i+n] for i in range (0, len(x), n)]

sinelines = group(sine, 16)
strlines = map(', '.join, sinelines)

print "const unsigned char sine[] = {"
print "    " + ",\n    ".join(strlines) + "\n"
print "};"
