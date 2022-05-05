import sqlite3
import datetime

# login_db = "/var/jail/home/team44/login.db" # contains name of user and time of last login
users_db = "/var/jail/home/team44/users.db"

users = []
loginpage = '''<!DOCTYPE html>
                <html>
                <head>
                <link rel="stylesheet" href="loginstyle.css">
                <script>
                function setCookie(event) {
                    alert("hi");
                    event.preventDefault();
                    const d = new Date();
                    d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));
                    let expires = "expires="+d.toUTCString();
                    cname = document.getElementById("user").value;
                    document.cookie = "user" + "=" + cname + ";" + expires + ";path=/";
                }
                </script>
                </head>
                <div id="loginbox">
                <h1>Sign in to get started!</h1>
                <div id="infobox"> <p>Enter your username. Then, we'll show your stats :)</p></div>
                <form onsubmit="(e) => setCookie(e)" action="http://608dev-2.net/sandbox/sc/team44/intermediate_page.py" method="get">   
                <label for="user">Username:</label>
                <input type="text" id="user" name="user" />
                <input type="submit" name="Go!" id="user"/>
                </form>
                </div>
                </html>
            '''
# redirectpage = '''<!DOCTYPE html>
#                 <html>
#                 <meta http-equiv="refresh" content="0; URL=http://608dev-2.net/sandbox/sc/team44/plots.py?user={}" />
#                 </html>
#             '''.format(cookiename)
# cookiesjs = '''function setCookie(cname, cvalue, exdays) {
#                 const d = new Date();
#                 d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));
#                 let expires = "expires="+d.toUTCString();
#                 document.cookie = cname + "=" + cvalue + ";" + expires + ";path=/";
#                 }

#                 function getCookie(cname) {
#                 let name = cname + "=";
#                 let ca = document.cookie.split(';');
#                 for(let i = 0; i < ca.length; i++) {
#                     let c = ca[i];
#                     while (c.charAt(0) == ' ') {
#                     c = c.substring(1);
#                     }
#                     if (c.indexOf(name) == 0) {
#                     return c.substring(name.length, c.length);
#                     }
#                 }
#                 return "";
#                 }
# '''

# def create_login_database():
#    conn = sqlite3.connect(login_db)  # connect to that database (will create if it doesn't already exist)
#    c = conn.cursor()  # move cursor into database (allows us to execute commands)
#    c.execute('''CREATE TABLE IF NOT EXISTS login_table (user text, timing timestamp, isLoggedIn int);''') # run a CREATE TABLE command
#    conn.commit() # commit commands (VERY IMPORTANT!!)
#    conn.close() # close connection to database

def create_user_database():
   conn = sqlite3.connect(users_db)  # connect to that database (will create if it doesn't already exist)
   c = conn.cursor()  # move cursor into database (allows us to execute commands)
   c.execute('''CREATE TABLE IF NOT EXISTS user_table (user text);''') # run a CREATE TABLE command
   conn.commit() # commit commands (VERY IMPORTANT!!)
   conn.close() # close connection to database

def request_handler(request):
    # create_login_database()  #call the function!
    create_user_database()
    if request["method"] == "GET":
        # get the most recent login, see if they're still logged in
        # if still logged in, redirect to plots.py page
        # if not still logged in, just return the default website
        # conn = sqlite3.connect(login_db)
        # return "hello world"
        return '''
                <div>
                <form action="http://608dev-2.net/sandbox/sc/team44/login_w3.py" name="loginhelp" id="loginhelp" method="POST">
                </form>
                <script>
                console.log("hello");
                function getCookie(cname) {
                    let name = cname + "=";
                    let ca = document.cookie.split(';');
                    for(let i = 0; i < ca.length; i++) {
                        let c = ca[i];
                        while (c.charAt(0) == ' ') {
                        c = c.substring(1);
                        }
                        if (c.indexOf(name) == 0) {
                        return c.substring(name.length, c.length);
                        }
                    }
                    return "";
                }
                let cookieval = getCookie("username");
                var btnInput = document.createElement("input");
                btnInput.type = "hidden";
                btnInput.name = "username";
                btnInput.value = cookieval;
                document.forms["loginhelp"].appendChild(btnInput);
                document.forms["loginhelp"].submit();
                </script>
            </div>
                '''
                
    else:
        cookiename = request['form']['username']
        if(cookiename == ''):
            return loginpage
        redirectpage = '''<!DOCTYPE html>
                <html>
                <meta http-equiv="refresh" content="0; URL=http://608dev-2.net/sandbox/sc/team44/intermediate_page.py?user={}" />
                </html>
                <span>reached here!</span>
            '''.format(cookiename)
        return redirectpage
