
# Python program to find current
# weather details of any city
# using openweathermap api

import dash
import dash_core_components as dcc
import dash_html_components as html
from dash.dependencies import Input, Output, State
from openWeatherCred import *
import datetime
from pytz import timezone
import requests
import threading
import json
# from pytemperature import k2f as k2f
from sqlalchemy import create_engine, update
from sqlalchemy.orm import sessionmaker
from alchemy_sample import weatherEntry, Base

engine = create_engine('sqlite:///sqlalchemy_example.db', connect_args={'check_same_thread': False}, echo=False)
# Bind the engine to the metadata of the Base class so that the
# declaratives can be accessed through a DBSession instance
Base.metadata.bind = engine
 
DBSession = sessionmaker(bind=engine)
# A DBSession() instance establishes all conversations with the database
# and represents a "staging zone" for all the objects loaded into the
# database session object. Any change made against the objects in the
# session won't be persisted into the database until you call
# session.commit(). If you're not happy about the changes, you can
# revert all of them back to the last commit by calling
# session.rollback()
session = DBSession()

threadingEn = False
local = False
global timeInterval


def checkForcast():
    if(threadingEn):
        threading.Timer(60 * 60 * 3, checkForcast).start()
        print("Checking forcast every 3 hours")
    # Current weather and sunriseset 
    # http://api.openweathermap.org/data/2.5/weather?zip=32569,us&APPID=
    # Current api https://openweathermap.org/current
    base_url = "http://api.openweathermap.org/data/2.5/forecast?"
    complete_url = base_url + "id=4163599&APPID=" + apiKey + "&units=imperial"
    print("forcast url: ", complete_url)
    # forecast api https://openweathermap.org/forecast5#limit
    # &units=imperial
    if(not local):
        response = requests.get(complete_url)
        w = response.json()

    if(local):
        with open('forcast.json') as jsonText:
            w = json.load(jsonText)

    if w["cod"] == "200":
        for n in range(w["cnt"]):
            x = w["list"][n]
            utc = x['dt']
            # print("\nindex:" + str(n) + "\nDT: " + str(utc))
            # str(datetime.datetime.utcfromtimestamp(x['dt'])))
            # https://avilpage.com/2014/11/python-unix-timestamp-utc-and-their.html

            # store the value of "main"
            # key in variable y
            y = x["main"]
            # store the value corresponding
            # to the "temp" key of y
            current_temperature = y["temp"]
            # store the value corresponding
            # to the "pressure" key of y
            current_pressure = y["pressure"]
            # store the value corresponding
            # to the "humidity" key of y
            current_humidiy = y["humidity"]
            # store the value of "weather"
            # key in variable z
            z = x["weather"]
            # store the value corresponding
            # to the "description" key at
            # the 0th index of z
            weather_description = z[0]["description"]
            cloud_level = x["clouds"]["all"]
            if('rain' in x):
                if('3h' in x["rain"]):
                    rain3h = x["rain"]["3h"]
                if('1h' in x["rain"]):
                    rain1h = x["rain"]["1h"]
            date = datetime.datetime.strptime(x["dt_txt"], '%Y-%m-%d %H:%M:%S')
            # print following values
            # print("Date = " +
            #       str(date.strftime('%B, %d %Y %I:%M%p')) +
            #       "\n Temperature (F) = " +
            #       str(k2f(current_temperature)) +
            #       "\n atmospheric pressure (in hPa unit) = " +
            #       str(current_pressure) +
            #       "\n humidity (in percentage) = " +
            #       str(current_humidiy) +
            #       "\n description = " +
            #       str(weather_description))
            # update https://code-maven.com/slides/python-programming/orm-update
            existing = session.query(weatherEntry).filter(weatherEntry.unixtime==utc)
            # if exists update
            if(existing.scalar()):
            # update
                existing = session.query(weatherEntry).filter(weatherEntry.unixtime==utc).one()
                existing.desc = weather_description
                existing.temp = current_temperature
                existing.press = current_pressure
                existing.hum = current_humidiy
                existing.cloudy = cloud_level
            else:
                print("New entry:" + str(datetime.datetime.utcfromtimestamp(utc)))
                entry = weatherEntry(unixtime=utc, desc=weather_description, temp=current_temperature, press=current_pressure, hum=current_humidiy, cloudy=cloud_level)
                session.add(entry)
            session.commit()
    else:
        print("Request fail code:" + str(w["cod"]))


def checkCurrent():
    # Current weather and sunriseset 
    # Current api https://openweathermap.org/current
    complete_url = "http://api.openweathermap.org/data/2.5/weather?id=4163599&APPID=" + apiKey + "&units=imperial"
    # forecast api https://openweathermap.org/forecast5#limit
    # &units=imperial
    if(not local):
        response = requests.get(complete_url)
        w = response.json()

    if(local):
        with open('current.json') as jsonText:
            w = json.load(jsonText)
    if w['cod'] == 200:
        curTemp = w["main"]["temp"]
        curHum = w["main"]["humidity"]
        curClouds = w["clouds"]["all"]
        curHi = w["main"]["temp_max"]
        heatFactor = curHi/(curClouds/100)
        utc = datetime.datetime.utcfromtimestamp(w['dt']).replace(tzinfo=timezone('UTC'))
        #dt = datetime.datetime.utcfromtimestamp(utc)
        # round_to_hour(utc).timestamp()
        return (round_to_hour(utc.timestamp()).timestamp(), curTemp, curHum, heatFactor, curClouds, curHi)
    elif w['cod']:
        print("Request fail code:" + str(w["cod"]))
    else:
        print("Bad or no weather response")



def round_to_hour(dt):
    round_delta = 60 * 30
    round_timestamp = dt + round_delta
    round_dt = datetime.datetime.fromtimestamp(round_timestamp).replace(tzinfo=timezone('US/Central'))
    return round_dt.replace(microsecond=0, second=0, minute=0)


def nearest(items, pivot):
    return min(items, key=lambda x: abs(x - pivot))


def updateCurrent():
    if(threadingEn):
        threading.Timer(60 * 60 * 1, updateCurrent).start()
        print("Updating current status hourly")
    utc, curTemp, curHum, heatFactor, clouds, max = checkCurrent()

    entries = session.query(weatherEntry).all()
    times = []
    for n in entries:
        times.append(n.unixtime)

    nowTime = nearest(times, utc)

    existing = session.query(weatherEntry).filter(weatherEntry.unixtime==nowTime)
    if(existing.scalar()):
        a = existing.one()
        # update
        a.temp=curTemp
        a.hum=curHum
        session.commit()
        # print(utc, " was updated into closestTime ", nowTime, curTemp, curHum)
    else:
        print("Bad nearest, update recent failed")
    

external_stylesheets = ['https://codepen.io/chriddyp/pen/bWLwgP.css']
app = dash.Dash(__name__, external_stylesheets=external_stylesheets)


app.layout = html.Div(children=[
    dcc.Location(id='url', refresh=False),
    html.H1(children='Local Weather'),

    html.Div(children='''
        From Openweathermap.org.
    '''),
    html.Div(id='information',children=""),
    dcc.DatePickerRange(
        id='my-date-picker-range',
        min_date_allowed=datetime.datetime(2019, 7, 16),
        max_date_allowed=datetime.datetime(2019, 7, 24),
        initial_visible_month=datetime.datetime(2019, 7, 16),
        end_date=datetime.datetime(2019, 7, 19)
    ),
    html.Div(id='output-container-date-picker-range'),
    dcc.Graph(
        id='example-graph',
        figure={
            'data': [
                {'x': [], 'y': [], 'type': 'bar', 'name': 'Temperature'},
                {'x': [], 'y': [], 'type': 'point', 'name': 'Humidity'}
            ],
            'layout': {
                'title': 'Current and Forcast live updating'
            }
        }
    )
])
@app.callback(dash.dependencies.Output('information', 'children'),
[Input('url', 'pathname')])
def onLoad(test):
    print("Page load",test)
    utc, curTemp, curHum, heatFactor, clouds, maxT= checkCurrent()
    text = "Heat Factor: " + str(round(heatFactor)) + " (Max Temp F / Cloud %) ~" + str(maxT) + " / " + str(clouds) + "%"
    return text

@app.callback(
    dash.dependencies.Output('example-graph', 'figure'),
    [dash.dependencies.Input('my-date-picker-range', 'start_date'),
     dash.dependencies.Input('my-date-picker-range', 'end_date')])
def update_output(start_date, end_date):
    if start_date is not None and end_date is not None:
        startTime = int(datetime.datetime.strptime(start_date[:10], '%Y-%m-%d').timestamp())
        endTime =  int((datetime.datetime.strptime(end_date[:10], '%Y-%m-%d') + datetime.timedelta(days=1)).timestamp())

        entries = session.query(weatherEntry).filter(weatherEntry.unixtime >= startTime,weatherEntry.unixtime < endTime).all()
        time = []
        temp = []
        humidity = []
        # utcfromts()..strftime('%B, %d %I %p')
        for n in entries:
            time.append(datetime.datetime.fromtimestamp(n.unixtime).strftime('%B, %d %I %p'))
            temp.append(n.temp)
            humidity.append(n.hum)
        return {
            'data': [
                {'x': time, 'y': temp, 'type': 'bar', 'name': 'Temperature'},
                {'x': time, 'y': humidity, 'type': 'point', 'name': 'Humidity'}
            ],
            'layout': {
                'title': 'Current and Forcast live updating',
                'zeroline': 'False',
                'legend': {'x':.8, 'y':1.2},
                'autorange': 'true',
                'yaxis': {'range':[70,100],
                    'tickfont': {'family':'Rockwell', 'size':25},
                    'xaxis': {
                            'tickfont': {'family':'Rockwell', 'size':25}
                    }
                }
            }
        }
    else:
        startTime = int(datetime.datetime.today().timestamp())
        endTime =  int((datetime.datetime.today() + datetime.timedelta(hours=16)).timestamp())
        entries = session.query(weatherEntry).filter(weatherEntry.unixtime >= startTime,weatherEntry.unixtime < endTime).all()
        time = []
        temp = []
        humidity = []
        # utcfromts()..strftime('%B, %d %I %p')
        for n in entries:
            time.append(datetime.datetime.fromtimestamp(n.unixtime).strftime('%B, %d %I %p'))
            temp.append(n.temp)
            humidity.append(n.hum)
        return {
                'data': [
                    {'x': time, 'y': temp, 'type': 'bar', 'name': 'Temperature'},
                    {'x': time, 'y': humidity, 'type': 'point', 'name': 'Humidity'}
                ],
                'layout': {
                    'title': 'Current and Forcast live updating',
                    'legend': {'x':.8, 'y':1.2},
                    'zeroline': 'False',
                    'tickangle': '0',
                    'yaxis': {
                        'range':[70,100],
                        'dtick':5,
                        'tickfont': {'family':'Rockwell', 'size':25}
                    },
                    'xaxis': {
                        'tickfont': {'family':'Rockwell', 'size':18}
                    }
                }
            }


if __name__ == "__main__":
    app.run_server(debug=True, host='0.0.0.0', port="7859")
    updateCurrent()
    checkForcast()
