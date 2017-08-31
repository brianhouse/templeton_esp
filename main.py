#!/usr/bin/env python3

from housepy import server, config, log, timeutil, strings
from mongo import db

class Home(server.Handler):

    def get(self, collar_id=None, start=None, stop=None):
        log.info("GET")
        if not len(collar_id):
            return self.text("/collar_id/start/end")
        if not len(start):
            start = "2017-01-01"
        if not len(stop):
            stop = "2020-01-31"
        collar_id = strings.as_numeric(collar_id)
        start = timeutil.string_to_dt(start, "America/New_York")            
        stop = timeutil.string_to_dt(stop, "America/New_York")
        log.info("%d (%s-%s)" % (collar_id, start, stop))
        start_t = timeutil.timestamp(start)
        stop_t = timeutil.timestamp(stop)
        template = {'t': {'$gt': start_t, '$lt': stop_t}, 'collar_id': collar_id}
        log.debug(template)
        results = list(db.entries.find(template).sort('t'))
        if len(results):
            min_t = results[0]['t']
            max_t = results[-1]['t'] - min_t
        for result in results:
            del result['_id']
            result['t'] -= min_t
            result['t'] /= max_t
        log.debug("Returned %s entries" % len(results))
        return self.render("home.html", data=results)

    def post(self, nop1=None, nop2=None, nop3=None):
        log.info("POST")
        current_t = timeutil.timestamp()
        raw = str(self.request.body, encoding="utf-8")
        batch = raw.split(';')
        d = 0
        entries = []
        for data in batch:
            if not len(data):
                continue
            try:
                if data[0:8] == data[8:16]: # who knows
                    data = data[8:]
                assert len(data) == 31  # minus ;
                fields = data.split(',')
                response = {'collar_id': int(fields[0]), 'rssi': int(fields[1]), 'bat': int(float(fields[2])), 't': (float(fields[3]) / 1000.0), 'mag': float(fields[4])}
                log.info("[ID %s] [RSSI %02d] [T %.3f] [BAT %02d] [MAG %.3f]" % (response['collar_id'], response['rssi'], response['t'], response['bat'], response['mag']))
                entries.append(response)
                d += 1
            except AssertionError as e:
                log.error(data)
                log.error("Length is %d" % len(data))
            except Exception as e:
                log.error(log.exc(e))   
                log.error(data)  
        log.info("--> received %d entries" % len(entries)) 
        entries.sort(key=lambda entry: entry['t'])
        max_t = entries[-1]['t']
        for entry in entries:
            entry['t'] = (current_t - 5.0 - max_t) + entry['t']     # 5 second delay on transmission
        try:
            db.entries.insert_many(entries)
        except Exception as e:
            log.error(log.exc(e))
        return self.text("OK")


handlers = [
    (r"/?([^/]*)/?([^/]*)/?([^/]*)", Home),
]    
server.start(handlers)