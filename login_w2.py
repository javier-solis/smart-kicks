import sqlite3
import datetime

login_db = "/var/jail/home/team44/login.db" # contains name of user and time of last login
users_db = "/var/jail/home/team44/users.db"
# location.href="http://608dev-2.net/sandbox/sc/team44/plots.py"
# <meta http-equiv="refresh" content="time; URL=http://608dev-2.net/sandbox/sc/team44/plots.py" />
users = []
loginpage = '''<!DOCTYPE html>
                <html>
                <h1>Sign in!</h1>
                <p>Enter your username. Then, we'll show your stats :)</p>
                <form action="http://608dev-2.net/sandbox/sc/team44/login_w2.py" method="post">   
                <label for="username">Username:</label>
                <input type="text" id="username" name="username" />
                </form>
                </html>
            '''
redirectpage = '''<!DOCTYPE html>
                <html>
                <meta http-equiv="refresh" content="0; URL=http://608dev-2.net/sandbox/sc/team44/plots.py" />
                </html>
            '''

def create_login_database():
   conn = sqlite3.connect(login_db)  # connect to that database (will create if it doesn't already exist)
   c = conn.cursor()  # move cursor into database (allows us to execute commands)
   c.execute('''CREATE TABLE IF NOT EXISTS login_table (user text, timing timestamp, isLoggedIn int);''') # run a CREATE TABLE command
   conn.commit() # commit commands (VERY IMPORTANT!!)
   conn.close() # close connection to database

def create_user_database():
   conn = sqlite3.connect(users_db)  # connect to that database (will create if it doesn't already exist)
   c = conn.cursor()  # move cursor into database (allows us to execute commands)
   c.execute('''CREATE TABLE IF NOT EXISTS user_table (user text);''') # run a CREATE TABLE command
   conn.commit() # commit commands (VERY IMPORTANT!!)
   conn.close() # close connection to database

def request_handler(request):
    create_login_database()  #call the function!
    create_user_database()
    if request["method"] == "GET":
        # get the most recent login, see if they're still logged in
        # if still logged in, redirect to plots.py page
        # if not still logged in, just return the default website
        conn = sqlite3.connect(login_db)
        c = conn.cursor()
        nconn = sqlite3.connect(users_db)
        nc = nconn.cursor()
        if len(nc.execute(('''SELECT * FROM user_table''')).fetchall()) == 0:
            nconn.commit()
            nconn.close()
            conn.commit()
            conn.close()
            return loginpage
        things = c.execute('''SELECT user, isLoggedIn FROM login_table ORDER BY timing DESC;''').fetchall()
        lmao = []
        for stuff in things:
            lmao.append(stuff)
        conn.commit()
        conn.close()
        nconn.commit()
        nconn.close()
        if(lmao[0][1] == 1):
        # if(things[1] == 1):
            return redirectpage
        else:
            return loginpage
    else:
        username = request['form']['username']
        conn = sqlite3.connect(users_db)
        c = conn.cursor()
        things = c.execute(('''SELECT * FROM user_table''')).fetchall()
        for row in things:
            users.append(row[0])
        if(username not in users):
            c.execute('''INSERT into user_table VALUES (?);''',(username,))
        newconn = sqlite3.connect(login_db)
        nc = newconn.cursor()
        nc.execute('''INSERT into login_table VALUES (?,?,?);''',(username, datetime.datetime.now(), 1))
        newconn.commit()
        newconn.close()
        conn.commit()
        conn.close()
        return redirectpage
