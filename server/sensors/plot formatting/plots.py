import sqlite3
import datetime
from bokeh.plotting import figure
from bokeh.embed import components
#script is meant for local development and experimentation with bokeh
example_db = "/var/jail/home/team44/skweek1.db" #database has table called sensor_data with entries: time_ timestamp, user text, temperature real, pressure real
# example_db = "example.db"
# USERS = login_w2.users # eventually, the "sign in" page on the website will add users to this list
login_db = "/var/jail/home/team44/login.db" # contains name of user and time of last login
steps_db = "/var/jail/home/team44/steps.db"
redirectpage = '''<!DOCTYPE html>
                <html>
                <meta http-equiv="refresh" content="0; URL=http://608dev-2.net/sandbox/sc/team44/login_w3.py" />
                </html>
                <script>
                document.cookie = "";
                </script>
            '''
# def create_login_database():
#    conn = sqlite3.connect(login_db)  # connect to that database (will create if it doesn't already exist)
#    c = conn.cursor()  # move cursor into database (allows us to execute commands)
#    c.execute('''CREATE TABLE IF NOT EXISTS login_table (user text, timing timestamp, isLoggedIn int);''') # run a CREATE TABLE command
#    conn.commit() # commit commands (VERY IMPORTANT!!)
#    conn.close() # 

def create_steps_database():
   conn = sqlite3.connect(steps_db)  # connect to that database (will create if it doesn't already exist)
   c = conn.cursor()  # move cursor into database (allows us to execute commands)
   c.execute('''CREATE TABLE IF NOT EXISTS login_table (user text, timing timestamp, isLoggedIn int);''') # run a CREATE TABLE command
   conn.commit() # commit commands (VERY IMPORTANT!!)
   conn.close() # 

def request_handler(request):
    # temp = sqlite3.connect(login_db)
    # t = temp.cursor()
    # t.execute('''CREATE TABLE IF NOT EXISTS login_table (user text, timing timestamp, isLoggedIn int);''') # run a CREATE TABLE command
    # temp.commit()
    # temp.close()
    # create_login_database()
    # temp = sqlite3.connect(login_db)
    # t = temp.cursor()
    # things = t.execute('''SELECT user FROM login_table ORDER BY timing DESC;''').fetchall()
    # temp.commit() # commit commands (VERY IMPORTANT!!)
    # temp.close()
    # return "lmao get trolled yyub"
    # return things
    # lmao = []
    # for thing in things:
    #     lmao.append(thing)
    # return lmao
    # current = things[0][0]
    global current
    current = None

    if request['method'] =="GET":
        if('user' not in request['values']):
            # return "you goofed"
            return redirectpage
        current = request['values']['user']
        if(current == ''):
            # return "stop goofing"
            return redirectpage
        return f'''<!DOCTYPE html>
        <html>
        <body>

        <h1>Pick your plot...</h1>
        <p>Boost your way to success</p>
              <form action="http://608dev-2.net/sandbox/sc/team44/plots.py?user={current}" method="post"> 
          <label for="plot">I want to see my...</label>
<select name="plot" id="plot">
 <option value="Velocity">Velocity</option>
 <option value="Altitude">Altitude</option>
 <option value="Step Count">Step Count</option>
 <option value="Temperature">Temperature</option>
</select>
<br>
<br>
    <input type="submit">
        </form>
        </body>
        </html>
'''
        
        #return some HTTP with graphs here!
    else:
        # create_login_database()
        # conn = sqlite3.connect(login_db)
        # c = conn.cursor()
        # c.execute('''INSERT into login_table VALUES (?,?,?);''',(current, datetime.datetime.now(), 0))
        # conn.commit()
        # conn.close()
        
        choice = request["form"]["plot"]
        current = request['values']['user']

        now = datetime.datetime.now()
        conn = sqlite3.connect(example_db)
        c = conn.cursor()
        plot2 = figure()
        plot3 = figure()
        plot4 = figure()
        x = []
        pressure = []
        temperature = []
        altitude = []
        steps = []
        stepsx = []
        # for i in range(len(USERS)):
        print("New User")
        # things = c.execute('''SELECT pres, alt, temp, timing FROM sk_table WHERE user = ? ORDER by timing ASC;''',(USERS[i],)).fetchall()
        things = c.execute('''SELECT pres, alt, temp, timing FROM sk_table WHERE user = ? ORDER by timing DESC LIMIT 1000;''', (current,)).fetchall()
        print(things)
        for row in things:
            # print(row)
            if row[0] is None or row[1] is None or row[2] is None or row[3] is None:
                raise Exception(f"{row = }")
            pressure.append(float(row[0]))
            temperature.append(float(row[2]))
            altitude.append(float(row[1]))
            dto = datetime.datetime.strptime(row[3],'%Y-%m-%d %H:%M:%S.%f')
            x.append(dto)
        # print("pressure: ", pressure)
        # print("temperature: ", temperature)
        # print("x: ", x)
        nconn = sqlite3.connect(steps_db)
        nc = nconn.cursor()
        stepThing = nc.execute('''SELECT steps, timing FROM steps_table WHERE user = ? ORDER by timing DESC LIMIT 1000;''', (current,)).fetchall()
        for row in stepThing:
            if row[0] is None:
                raise Exception(f"{row = }")
            steps.append(float(row[0]))
            dto = datetime.datetime.strptime(row[1],'%Y-%m-%d %H:%M:%S.%f')
            print("dto: ", dto)
            stepsx.append(dto)
        plot2.xaxis.axis_label = "time (sec)"
        plot3.xaxis.axis_label = "time (sec)"
        plot4.xaxis.axis_label = "time (sec)"
        plot2.yaxis.axis_label = "altitude (m)"
        plot3.yaxis.axis_label = "temperature (F)"
        plot4.yaxis.axis_label = "steps"
    
        plot2.line(x, altitude, legend_label="altitude", line_dash=[4, 4], line_color="green", line_width=2)
        plot3.line(x, temperature, legend_label="temperature", line_dash=[4, 4], line_color="blue", line_width=2)
        plot4.line(stepsx, steps, legend_label="steps", line_dash=[4, 4], line_color="blue", line_width=2)

        conn.commit()
        conn.close()
        nconn.commit()
        nconn.close()

        script2, div2 = components(plot2)
        script3, div3 = components(plot3)
        script4, div4 = components(plot4)

          
        if choice == "Velocity":
            return f'''<!DOCTYPE html>
              <html> <script src="https://cdn.bokeh.org/bokeh/release/bokeh-2.4.0.min.js"></script>
              <body>

                    <h1>Pick your plot...</h1>
                    <p>Boost your way to success</p>
                    <form action="http://608dev-2.net/sandbox/sc/team44/plots.py?user={current}" method="post">   
                      <label for="plot">I want to see my...</label>
            <select name="plot" id="plot">
          <option value="Velocity">Velocity</option>
          <option value="Altitude">Altitude</option>
          <option value="Step Count">Step Count</option>
          <option value="Temperature">Temperature</option>
            </select>
            <br>
            <br>
                      
                      <input type="submit">
                    </form>
<br>
<p>Current Plot: {choice}</p>
                    </body>
                    
                <body>

  <div id="myplot"></div>

  <script>
    const mainUrl="http://608dev-2.net/sandbox/sc/team44/map/main.py?velocity-json=Joe";

  fetch(mainUrl)
    .then(function(response) {{ return response.json(); }})
    .then(function(item) {{ return Bokeh.embed.embed_item(item); }})
  </script>
</body>
   
              </html>
              ''' 

        #{current}
        elif choice == "Altitude":
            return f'''<!DOCTYPE html>
              <html> <script src="https://cdn.bokeh.org/bokeh/release/bokeh-2.4.0.min.js"></script>
              <body>

                    <h1>Pick your plot...</h1>
                    <p>Boost your way to success</p>
                    <form action="http://608dev-2.net/sandbox/sc/team44/plots.py?user={current}" method="post">   
                      <label for="plot">I want to see my...</label>
            <select name="plot" id="plot">
     <option value="Velocity">Velocity</option>
     <option value="Altitude">Altitude</option>
     <option value="Step Count">Step Count</option>
     <option value="Temperature">Temperature</option>
            </select>
            <br>
            <br>
                      
                      <input type="submit">
                    </form>

<br>
<p>Current Plot: {choice}</p>
                    </body>

    
              <body>
                  {div2}
              </body>
                  {script2}
     
              </html>
              ''' 
        elif choice == "Step Count":
            return f'''<!DOCTYPE html>
              <html> <script src="https://cdn.bokeh.org/bokeh/release/bokeh-2.4.0.min.js"></script>
              <body>

                    <h1>Pick your plot...</h1>
                    <p>Boost your way to success</p>
                    <form action="http://608dev-2.net/sandbox/sc/team44/plots.py?user={current}" method="post">   
                      <label for="plot">I want to see my...</label>
            <select name="plot" id="plot">
     <option value="Velocity">Velocity</option>
     <option value="Altitude">Altitude</option>
     <option value="Step Count">Step Count</option>
     <option value="Temperature">Temperature</option>
            </select>
            <br>
            <br>
                      
                      <input type="submit">
                    </form>

<br>
<p>Current Plot: {choice}</p>
                    </body>

    
              <body>
                  {div4}
              </body>
                  {script4}
     
              </html>
              ''' 
        else:
            return f'''<!DOCTYPE html>
              <html> <script src="https://cdn.bokeh.org/bokeh/release/bokeh-2.4.0.min.js"></script>
              <body>

                    <h1>Pick your plot...</h1>
                    <p>Boost your way to success</p>
                    <form action="http://608dev-2.net/sandbox/sc/team44/plots.py?user={current}" method="post">   
                      <label for="plot">I want to see my...</label>
            <select name="plot" id="plot">
      <option value="Velocity">Velocity</option>
      <option value="Altitude">Altitude</option>
      <option value="Step Count">Step Count</option>
      <option value="Temperature">Temperature</option>
            </select>
            <br>
            <br>
                      
                      <input type="submit">
                    </form>
<br>
<p>Current Plot: {choice}</p>
                    </body>

              <body>
                  {div3}
              </body>
                  {script3}
              </html>
              ''' 