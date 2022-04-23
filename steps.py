# -*- coding: utf-8 -*-
"""
Created on Sat Mar 12 20:01:28 2022

@author: ndiay
"""

import sqlite3
import datetime
steps_db = "/var/jail/home/team44/steps.db" # just come up with name of database


def create_database():
   conn = sqlite3.connect(steps_db)  # connect to that database (will create if it doesn't already exist)
   c = conn.cursor()  # move cursor into database (allows us to execute commands)
   c.execute('''CREATE TABLE IF NOT EXISTS steps_table (user text, timing timestamp, steps int);''') # run a CREATE TABLE command
   conn.commit() # commit commands (VERY IMPORTANT!!)
   conn.close() # close connection to database



def request_handler(request):
    create_database()  #call the function!
    if request["method"] == "POST":
        user = request['form']['user']
        steps = request['form']['steps']
        conn = sqlite3.connect(steps_db)
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS steps_table (user text, timing timestamp, steps int);''') # run a CREATE TABLE command
        c.execute('''INSERT into steps_table VALUES (?,?,?);''',(user, datetime.datetime.now(),steps))
        conn.commit()
        conn.close()

            
        
        
        
            
        
        
        