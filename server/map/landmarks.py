
from ctypes.wintypes import LANGID
import sqlite3
current_db = '/var/jail/home/team44/map/main.db'
from datetime import datetime, timedelta

def get_now_time() -> int:
    return int(datetime.now().timestamp())

def request_handler(request) -> str:
    with sqlite3.connect(current_db) as c:
        c.execute('''CREATE TABLE IF NOT EXISTS landmarks (user text, landmark_name real, timing int)''')

    if request["method"] == "POST":
        try:
            user = str(request["form"]["user"])
            landmark_name = str(request["form"]["landmark_name"])

        except:
            return "Error: Invalid POST body."
        
        with sqlite3.connect(current_db) as c:
                c.execute('''INSERT into landmarks VALUES (?, ?, ?)''', (user, landmark_name, get_now_time() ))
 
        return "Succesfully POSTed location data."


    if request["method"] == "GET":
        return "Nothing to see here."
    
    return "Error."
