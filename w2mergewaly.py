import sqlite3
import datetime
from bokeh.plotting import figure
from bokeh.embed import components
#script is meant for local development and experimentation with bokeh
example_db = "/var/jail/home/team44/skweek1.db" #database has table called sensor_data with entries: time_ timestamp, user text, temperature real, pressure real
# example_db = "example.db"
USERS = ["toy_user"] # eventually, the "sign in" page on the website will add users to this list

<<<<<<< HEAD

=======
>>>>>>> parent of 9551acd (Update w2mergewaly.py)
def request_handler(request):
    now = datetime.datetime.now()
    conn = sqlite3.connect(example_db)
    c = conn.cursor()
    plot1 = figure()
    plot2 = figure()
    plot3 = figure()
    x = [[]]
    pressure = [[]]
    temperature = [[]]
    altitude = [[]]
    for i in range(len(USERS)):
        print("New User")
        # things = c.execute('''SELECT pres, alt, temp, timing FROM sk_table WHERE user = ? ORDER by timing ASC;''',(USERS[i],)).fetchall()
        things = c.execute('''SELECT pres, alt, temp, timing FROM sk_table ORDER by timing ASC;''').fetchall()
        print(things)
        for row in things:
            # print(row)
            if row[0] is None or row[1] is None or row[2] is None or row[3] is None:
                raise Exception(f"{row = }")
            pressure[i].append(float(row[0]))
            temperature[i].append(float(row[2]))

            altitude[i].append(float(row[1]))
            dto = datetime.datetime.strptime(row[3],'%Y-%m-%d %H:%M:%S.%f')
            print("dto: ", dto)
            x[i].append(dto)
    # print("pressure: ", pressure)
    # print("temperature: ", temperature)
    # print("x: ", x)
    plot1.xaxis.axis_label = "time (sec)"
    plot2.xaxis.axis_label = "time (sec)"
    plot3.xaxis.axis_label = "time (sec)"
    plot1.yaxis.axis_label = "pressure (Pa)"
    plot2.yaxis.axis_label = "altitude (m)"
    plot3.yaxis.axis_label = "temperature (F)"
    plot1.line(x[0], pressure[0], legend_label="pressure", line_dash=[4, 4], line_color="orange", line_width=2)
    plot2.line(x[0], altitude[0], legend_label="altitude", line_dash=[4, 4], line_color="green", line_width=2)
    plot3.line(x[0], temperature[0], legend_label="temperature", line_dash=[4, 4], line_color="blue", line_width=2)



    conn.commit()
    conn.close()

    #either show plot1 or plot2
    # show(plot1)
    # show(plot2)
    script1, div1 = components(plot1)
    script2, div2 = components(plot2)
    script3, div3 = components(plot3)
    if request['method'] =="GET":
        return '''<!DOCTYPE html>
        <html>
        <body>

        <h1>Pick your plot...</h1>
        <p>Boost your way to success</p>
        <form action="http://608dev-2.net/sandbox/sc/team44/walyw2demo.py" method="post">   
          <label for="plot">I want to see my...</label>
<select name="plot" id="plot">
    <option value="pres">Pressure</option>
  <option value="alt">Altitude</option>
  <option value="temp">Temperature</option>
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
        choice = request["form"]["plot"]
          
        if choice == "pres":
            return f'''<!DOCTYPE html>
              <html> <script src="https://cdn.bokeh.org/bokeh/release/bokeh-2.4.0.min.js"></script>
              <body>

                    <h1>Pick your plot...</h1>
                    <p>Boost your way to success</p>
                    <form action="http://608dev-2.net/sandbox/sc/team44/walyw2demo.py" method="post">   
                      <label for="plot">I want to see my...</label>
            <select name="plot" id="plot">
                <option value="pres">Pressure</option>
              <option value="alt">Altitude</option>
              <option value="temp">Temperature</option>
            </select>
            <br>
            <br>
                      
                      <input type="submit">
                    </form>
<br>
<p>Current Plot: {choice}</p>
                    </body>
                    
                
              <body>
                  {div1}
              </body>
                  {script1}
   
              </html>
              ''' 
        elif choice == "alt":
            return f'''<!DOCTYPE html>
              <html> <script src="https://cdn.bokeh.org/bokeh/release/bokeh-2.4.0.min.js"></script>
              <body>

                    <h1>Pick your plot...</h1>
                    <p>Boost your way to success</p>
                    <form action="http://608dev-2.net/sandbox/sc/team44/walyw2demo.py" method="post">   
                      <label for="plot">I want to see my...</label>
            <select name="plot" id="plot">
                <option value="pres">Pressure</option>
              <option value="alt">Altitude</option>
              <option value="temp">Temperature</option>
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
        else:
            return f'''<!DOCTYPE html>
              <html> <script src="https://cdn.bokeh.org/bokeh/release/bokeh-2.4.0.min.js"></script>
              <body>

                    <h1>Pick your plot...</h1>
                    <p>Boost your way to success</p>
                    <form action="http://608dev-2.net/sandbox/sc/team44/walyw2demo.py" method="post">   
                      <label for="plot">I want to see my...</label>
            <select name="plot" id="plot">
                <option value="pres">Pressure</option>
              <option value="alt">Altitude</option>
              <option value="temp">Temperature</option>
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
  

  
    
  
    
  
#.format(div1, script1, div2, script2, div3, script3)