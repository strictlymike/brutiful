# Brute force script. Iterates through all possible values of a buffer given a
# particular set of characters and runs each string value through an evaluator
# function to determine whether it was correct. First used for Hackptch
# challenge in FireEye / ForAll Secure CTF 2016, using socket library to check
# each password against the server. It was... Slow going. But I thought I would
# kick it off while I worked on the problem from a different angle, because I'm
# an opportunist like that.

import string
import sys

class IncString:
    """Incrementing string. Iterates through all values for that string given
    the specified character set.
    """
    defaultcharset = string.ascii_letters + string.digits + string.punctuation
    charset = defaultcharset
    def __init__(self, length, charset=defaultcharset):
        self.charset = charset
        self.length = length
        self.chars = len(charset)
        self.buf = [0 for i in range(self.length)]

    def value(self):
        """Get string value. Returns the current value of the string."""
        s = ''.join([self.charset[i] for i in self.buf])
        return s

    def charsetlen(self):
        """Return the length of the character set being used."""
        return len(self.charset)

    def increment(self):
        """Increment string. Increments the least significant element of the
        string, rolling over and moving to the next element if necessary.
        """
        for i in range(self.length-1, -1, -1):
            self.buf[i] = self.buf[i] + 1
            if (self.buf[i] < self.chars):
                break
            self.buf[i] = 0

def try_a_value(length, seq, buf):
    """Evaluate whether the current buffer value is the solution. Sequence
    number is for adding restart logic."""
    # print 'Trying (' + str(length) + ', ' + str(seq) + '): ' + buf
    if buf == "Ha!":
        return True
    return False

charset = string.ascii_letters + string.digits + string.punctuation
len_start = 0
len_max = 100
len_end = len_max + 1

for length in range(len_start, len_end):
    s = IncString(length, charset)
    chars = s.charsetlen()
    combinations = pow(chars, length)

    seq_start = 0
    seq_end = combinations

    for i in range(seq_start, seq_end):
        done = False
        while not done:
            if try_a_value(length, i, s.value()):
                print 'Solution: ' + s.value()
                sys.exit(0)
            done = True
        s.increment()
