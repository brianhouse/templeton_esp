#!/usr/bin/env python3

import serial, threading, time, os, socket, queue, math
from housepy import config, log, timeutil


class ESPListener(threading.Thread):

    def __init__(self, port=23232, message_handler=None, blocking=False):
        super(ESPListener, self).__init__()
        self.daemon = True
        self.messages = queue.Queue()
        ESPHandler(self.messages, message_handler)
        self.events = {}
        self.rates = {}
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
        t_start = time.time()
        while True:
            try:
                message, address = self.socket.recvfrom(1024)
                ip, port = address
                data = message.decode('utf-8').split(',')
                esp_id = int(data[0])
                if esp_id not in self.events:
                    self.events[esp_id] = []
                if esp_id not in self.rates:
                    self.rates[esp_id] = 0
                self.events[esp_id].append(1)
                data = {'id': esp_id, 'rssi': int(data[1]), 'bat': int(((math.floor(int(data[2])/100)/10) / 4.0) * 100), 'rate': self.rates[esp_id], 'ip': ip, 't_utc': timeutil.timestamp(ms=True), 'data': [v for v in data[3:]]}
                self.messages.put(data)
                elapsed_t = time.time() - t_start
                if elapsed_t >= 1:
                    for esp_id in self.events:
                        events = len(self.events[esp_id])
                        rate = math.floor(events / elapsed_t)
                        self.rates[esp_id] = rate
                        self.events[esp_id] = []
                    t_start = time.time()
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


class ESPSender(threading.Thread):

    def __init__(self, blocking=False):
        super(ESPSender, self).__init__()
        self.daemon = True
        self.messages = queue.Queue()
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.settimeout(1)
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
                message, address = self.messages.get()
                log.info("SENDING [%s] to %s:%s" % (message, address[0], address[1]))
                self.socket.sendto(message.encode('ascii'), address)
            except Exception as e:
                log.error(log.exc(e))

    def send(self, message, address):
        self.messages.put((message, address))


if __name__ == "__main__":
    fs = ESPSender()
    def message_handler(response):
        log.info("%f [ID %s] [IP %s] [RSSI %d] [BAT %s%%] [HZ %s]:\t%s" % (response['t_utc'], response['id'], response['ip'], response['rssi'], response['bat'], response['rate'], "".join(["%s " % f for f in response['data']])))
    fl = ESPListener(message_handler=message_handler)    
    while True:
        time.sleep(1)