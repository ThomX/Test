import threading
import os
import fcntl
import sys
import time
import types
import gevent
import logging
import weakref
import Queue

if not hasattr(gevent, "wait"):
  def gevent_wait(timeout=None):
    return gevent.run(timeout)
  gevent.wait=gevent_wait

inq = Queue.Queue()
outq = Queue.Queue()
objects_thread = None
exported_objects = weakref.WeakValueDictionary()
read_event_watcher = None

def process_requests(inq, outq):
  devices = {}

  def deal_with_job(job, args, kwargs):
    if job == "new":
      klass = args[0]
      new_device = klass(*args[1:], **kwargs)
      devices[id(new_device)]=new_device
      inq.put(id(new_device))
    elif job == "exit":
      sys.exit(0)
    else:
      device = devices[args[0]]
      method = getattr(device, job)

      try:
        result = method(*args[1:], **kwargs)
      except:
        # to do: if result is an exception, transmit it
        logging.exception("Cannot execute job %s", job)
        result = None
      inq.put(result) 

  def read_from_queue():
    job, args, kwargs = outq.get() 
    logging.info("received new job %r(%r, %r)", job, args, kwargs)
    deal_with_job(job, args, kwargs)

  global read_event_watcher
  read_event_watcher = gevent.get_hub().loop.async() 
  read_event_watcher.start(read_from_queue)

  gevent.wait()

class call_from_tango:
  def __init__(self, obj_id, method):
    self.obj_id = obj_id
    self.method = method
  def __call__(self, *args, **kwargs):
    outq.put((self.method, [self.obj_id]+list(args), kwargs))
    read_event_watcher.send()
    result = inq.get()
    logging.info("received from object: %r", result)
    return result


class objectProxy:
  @staticmethod
  def exit():
    outq.put(("exit", [None], {}))
    read_event_watcher.send()

  def __init__(self, obj_id, ):
    self.obj_id = obj_id

  def __getattr__(self, attr):
    # to do: replace getattr by proper introspection
    # to make a real proxy
    return call_from_tango(self.obj_id, attr)


def TangoBridge(object_class, *args, **kwargs):
  """Instanciate new object from given class in a separate thread"""
  global objects_thread

  if objects_thread is None:
    objects_thread = threading.Thread(target=process_requests, args=(inq, outq,))
    objects_thread.start()
  
  time.sleep(0.1) #dummy way of synchronizing with thread start
  outq.put(("new", [object_class]+list(args), kwargs))
  read_event_watcher.send()
  obj_id = inq.get()
  new_object = objectProxy(obj_id)
  exported_objects[obj_id]=new_object
  return new_object
