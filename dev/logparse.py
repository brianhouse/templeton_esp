#!/usr/bin/env python3

import time
import numpy as np
from housepy import chart

ts = []
bats = []

with open("output.txt") as f:
    for line in f:
        try:
            t = float(line.split("[T ")[-1].split("]")[0]) / 60
            # bat = int((float(line.split("BAT ")[-1].split("%")[0]) * 1024))
            # bat = (((bat - 2724) / (3622 - 2724)) * 1.2) + 3.0
            bat = int(line.split("BAT ")[-1].split(']')[0])
            ts.append(t)
            bats.append(bat)
        except Exception as e:
            pass

data = np.array(list(zip(ts, bats)))

chart.plot(data)

chart.show(filename="battery_voltage.png", labels=True)


