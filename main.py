#!/usr/bin/env python3

from housepy import server, config, log, timeutil, strings
from mongo import ASCENDING, DESCENDING

class Home(server.Handler):

    def get(self, collar_id=None, session_id=None, start_t=None, stop_t=None):
        log.info("GET")
        session_list = []
        if not len(collar_id) or not len(session_id):
            collar_ids = list(self.db.entries.find().distinct("collar_id"))
            for collar_id in collar_ids:
                sessions = list(self.db.entries.find({'collar_id': collar_id}).distinct("session"))
                session_list.append({'collar_id': collar_id, 'sessions': sessions})
            return self.render("index.html", session_list=session_list)
        if not len(start_t):
            start_t = 0
        else:
            start_t = int(start_t)
        if not len(stop_t):
            stop_t = 86400000
        else:
            stop_t = int(stop_t)
        collar_id = strings.as_numeric(collar_id)
        session_id = strings.as_numeric(session_id)
        log.info("%d (%s-%s)" % (collar_id, start_t, stop_t))
        template = {'t': {'$gt': start_t, '$lt': stop_t}, 'collar_id': collar_id, 'session': session_id}
        log.debug(template)
        results = list(self.db.entries.find(template).sort('t'))
        start_segment = None
        stop_segment = None
        if len(results):
            start_segment = timeutil.seconds_to_string(results[0]['t'])
            stop_segment = timeutil.seconds_to_string(results[-1]['t'])
        for result in results:
            del result['_id']
            del result['session']
        log.debug("Returned %s entries" % len(results))
        return self.render("home.html", data=results, collar_id=collar_id, session=session_id, start_segment=start_segment, stop_segment=stop_segment)

    def post(self, nop1=None, nop2=None, nop3=None, nop4=None):
        log.info("POST")
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
        result = list(self.db.entries.find().limit(1).sort([('t', DESCENDING)]))
        if len(result):
            final_t = result[0]['t']
            session = result[0]['session']
        else:
            final_t = 0
            session = 1
        if max_t < final_t:
            session += 1
        for entry in entries:
            entry['session'] = session            
        try:
            self.db.entries.insert_many(entries)
        except Exception as e:
            log.error(log.exc(e))
        return self.text("OK")


handlers = [
    (r"/?([^/]*)/?([^/]*)/?([^/]*)/?([^/]*)", Home),
]    
server.start(handlers)