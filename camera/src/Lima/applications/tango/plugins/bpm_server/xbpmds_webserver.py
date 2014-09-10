import os
import gevent
import gevent.event
import gevent.queue
import gevent.server
import bottle
import socket
import json
import time
from bottle.ext.websocket import GeventWebSocketServer
from bottle.ext.websocket import websocket

HOMEPAGE_TITLE = "BPM Monitor"
WEB_QUERIES = None 

# patch socket module
socket.socket._bind = socket.socket.bind
def my_socket_bind(self, *args, **kwargs):
  self.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  return socket.socket._bind(self, *args, **kwargs)
socket.socket.bind = my_socket_bind  

def find_free_port():
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.bind(('',0))
  _, port = s.getsockname()
  s.close()
  return port

def get_query():
  global WEB_QUERIES
  if WEB_QUERIES is None:
      WEB_QUERIES = gevent.queue.Queue()
  return WEB_QUERIES.get()

def query(name, **kwargs):
    reply = {}
    event = gevent.event.Event()
    query = { "query": name, "reply": reply, "event": event }
    query.update(kwargs)   
    print "putting query in queue:",query
    WEB_QUERIES.put(query)
    event.wait()
    return reply    

def webserver(port=None, home_title=None):
  if port is None:
    port = find_free_port()
  else:
    # check if port number is free
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
      s.bind(('', port))
    except: 
      port = find_free_port()
    s.close()

  webserver_thread = gevent.spawn(bottle.run, server=GeventWebSocketServer, host="", port=port, monkey=False, quiet=True)
  webserver_thread.port = port
  if home_title:
      global HOMEPAGE_TITLE
      HOMEPAGE_TITLE = home_title
  return webserver_thread

@bottle.route('/')
def bpm_monitor_application():
  homepage = open(os.path.join(os.path.dirname(__file__), "bpm_monitor.html"), "r")
  return homepage.read() % HOMEPAGE_TITLE
  #return bottle.static_file("bpm_monitor.html", root=os.path.dirname(__file__))

@bottle.route('/:filename')
def send_static(filename):
  return bottle.static_file(filename, root=os.path.dirname(__file__))

@bottle.get('/get_bpms')
def get_bpms():
  return query("get_bpms")

@bottle.get('/get_status')
def get_status():
  return query("get_status", client_id=bottle.request.GET["client_id"])

@bottle.get("/set_roi")
def set_roi():
  res = query("set_roi", client_id=bottle.request.GET["client_id"], 
                          x=int(bottle.request.GET["x"]),
                          y=int(bottle.request.GET["y"]),
                          w=int(bottle.request.GET["w"]),
                          h=int(bottle.request.GET["h"]))
  return res

@bottle.get("/get_beam_position")
def acquire():
  return query("get_beam_position", client_id=bottle.request.GET["client_id"], 
                                    exp_t=bottle.request.GET["exp_t"], 
                                    live = bottle.request.GET["live"],
                                    do_bpm = bottle.request.GET["do_bpm"],
                                    acq_rate = bottle.request.GET["acq_rate"])

@bottle.get("/img_display_config")
def set_config():
  return query("set_img_display_config", client_id=bottle.request.GET["client_id"],
                                         autoscale=bottle.request.GET["autoscale"],
                                         color_map=bottle.request.GET["color_map"],
                                         lut_method=bottle.request.GET["autoscale_option"])

@bottle.get("/update_calibration")
def update_calib():
  return query("update_calibration", client_id=bottle.request.GET["client_id"],
                                     calib_x=bottle.request.GET["x"],
                                     calib_y=bottle.request.GET["y"],
                                     save=bottle.request.GET["save"])

@bottle.get("/lock_beam_mark")
def lock_beam_mark():
  return query("lock_beam_mark", client_id=bottle.request.GET["client_id"],
                                 x=bottle.request.GET["x"],
                                 y=bottle.request.GET["y"])
                                 

@bottle.get("/get_intensity")
def get_intensity():
  return query("get_intensity", client_id=bottle.request.GET["client_id"],
                                x = bottle.request.GET["x"],
                                y = bottle.request.GET["y"])


@bottle.get("/set_background")
def set_background():
  return query("set_background", client_id=bottle.request.GET["client_id"],
                                 set = bottle.request.GET["set"])


@bottle.get('/image_channel', apply=[websocket])
def provide_images(ws):
  while True:
    client_id = ws.receive()
    if client_id is not None:
      qres = query("new_image", client_id=client_id)
      tosend = json.dumps(qres)
      print "sending",len(tosend), "bytes"
      ws.send(tosend)
    else: break
 
@bottle.get('/ext_changes_channel', apply=[websocket])
def set_config(ws):
  while True:
    client_id = ws.receive()
    if client_id is not None:
      qres = query("external_changes", client_id=client_id)
      tosend = json.dumps(qres)
      print "changes from Tango server side:", qres
      ws.send(tosend)
    else: break


