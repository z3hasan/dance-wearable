import sys
import time
import audiogen
import itertools

current_note = None

notes = [
    [440, "A"],
    [493.88, "B"],
    [523.25, "C"],
    [587.33,"D"],
    [659.25, "E"],
    [698.46, "F"],
    [783.99, "G"],
]

def play(freq):
    time = 0.5
    n = audiogen.sampler.play(audiogen.tone(freq), blocking=False)
    time.sleep(time)
    n.close()

play(440)
