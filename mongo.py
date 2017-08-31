#!/usr/bin/env python3

import sys
from housepy import config, log
from pymongo import MongoClient, ASCENDING, DESCENDING

mongo = config['mongo']
client = MongoClient(mongo['host'], mongo['port'])
db = client[mongo['database']]

def make_indexes():
    try:
        db.entries.drop_indexes()
        db.entries.create_index("collar_id")
        db.entries.create_index("session")
        db.entries.create_index([("t", ASCENDING)])
        db.entries.create_index([("t", DESCENDING)])
    except Exception as e:
        log.error(log.exc(e))

if __name__ == "__main__":
    arg = None
    if len(sys.argv) > 1:
        arg = sys.argv[1]
    else:
        print("[dump|index]")
        exit()
    if arg == 'dump':
        print("Dumping all entries...")
        result = db.entries.remove()
        print(result)
    if arg == 'index':
        print("Regenerating indexes...")
        make_indexes()
        print("--> done")