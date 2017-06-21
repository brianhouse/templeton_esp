#!/usr/bin/env python3

from housepy import server, config, log

class Home(server.Handler):

    def get(self, page=None):
        return self.text("OK")

    def post(self, nop=None):
        log.info("POST")
        raw = str(self.request.body, encoding="utf-8")
        batch = raw.split(';')
        for d, data in enumerate(batch):
            if not len(data):
                continue
            try:
                data = data.split(',')
                response = {'id': int(data[0]), 'rssi': int(data[1]), 'bat': int(float(data[2])), 't': float(data[3]) / 1000.0, 'mag': float(data[4])}
                log.info("[ID %s] [RSSI %02d] [T %.3f] [BAT %02d] [MAG %.3f]" % (response['id'], response['rssi'], response['t'], response['bat'], response['mag']))
            except Exception as e:
                log.error(log.exc(e))        
        return self.text("OK")

handlers = [
    (r"/?([^/]*)", Home),
]    
server.start(handlers)