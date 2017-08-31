#!/usr/bin/env python3

import sys
from housepy import config, log
from pymongo import MongoClient, ASCENDING, DESCENDING

mongo = config['mongo']
client = MongoClient(mongo['host'], mongo['port'])
db = client[mongo['database']]

def make_indexes():
    try:
        db.entries.create_index("id")
        db.entries.create_index([("t", ASCENDING)])
    except Exception as e:
        log.error(log.exc(e))

if __name__ == "__main__":
    arg = None
    if len(sys.argv) > 1:
        arg = sys.argv[1]
    if arg == 'dump':
        result = db.entries.remove()
        print(result)
    else:
        make_indexes()
