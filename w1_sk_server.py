# -*- coding: utf-8 -*-
"""
Created on Sat Mar 12 20:01:28 2022

@author: ndiay
"""

import sqlite3
import datetime
import sys
example_db = "/var/jail/home/team44/skweek1.db" # just come up with name of database
local_db = "/var/jail/home/team44/skweek1_local.db"
login_db = "/var/jail/home/team44/login.db" # contains name of user and time of last login

def create_database():
   conn = sqlite3.connect(example_db)  # connect to that database (will create if it doesn't already exist)
   c = conn.cursor()  # move cursor into database (allows us to execute commands)
   c.execute('''CREATE TABLE IF NOT EXISTS sk_table (pres real, alt real, temp real, timing timestamp, user text);''') # run a CREATE TABLE command
   conn.commit() # commit commands (VERY IMPORTANT!!)
   conn.close() # close connection to database

def request_handler(request):
    create_database()  #call the function!
    temp = sqlite3.connect(login_db)
    t = temp.cursor()
    t.execute('''CREATE TABLE IF NOT EXISTS login_table (user text, timing timestamp, isLoggedIn int);''') # run a CREATE TABLE command
    things = t.execute('''SELECT user FROM login_table ORDER BY timing DESC;''').fetchone()
    # return str(things)
    lmao = []
    for thing in things:
        lmao.append(thing)
    # return lmao
    currentUser = lmao[0][0]
    temp.commit()
    temp.close()
    if request["method"] == "GET":
        try:
            request['values']['hello'] == "7"
            conn = sqlite3.connect(example_db)
            c = conn.cursor()
            things = c.execute('''SELECT pres, alt, temp FROM sk_table WHERE user = ? ORDER BY timing DESC;''',(currentUser,)).fetchone()
            conn.commit()
            conn.close()
            return f"{int(things[0])},{int(things[1])},{int(things[2])}\r\n"
        except:
            conn = sqlite3.connect(example_db)
            c = conn.cursor()
            if len(c.execute(('''SELECT * FROM sk_table''')).fetchall()) == 0:
                c.execute('''INSERT OR IGNORE into sk_table VALUES (?,?,?,?,?);''',(0, 0, 0, datetime.datetime.now(), currentUser))
            things = c.execute('''SELECT pres, alt, temp FROM sk_table ORDER BY timing DESC;''').fetchone()
            conn.commit()
            conn.close()
            pres = things[0]
            alt = things[1]
            temp = things[2]
            conn2 = sqlite3.connect(local_db)
            c = conn2.cursor()
            if len(c.execute(('''SELECT * FROM location_table''')).fetchall()) == 0:
                c.execute('''INSERT OR IGNORE into location_table VALUES (?,?);''',("You Still Need To Select A Location!", datetime.datetime.now()))
            things = c.execute('''SELECT location FROM location_table ORDER BY timing DESC;''').fetchone()
            conn2.commit()
            conn2.close()

            location = things[0]
            return '''<!DOCTYPE html>
        <html>
        <body>

        <h1>Smart Kicks</h1>
        <h2> Current User: {}</h2>
        <p>Boost your way to success</p>
        <p>Pressure (Pa): {}</p>
        <p>Altitude (m): {}</p>
        <p>Temperature (F): {}</p>
        <p>Current Desired Location: {}</p>
        <form action="http://608dev-2.net/sandbox/sc/team44/w1_sk_server.py" method="post">   
          <label for="location">Teleport me to:</label>
<select name="location" id="location">
    <option value="lobby7">Lobby 7</option>
  <option value="stata">Stata Center</option>
  <option value="sloan">The Alfred P. Sloan School of Business</option>
  <option value="killian">Killian Court</option>
</select>
<br>
<br>
          
          <input type="submit">
        </form>

        </body>
        </html>
'''.format(currentUser, pres,alt,temp,location)
                    
    else:
        #return str(request)
        
        try:
            
            request['form']['hello'] == "7"
            pres = float(request['form']['pressure'])
            alt = float(request['form']['altitude'])
            temp = float(request['form']['temperature'])
            currentUser = request['form']['user']
            conn = sqlite3.connect(example_db)
            c = conn.cursor()
            c.execute('''CREATE TABLE IF NOT EXISTS sk_table (pres real, alt real, temp real, timing timestamp, user text);''') # run a CREATE TABLE command
            c.execute('''INSERT into sk_table VALUES (?,?,?,?,?);''',(pres, alt, temp, datetime.datetime.now(), currentUser))
            conn.commit()
            conn.close()
            conn = sqlite3.connect(local_db)
            c = conn.cursor()
            things = c.execute('''SELECT location FROM location_table ORDER BY timing DESC;''').fetchone()
            conn.commit()
            conn.close()
            location = things[0]
            
            return location
            
        except:
            try:
                location = str(request['form']['location'])
                conn = sqlite3.connect(local_db)
                c = conn.cursor()
                c.execute('''CREATE TABLE IF NOT EXISTS location_table (location text, timing timestamp);''') # run a CREATE TABLE command
                if c.execute('''SELECT location FROM location_table ORDER BY timing DESC;''').fetchone() != location:
                    c.execute('''INSERT into location_table VALUES (?,?);''',(location, datetime.datetime.now()))
                conn.commit()
                conn.close()
                conn = sqlite3.connect(example_db)
                c = conn.cursor()
                if len(c.execute(('''SELECT * FROM sk_table''')).fetchall()) == 0:
                    c.execute('''INSERT OR IGNORE into sk_table VALUES (?,?,?,?,?);''',(0, 0, 0, datetime.datetime.now(), currentUser))
                things = c.execute('''SELECT pres, alt, temp FROM sk_table WHERE user = ? ORDER BY timing DESC;''',(currentUser,)).fetchone()
                conn.commit()
                conn.close()
                pres = things[0]
                alt = things[1]
                temp = things[2]
                return '''<!DOCTYPE html>
            <html>
            <body>

            <h1>Smart Kicks</h1>
            <h2>Current User: {}</h2>
            <p>Boost your way to success</p>
            <p>Pressure (Pa): {}</p>
            <p>Altitude (m): {}</p>
            <p>Temperature (F): {}</p>
            <p>Current Desired Location: {}</p>
            <form action="http://608dev-2.net/sandbox/sc/team44/w1_sk_server.py" method="post">   
              <label for="location">Teleport me to (desired location):</label>
    <select name="location" id="location">
        <option value="lobby7">Lobby 7</option>
      <option value="stata">Stata Center</option>
      <option value="sloan">The Alfred P. Sloan School of Business</option>
      <option value="killian">Killian Court</option>
    </select>
    <br>
    <br>
              
              <input type="submit">
            </form>

            </body>
            </html>
    '''.format(currentUser, pres,alt,temp,location)

            except Exception as e:
                
                return str(e)
            
        
        
        
            
        
        
        