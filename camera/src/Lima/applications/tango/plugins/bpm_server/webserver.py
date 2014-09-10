#!/usr/bin/env python
import bottle
import PyTango
import sys
import threading
import time

def start_webserver(port_number):
    global beam_viewer_servers
    server = bottle.WSGIRefServer(host="", port=port_number)
    default_app = bottle.Bottle()

    @default_app.route('/')
    def applications_monitor():
        reply = "<html><body><h1>ESRF XBPM applications</h1><h3>1. Beam viewers</h3><ul>"
        beam_viewer_servers = get_beam_viewers_dict()
        for bv_server_name, bv_devices_list in beam_viewer_servers.iteritems():
          reply += "<li><a href='http://%s'>%s</a><ul>" % (bv_devices_list[0][1], bv_server_name)
          for bv_device_name, bv_application_address in bv_devices_list:
            try:
              PyTango.DeviceProxy(bv_device_name).ping()  
            except:
              color = "red"
            else:
              color = "green"
            reply += "<li><font color='%s'>%s</font></li>" % (color, bv_device_name)
          reply += "</ul>"
        reply += "</ul></body></html>"
        return reply
    @default_app.route('/device/<dev:path>')
    def redirect_to_device_server(dev):
        beam_viewer_servers = get_beam_viewers_dict()
        for bv_server_name, bv_devices_list in beam_viewer_servers.iteritems():
          for bv_device_name, bv_application_address in bv_devices_list:
            if dev == bv_device_name:
              return """<html><script language="JavaScript">document.location.href="http://%s"</script></html>""" % bv_application_address

    applications_webserver = threading.Thread(target=server.run, args=(default_app,))

    def launch_webserver():
      try:
        applications_webserver.start()
      except:
        pass

    webserver_start_timer = threading.Timer(1, launch_webserver)
    webserver_start_timer.start()

def get_beam_viewers_dict():
  # make the list of bpm servers and find port numbers
  tango_db = PyTango.DeviceProxy("sys/database/2")
  beam_viewers = tango_db.DbGetDeviceList(["*", "BeamViewer"])
  beam_viewer_servers = {}

  for bv_device_name in beam_viewers:
      try:
        prop = tango_db.DbGetDeviceProperty([bv_device_name, "port"])
        port_number = int(prop[-1])
        device_info = tango_db.DbGetDeviceInfo(bv_device_name)
        host = device_info[1][4]
        #prop = tango_db.DbGetDeviceProperty([bv_device_name, "display_group"])
        #group_name = str(prop[-1])
        #if group_name != ' ':
        #    server = group_name 
        #else:
        if True:
            server = device_info[1][3]
        beam_viewer_servers.setdefault(server, []).append((bv_device_name, "%s:%d" % (host, port_number)))
      except:
        pass

  return beam_viewer_servers


if __name__ == '__main__': 
  try:
    port_number = int(sys.argv[1])
  except:
    port_number = 8066

  start_webserver(port_number) 
