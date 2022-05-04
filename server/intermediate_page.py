import sqlite3
import datetime
from bokeh.plotting import figure
from bokeh.embed import components
# def create_login_database():
#    conn = sqlite3.connect(login_db)  # connect to that database (will create if it doesn't already exist)
#    c = conn.cursor()  # move cursor into database (allows us to execute commands)
#    c.execute('''CREATE TABLE IF NOT EXISTS login_table (user text, timing timestamp, isLoggedIn int);''') # run a CREATE TABLE command
#    conn.commit() # commit commands (VERY IMPORTANT!!)
#    conn.close() # 

# def create_steps_database():
#    conn = sqlite3.connect(steps_db)  # connect to that database (will create if it doesn't already exist)
#    c = conn.cursor()  # move cursor into database (allows us to execute commands)
#    c.execute('''CREATE TABLE IF NOT EXISTS login_table (user text, timing timestamp, isLoggedIn int);''') # run a CREATE TABLE command
#    conn.commit() # commit commands (VERY IMPORTANT!!)
#    conn.close() # 

def request_handler(request):
    redirectpage = '''<!DOCTYPE html>
                <html>
                
                <meta http-equiv="refresh" content="0; URL=https://608dev-2.net/sandbox/sc/team44/login_w3.py" />

                <script>
                document.cookie = "";
                </script>

                </html>
                
            '''
    if(request['method'] == 'GET'):
        if('user' not in request['values']):
            # return "you goofed"
            return redirectpage
        return f'''<!DOCTYPE html>
        <html>
        <form action="http://608dev-2.net/sandbox/sc/team44/intermediate_page.py" method="post">
        <label for="plots">Click here to check your plot data:</label>
        <input type="submit" id="plots" name="plots"><br>
        <label for="maps">Click here to check your map data:</label><br>
        <input type="submit" id="maps" name="maps"><br>
        <label for="logout">Click here to logout:</label><br>
        <input type="submit" id="logout" name="logout"><br>
        <input type="hidden" id="user" name="user" value={request['values']['user']}>User: {request['values']['user']}</input>
        </form>
        </html>
        ''' 
        #return some HTTP with graphs here!
    else:
        if('plots' in request['form'] and 'user' in request['form']):
            redirectpage = '''<!DOCTYPE html>
                <html>

                <meta http-equiv="refresh" content="0; URL=https://608dev-2.net/sandbox/sc/team44/plots.py?user={}" />
                <script>
                    document.cookie = "";
                </script>

                </html>
                
            '''.format(request['form']['user'])
        elif('maps' in request['form'] and 'user' in request['form']):
            redirectpage = '''<!DOCTYPE html>
                <html>

                <meta http-equiv="refresh" content="0; URL=https://608dev-2.net/sandbox/sc/team44//map/index.html?user={}" />

                <script>
                document.cookie = "";
                </script>

                </html>
                
            '''.format(request['form']['user'])
        return redirectpage