#!/usr/bin/env python3

from housepy import server, config, log

class Home(server.Handler):

    def get(self, page=None):
        return self.text("OK")

    def post(self, nop=None):
        log.info("POST")
        raw = str(self.request.body, encoding="utf-8")
        batch = raw.split(';')
        d = 0
        for data in batch:
            if not len(data):
                continue
            try:
                if data[0:8] == data[8:16]: # who knows
                    data = data[8:]
                assert len(data) == 31  # minus ;
                fields = data.split(',')
                response = {'id': int(fields[0]), 'rssi': int(fields[1]), 'bat': int(float(fields[2])), 't': float(fields[3]) / 1000.0, 'mag': float(fields[4])}
                log.info("[ID %s] [RSSI %02d] [T %.3f] [BAT %02d] [MAG %.3f]" % (response['id'], response['rssi'], response['t'], response['bat'], response['mag']))
                d += 1
            except AssertionError as e:
                log.error(data)
                log.error("Length is %d" % len(data))
            except Exception as e:
                log.error(log.exc(e))   
                log.error(data)    
        log.info("--> received %d entries" % d) 
        return self.text("OK")

handlers = [
    (r"/?([^/]*)", Home),
]    
server.start(handlers)