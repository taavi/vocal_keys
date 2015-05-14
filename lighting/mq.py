from collections import deque
from threading import Semaphore

class MQ(object):
    def __init__(self):
        self._semaphore = Semaphore(0)
        self._q = deque()

    def write(self, msg):
        self._q.append(msg)
        self._semaphore.release()

    def read(self, blocking=True):
        have_item = self._semaphore.acquire(blocking=blocking)
        if have_item:
            return self._q.popleft()
        else:
            return None
