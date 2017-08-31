#!/usr/bin/env python3

import time, os, queue
from collections import deque, OrderedDict
from esp_link import ESPListener
from housepy import config, log, timeutil, animation

sensor_data = {}
sensor_rssi = OrderedDict()
labels = []

RANGE = -20, 20
COLORS = (.8, 0., 0., 1.), (0., .8, 0., 1.), (0., 0., .8, 1.), (.8, .8, 0., 1.), (0., .8, .8, 1.), (.8, 0., .8, 1.)


def draw():
    t_now = timeutil.timestamp(ms=True)

    # do labels
    for s, (esp_id, (t, rssi)) in enumerate(sensor_rssi.copy().items()):
        if t_now - t > 3:
            bar = 0.01
        else:
            bar = 1.0 - (max(abs(rssi) - 25, 0) / 100)
        x = (20 + (s * 20)) / ctx.width
        ctx.line(x, .1, x, (bar * 0.9) + .1, color=COLORS[esp_id % len(COLORS)], thickness=10)
        if esp_id not in labels:
            print("Adding label for esp_id %s" % esp_id)
            labels.append(esp_id)
            ctx.label(x, .05, str(esp_id), font="Monaco", size=10, width=10, center=True)

    # data
    for s, esp_id in enumerate(list(sensor_data)):
        samples = sensor_data[esp_id]    
        if len(samples):
            ctx.lines([ ((t_now - sample[0]) / 10.0, (sample[1] - RANGE[0] - 9.8) / (RANGE[1] - RANGE[0]) ) for sample in list(samples)], color=COLORS[esp_id % len(COLORS)])    # subtract 9.8 to center it


if __name__ == "__main__":
    def message_handler(response):
        # db.branches.insert(data)
        if response['id'] not in sensor_data:
            sensor_data[response['id']] = deque()
            sensor_rssi[response['id']] = None
        sensor_data[response['id']].appendleft((response['t_utc'], response['mag']))
        sensor_rssi[response['id']] = response['t_utc'], response['rssi']
        if len(sensor_data[response['id']]) == 1000:
            sensor_data[response['id']].pop()        
    ESPListener(message_handler=message_handler)    

    ctx = animation.Context(1000, 300, background=(1., 1., 1., 1.), fullscreen=False, title="RAT", smooth=True)    
    ctx.start(draw)