from flask import Flask
from flask import request
import datetime
import sqlite3
import datetime as DT

app = Flask(__name__)

conn = sqlite3.connect('time_db.sqlite')
#sqlite3.enable_callback_tracebacks(True)
c = conn.cursor()

# Wemos Battery and BMS Temp Readings
c.execute('''
    CREATE TABLE
    IF NOT EXISTS test (timecol text NOT NULL, chipname varchar(250) NOT NULL, friendlyName varchar(250) NOT NULL, temp REAL NOT NULL)''')

# Solar Controller Responses
c.execute('''
    CREATE TABLE
    IF NOT EXISTS solarCont (timecol text NOT NULL, temp REAL NOT NULL, battvolts REAL NOT NULL,
                            loadpwr REAL NOT NULL, loadcur REAL NOT NULL, solarvolts REAL NOT NULL,
                            solarcur REAL NOT NULL, solarpow REAL NOT NULL, battremain REAL NOT NULL,
                            batttemp REAL NOT NULL)''')

# Solar Controller Responses
c.execute('''
    CREATE TABLE
    IF NOT EXISTS batteryTrack (timecol text NOT NULL, bank1volts REAL NOT NULL, bank2volts REAL NOT NULL,
                            bank1perc REAL NOT NULL, bank2perc REAL NOT NULL)''')
# House Log
# houseTrack (timecol, sensor, temp)
c.execute('''
    CREATE TABLE
    IF NOT EXISTS houseTrack (timecol text NOT NULL, intake REAL NOT NULL, output REAL NOT NULL, diff REAL NOT NULL )''')
conn.commit()
conn.close()

lookupNames = {
            '28 ff b1 c2 6b 18 3 e8 ':"BMS#1",
             '28 ff db 2e 78 18 1 d ':"Bank1,1,2",
            '28 ff 51 9d 77 18 1 3d ':"Bank1,2,3",
            '28 ff 46 cd 6b 18 3 85 ':"Bank1,3,4",
               
               '28 ff b1 7 6c 18 3 2a ':"BMS#2",
               '28 ff ae d3 77 18 1 1 ':"Bank2,1,2",
              '28 ff 2e 76 77 18 1 e2 ':"Bank2,2,3",
               '28 ff 4f 62 77 18 1 8 ':"Bank2,3,4",
               }

@app.route('/housePostJSON', methods = ['POST'])
def housepostJsonHandler():
    currentDT = datetime.datetime.now()

    content = request.get_json()

    intemp = content['Temperature'][0]
    outtemp = content['Temperature'][1]
    diff = content['Temperature'][2]
    print("Intake:",intemp,"output:",outtemp)
    con = sqlite3.connect("time_db.sqlite")
    cur = con.cursor()

    query = "insert into houseTrack (timecol, intake, output, diff) values (?,?,?,?)"
    values = (currentDT, intemp, outtemp, diff)


    cur.executemany(query, (values,))
    con.commit()
    con.close()
    return 'House JSON posted'

@app.route('/postjson', methods = ['POST'])
def postJsonHandler():
    content = request.get_json()
    currentDT = datetime.datetime.now()
    con = sqlite3.connect("time_db.sqlite")
    cur = con.cursor()
    name = content['Sensor'][0] # key error if bad post
    temp = content['Temperature'][0]
    print(lookupNames[name], " is ",temp)
    query = "insert into test (timecol, chipname, friendlyName, temp) values (?,?,?,?)"
    values = (currentDT,name, lookupNames[name],temp)
    cur.executemany(query, (values,))

    # print (currentDT,content,"\n")
    con.commit()
    con.close()
    return 'Temperature JSON posted '+lookupNames[name]

@app.route('/solarPostJSON', methods = ['POST'])
def solarpostJsonHandler():
    content = request.get_json()
    currentDT = datetime.datetime.now()
    con = sqlite3.connect("time_db.sqlite")
    cur = con.cursor()
    
    query = "insert into solarCont (timecol, temp, battvolts, loadpwr, loadcur, solarvolts, solarcur, solarpow, battremain, batttemp) values (?,?,?,?,?,?,?,?,?,?)"
    values = (currentDT, content['ContTemp'], content['BattVolts'], content['LoadPwr'],
             content['LoadCur'], content['SolarVolts'], content['SolarCur'], content['SolarPwr'],
             content['BattRemain'], content['BattTemp'])

    cur.executemany(query, (values,))
    con.commit()
    con.close()
    return 'Controller JSON posted'

@app.route('/voltagePOSTJSON', methods = ['POST'])
def voltagePOSTJSONHandler():
    content = request.get_json()
    currentDT = datetime.datetime.now()
    con = sqlite3.connect("time_db.sqlite")
    cur = con.cursor()
    
    query = "insert into batteryTrack (timecol, bank1volts, bank2volts, bank1perc, bank2perc) values (?,?,?,?,?)"
    values = (currentDT, content['Voltages'][0], content['Voltages'][1], content['Perc'][0], content['Perc'][1])

    cur.executemany(query, (values,))
    con.commit()
    con.close()
    return 'Battery Voltage JSON posted ' + str(currentDT)

def takeTemp(elem):
    return elem[3]

@app.route('/BatteryMonitor', methods = ['GET'])
def housegetJsonHandler():
    con = sqlite3.connect("time_db.sqlite")
    cur = con.cursor()
    query = "SELECT * FROM test WHERE timecol >= datetime('now', '-2 minutes', 'localtime') ORDER by temp desc"
    cur.execute(query)
    tempresults = cur.fetchall()
    tempresults.sort(key=takeTemp)
    query = "SELECT * FROM batteryTrack WHERE timecol >= datetime('now', '-6.1 minutes', 'localtime')"
    cur.execute(query)
    voltsresults = cur.fetchall()
    output = ""
    if (len(voltsresults) > 0 ):
        # Average Pack temperature & Average BMS Temperature
        batterys = "<p>At " + str(voltsresults[-1][0]) + "</p><p>Bank 1: " + str(voltsresults[-1][1]) + "Volts " + str(voltsresults[-1][3]) +"%</p><p>Bank 2: " + str(voltsresults[-1][2]) + "Volts " + str(voltsresults[-1][4]) +"%</p>"
        output += batterys
    elif(len(tempresults) > 0):
        temps = "<p>The highest temperature read is:" + str(takeTemp(tempresults[-1])) + " on sensor " + str(tempresults[-1][2]) + " at " + str(tempresults[-1][0]) + "\n" + batterys + "</p>"
        output += temps
    else:
        output = "Missing recent data"
    con.close()
    return output

@app.route('/postjson', methods = ['GET'])
def getJsonHandler():
    #content = request.get_json()
    #print (content)
    return 'Hey Get Request'

if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=True)