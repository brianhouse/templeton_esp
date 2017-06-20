#!/usr/bin/env python3

import serial, threading, time, os, socket, queue, math
from housepy import config, log, timeutil


class ESPListener(threading.Thread):

    def __init__(self, port=23232, message_handler=None, blocking=False):
        super(ESPListener, self).__init__()
        self.daemon = True
        self.messages = queue.Queue()
        ESPHandler(self.messages, message_handler)
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.socket.bind(('', port))
        except Exception as e:
            log.error(log.exc(e))
            return
        self.start()
        if blocking:
            try:
                while True:
                    time.sleep(1)
            except (KeyboardInterrupt, SystemExit):
                self.connection.close()
                pass      

    def run(self):
        while True:
            try:
                message, address = self.socket.recvfrom(1024)   # shit, this is where the limit comes from
                ip, port = address
                batch = message.decode('utf-8')
                batch = batch.split(';')
                for d, data in enumerate(batch):
                    if not len(data):
                        continue
                    try:
                        data = data.split(',')
                        esp_id = int(data[0])
                        data = {'id': esp_id, 'rssi': int(data[1]), 'bat': int(float(data[2])), 'ip': ip, 't_utc': timeutil.timestamp(ms=True), 't': float(data[3]) / 1000.0, 'mag': float(data[4])}
                        self.messages.put(data)
                    except Exception as e:
                        log.error(log.exc(e))
            except Exception as e:
                log.error(log.exc(e))


class ESPHandler(threading.Thread):

    def __init__(self, messages, message_handler):
        super(ESPHandler, self).__init__()        
        if message_handler is None:
            return
        self.messages = messages
        self.message_handler = message_handler
        self.daemon = True
        self.start()

    def run(self):
        while True:
            try:
                message = self.messages.get()
                self.message_handler(message)
            except Exception as e:
                log.error(log.exc(e))


if __name__ == "__main__":
    start_t = None
    current_t = None
    entries = 0
    rate = 0
    def message_handler(response):
        global start_t, stop_t, entries, rate
        log.info("[ID %s] [IP %s] [RSSI %02d] [T %.3f] [BAT %02d] [MAG %.3f]" % (response['id'], response['ip'], response['rssi'], response['t'], response['bat'], response['mag']))
        entries += 1
        current_t = response['t']
        if start_t is None:
            start_t = current_t
        elapsed_t = current_t - start_t
        if elapsed_t > 0:
            rate = entries / elapsed_t
    fl = ESPListener(message_handler=message_handler)    

    while True:
        time.sleep(5)
        log.info("[HZ %.2f]" % rate)
