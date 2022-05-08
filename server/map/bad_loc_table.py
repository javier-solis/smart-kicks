
import sqlite3
import sys
from datetime import datetime, timedelta

sys.path.append('/var/jail/home/team44/map/')
current_db = '/var/jail/home/team44/map/main.db'
from main import make_html_table


def request_handler(request) -> str:
    c.execute('''CREATE TABLE IF NOT EXISTS bad_loc_data (lat real, lon real)''') # this exists just for curiosity purposes. May be useful later on too.

    with sqlite3.connect(current_db) as c:
        allRows = c.execute('''SELECT * FROM bad_loc_data''').fetchall()

    html_render = make_html_table(("Latitude", "Longitude"), allRows)
    
    return html_render