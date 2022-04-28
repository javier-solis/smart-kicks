
import sqlite3
import sys
from datetime import datetime, timedelta

sys.path.append('/var/jail/home/team44/map/')
from main import make_html_table, get_now_time

current_db = '/var/jail/home/team44/map/main.db'

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

        if("table" in request["values"]):  #just checking for truthiness, really
            if(len(request["values"]["table"])>0): # so this line isn't really needed

                with sqlite3.connect(current_db) as c:
                    allRows = c.execute('''SELECT * FROM landmarks''').fetchall()

                html_render = make_html_table(("User", "Landmark Chosen", "Time"), allRows)
                
                return html_render
    
    return "Error."
