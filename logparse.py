#!/usr/bin/env python3

import time
import numpy as np
from housepy import chart

ts = []
bats = []

with open("esp_link.log") as f:
    for line in f:
        t = float(line.split("[T ")[-1].split("]")[0]) / 60
        bat = float(line.split("BAT ")[-1].split("%")[0]) + .71
        ts.append(t)
        bats.append(bat)

data = np.array(list(zip(ts, bats)))

chart.plot(data)

chart.show(filename="battery_voltage.png", labels=True)