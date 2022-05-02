import sqlite3
from datetime import datetime, timedelta
import json
from typing import *
import numpy as np
import sys
from bokeh.plotting import figure
from bokeh.embed import json_item

# ==

# Importing Functions From Other Python Files
sys.path.append('/var/jail/home/team44/map/')
from geo_funcs import *

# Database Stuff
current_db = '/var/jail/home/team44/map/main.db'

# Time Stuff
time_format = "%m-%d-%Y, %H:%M:%S"
# tz = timezone('EST')

# Precision Variables

degPrecision = 6 # storing at most 6 decimal places, because that's the best accuracy 
                # that I think is possible, plus makes formatting easier/better.
distPrecision = 1 # at most 0.1 meters
timePrecision = 0 # round to nearest second

# Locations
TuftMedical= (42.34956664091039, -71.06445822683442)
HarvardStadium = (42.36675464638942, -71.12651381366604)

# ==

class GetRequestTypes():
    """
    Class that sort of acts like an enum of all acceptable GET requests, along 
    with some helpful methods.
    All of these also expect a username value.
    """

    def __init__(self) -> None:
        self.trail_table="trail-table"
        self.trail_json="trail-map"
        self.destination="destination"
        self.vel_tables="velocity-tables"
        self.vel_json="velocity-json"
    
    def all_members(self) -> set:
        mapping = vars(self)
        result = set()
        for key in mapping.keys():
            result.add(mapping[key])

        return result
    
    def includes(self, that):
        return that in self.all_members()

    def size(self) -> int:
        return len(self.all_members())        

get_request = GetRequestTypes()

def make_html_table(headers, allRows) -> str:
    html_render=f"<style>table, th, td {{border:1px solid black;}}</style>"

    html_render+="<table> <tr>"
    for col in headers:
        html_render+=f"<th>{str(col)}</th>"
    html_render+="</tr>"
    
    for row in allRows:
        if(len(headers)!=len(row)):
            raise Exception("Header row and field row dimensions did not match.")

        html_render+="<tr>"
        for col in row:
            html_render+=f"<td>{col}</td>"
        html_render+="</tr>"

    html_render+="</table>"

    return html_render

def check_in_bounds(coord: Tuple[float, float]):
    """
    Returns True if coord is within MIT's rectangular bounds, False otherwise
    """
    top_left = HarvardStadium
    bot_right = TuftMedical

    return (top_left[0] < coord[0] and coord[1] < top_left[1] and \
            coord[0] < bot_right[0] and bot_right[0] < coord[1])

def make_datatime_object(unix_time: int) -> datetime: # seems a little complex but im not sure what else to do
    return datetime.strptime( datetime.utcfromtimestamp(unix_time).strftime(time_format) , time_format) 

def get_now_time() -> int:
    return int(datetime.now().timestamp())

# ==

def get_data(GET_type: str, user: str) -> str:

    # === Mostly used by ESP32 ===

    if GET_type == get_request.destination:
        with sqlite3.connect(current_db) as c:
            row = c.execute('''SELECT landmark_name FROM landmarks WHERE user=? ORDER BY timing DESC;''', (user,)).fetchone()
        
        if row==None:
            return "User has never chosen a landmark."

        return row[0]


    one_hour_ago = get_now_time() - 60*60

    # === Mostly used by web-browser for debugging or the curious ===

    # Checking that the user has ever collected data.
    with sqlite3.connect(current_db) as c:
        userExistsCheck = c.execute("SELECT EXISTS(SELECT 1 FROM all_users WHERE user=?)", (user,)).fetchone()

    if(not userExistsCheck[0]):
        return "User has not collected any data."

    # Checking that a valid GET request name was given
    if not get_request.includes(GET_type):
        return "Invalid request name."

    if GET_type == get_request.trail_table:

        with sqlite3.connect(current_db) as c:
            allRows = c.execute('''SELECT * FROM loc_data WHERE user=? AND timing>?''', (user, one_hour_ago)).fetchall()

        html_render = make_html_table(("User", "Latitude", "Longitude", "Distance Delta (m)", "Time Delta (s)", "Time (Unix)"), allRows)
        
        return html_render


    elif GET_type == get_request.vel_tables: # for now, showing the latest 1 hour
        with sqlite3.connect(current_db) as c:
            allRows = c.execute('''SELECT * FROM vel_data WHERE user=? AND timing > ?''', (user, one_hour_ago)).fetchall()

        html_render = make_html_table(("User", "Consecutive Velocity (m/s)", "Average Velocity (m/s)", "Time Delta (s)", "Distance Delta (m)", "Time (Unix)"), allRows)
        
        return html_render

    # === Mostly used in JS code ===

    elif GET_type == get_request.trail_json:
        with sqlite3.connect(current_db) as c:
            rows = c.execute('''SELECT lat, lon, timing FROM loc_data WHERE user=? AND timing>? ORDER BY timing DESC;''', (user, one_hour_ago)).fetchall() # decreasing in time

            output: Dict[str, Any] = {"locations": [], "timing": []}
            for row in rows:
                output["locations"].append([row[0],row[1]])
                output["timing"].append(row[2])

            return json.dumps(output)

    elif GET_type == get_request.vel_json:
        with sqlite3.connect(current_db) as c:
            allRows = c.execute('''SELECT * FROM vel_data WHERE user=? AND timing>? ORDER by timing ASC''', (user, one_hour_ago)).fetchall()

        plot = figure(x_axis_label="Time (s)", y_axis_label="Velocity (m/s)", x_axis_type='datetime')
        
        time: List[datetime] = []
        consec_vel: List[float] = []
        avg_vel: List[float] = []
        
        for row in allRows:
            consec_vel.append(float(row[1]))
            avg_vel.append(float(row[2]))

            time_slice = make_datatime_object(row[3])
            time.append(time_slice)

        # Week 4: make the graph more colorful/visually pleasing
        plot.line(time, consec_vel, legend_label="Consecutive Velocity", line_color="orange", line_width=2, )
        plot.line(time, avg_vel, legend_label="Average Velocity", line_color="green", line_width=2)

        return json.dumps(json_item(plot, "myplot"))

    else:
        return "Error."

# ==

def request_handler(request) -> str:
    one_hour_ago: int = get_now_time() - 60*60

    with sqlite3.connect(current_db) as c:
        c.execute('''CREATE TABLE IF NOT EXISTS loc_data (user text, lat real, lon real, dist_delta real, time_delta, timing int)''') # dist_delta in meters
        c.execute('''CREATE TABLE IF NOT EXISTS vel_data (user text, consec_vel real, avg_vel real, timing int)''')
        c.execute('''CREATE TABLE IF NOT EXISTS all_users (user text)''')

    if request["method"] == "POST":
        try:
            user = str(request["form"]["user"])
            lat = round(float(request["form"]["lat"]), degPrecision)
            lon = round(float(request["form"]["lon"]), degPrecision)

        except:
            return "Error: Invalid POST body."
        
        with sqlite3.connect(current_db) as c:

            user_exists = c.execute('''SELECT * FROM all_users WHERE user=?''', (user,)).fetchone()
        
            if user_exists==None: # this is needed if we're dealing with a new user

                c.execute('''INSERT into all_users VALUES (?)''', (user,)) # edit so that it inserts only if it doesnt exist already in the DB
                c.execute('''INSERT into loc_data VALUES (?, ?, ?, ?, ?, ?)''', (user, lat, lon, 0, 0, get_now_time() ))
            else:

                lat_lon_timing_latest = c.execute('''SELECT lat, lon, timing FROM loc_data WHERE user=? and timing>? ORDER BY timing DESC LIMIT 1''', (user,one_hour_ago)).fetchone()
                oldest_time = c.execute('''SELECT timing FROM loc_data WHERE user=? AND timing > ? ORDER BY timing ASC LIMIT 1;''', (user, one_hour_ago)).fetchone()
                dist_deltas = c.execute('''SELECT dist_delta FROM loc_data WHERE user=? AND timing>? ORDER BY timing DESC;''', (user, one_hour_ago)).fetchall()

                # this should happen once during the 2 hour session
                if (lat_lon_timing_latest==None or oldest_time==None or len(dist_deltas)==0):
                    c.execute('''INSERT into loc_data VALUES (?, ?, ?, ?, ?, ?)''', (user, lat, lon, 0, 0, get_now_time() ))

                else:

                    last_time = int(lat_lon_timing_latest[2])
                    time_delta = get_now_time() - last_time
                    
                    last_coord = (lat_lon_timing_latest[0], lat_lon_timing_latest[1])
                    new_coord = (lat, lon)
                    distDelta = round(get_distance(new_coord, last_coord), distPrecision)
                
                    total_time: int = get_now_time()- oldest_time[0]

                    total_distance: float = 0
                    for dist in dist_deltas:
                        total_distance += dist[0]

                    avg_vel: float = total_distance/total_time
                    consec_vel: float
                    
                    if time_delta==0:
                        consec_vel = 0
                    else:
                        consec_vel = distDelta/time_delta

                    c.execute('''INSERT into loc_data VALUES (?, ?, ?, ?, ?, ?)''', (user, lat, lon, distDelta, time_delta, get_now_time()))
                    c.execute('''INSERT into vel_data VALUES (?, ?, ?, ?)''', (user, consec_vel, avg_vel, get_now_time()))

        return "Succesfully POSTed location data."


    elif request["method"] == "GET":
        if(len(request["values"].keys())>0):
        
            requestType = list(request["values"].keys())[0] # getting the name of the type that interested, there might be a better way to do this !!!!!!!!!!!!!!!!!1

            try:
                user = request["values"][requestType]
            except:
                return "Missing parameter's value."

            return get_data(requestType, user)

        else:
            return "Missing a parameter."
    
    return "Error."
