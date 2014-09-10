import sys
import gevent
import gevent.event
import gevent.monkey; gevent.monkey.patch_all(thread=False,subprocess=False)
import os
sys.path.insert(0, os.path.dirname(__file__))
os.environ["QUB_SUBPATH"]="qt4"
from Qub.CTools import pixmaptools #for 16-bits to 8-bits conversion
import xbpmds_webserver
import time
import Image
import base64
#import msgpack
import cStringIO
import threading
import numpy
import math
import logging

BEAM_VIEWERS = {}
webserver = None
webserver_queries_handling = None
default_colormap = """
0 0 0
0 0 1
1 0 1
1 0 0
1 1 0
1 1 1
"""

def palette(colormap=None):
    if colormap is None:
      colormap = default_colormap
    npa = numpy.array([255*float(i) for i in colormap.split()])
    size = npa.size / 3
    npb = npa.reshape((size, 3))
    steps = [ float(i) / (size - 1) for i in range(size)]
    r = lambda x:numpy.interp(x, steps, npb[:, 0])
    g = lambda x:numpy.interp(x, steps, npb[:, 1])
    b = lambda x:numpy.interp(x, steps, npb[:, 2])
    palette = [ (int(r(i / 255.)), int(g(i / 255.)), int(b(i / 255.))) for i in range(256)]
    palette = map(lambda a: chr(a[0]) + chr(a[1]) + chr(a[2]), palette)
    return "".join(palette)
        

class NoBpmResult:
  def __init__(self):
    self.frameNumber = 0
    self.errorCode = "ERROR"


def handle_new_image(bv, query):
    bv._new_image_event.clear()
    bv._new_image_event.wait()

    query["reply"].update(bv.last_image)
    query["event"].set()


def handle_external_changes(bv, query):
    bv._ext_change_event.wait()

    query["reply"].update({"exp_t": bv.ccd_exposure_time,
                           "in_acquisition": bv.in_acquisition(),
                           "background": bv._has_background() })

    bv._ext_change_event.clear()

    query["event"].set()


def handle_webserver_queries():
    while True:
        print "*"*50
        query = xbpmds_webserver.get_query() 
        print "processing query", query

        if query["query"]=="get_bpms":
          all_bpms = []
          sorted_bpm_list = []
          for key in BEAM_VIEWERS.keys():
            bv = BEAM_VIEWERS[key]
            sorted_bpm_list.append((bv.get_name(), key))
          sorted_bpm_list.sort()
          for name, key in sorted_bpm_list:
            all_bpms.append({"name":name, "type": "bv", "key":key })
          query["reply"]["all_bpms"] = all_bpms 
          query["event"].set()
          continue

        client_id = int(query["client_id"]) 

        try:
          bv = BEAM_VIEWERS[client_id]
        except KeyError:
          continue

        #print "received query", query, 'from', client_id

        if query["query"] == "new_image":
           gevent.spawn(handle_new_image, bv, query)
        elif query["query"] == "external_changes":
           # this is the query used to communicate user changes through Tango to web app.
           gevent.spawn(handle_external_changes, bv, query)
        elif query["query"] == "get_beam_position":
           bv._set_acquisition_parameters(query)

           if bool(int(query["live"])):
              # set live
              bv.acquire_live(1, from_web=True)
           else:
              if bv.live:
                bv.acquire_live(0, from_web=True)
              else:
                bv.get_position(from_web=True)

           query["event"].set()
        elif query["query"] == "get_status":
           query["reply"].update({ "exposure_time": bv.ccd_exposure_time,
                                   "live": bv.live, 
                                   "roi": bv.has_roi, 
                                   "full_width": bv.max_width / 2, 
                                   "full_height": bv.max_height / 2, 
                                   "acq_rate": bv._get_acquisition_rate(), 
                                   "bpm_on": bv.do_bpm, 
                                   "color_map": bv.color_map==pixmaptools.LUT.Palette.TEMP, 
                                   "autoscale": bv.autoscale,
                                   "calib_x": bv.calib_x,
                                   "calib_y": bv.calib_y,
                                   "background": bv._has_background(),
                                   "beam_mark_x": bv.bm_x,
                                   "beam_mark_y": bv.bm_y })
           query["event"].set()
        elif query["query"] == "set_roi":
           try:
             bv._set_roi(query["x"],query["y"],query["w"],query["h"])
           except:
             logging.exception("Could not set roi")
           else:
             pass #print "roi is set"  
           query["event"].set()
        elif query["query"] == "set_img_display_config":
           bv.color_map = bool(int(query["color_map"])) and pixmaptools.LUT.Palette.TEMP or pixmaptools.LUT.Palette.GREYSCALE
           bv.autoscale = bool(int(query["autoscale"]))
           if query["lut_method"] == "Logarithmic":
             bv.lut_method = pixmaptools.LUT.LOG
           else:
             bv.lut_method = pixmaptools.LUT.LINEAR
           query["event"].set()
        elif query["query"] == "update_calibration":
           if bool(int(query["save"])):
             bv._save_calibration(float(query["calib_x"]), float(query["calib_y"]))
           else:
             bv._update_calibration(float(query["calib_x"]), float(query["calib_y"]))
           
           query["event"].set()
        elif query["query"] == "lock_beam_mark":
          bv.tg_device.LockBeamMark([float(query["x"]), float(query["y"])])
          bv.bm_x = float(query["x"])
          bv.bm_y = float(query["y"])
          query["event"].set()
        elif query["query"] == "get_intensity":
          x = int(query["x"]); y = int(query["y"])
          query["reply"].update({ "intensity": bv.get_intensity(x,y) })
          query["event"].set()
        elif query["query"] == "set_background": 
          if int(query["set"]):
            bv.take_background()
          else:
            bv.reset_background()
          query["event"].set()
          

class BeamViewer:
    def __init__(self, ccd_ip, ccd_exposure_time=0.01, acquisition_rate=10, orientation=0, calib_x=1, calib_y=1, beam_x=0, beam_y=0, ccd_control=None, name=None, tango_ds_name=None):
        # need to import LiMA
        self.Lima = __import__("Lima", globals(), locals(), ["Core"]) #, "Basler"])

	if name is None:
          name = "BeamViewer%d" % id(self)

        if tango_ds_name is not None:
          # in case we are called from Tango, to inform Tango about state changes
          # yes, I know this is ugly...
          # ... but there is no other solution AFAIK
          PyTango = __import__("PyTango", globals(), locals(), [])
          self.tg_device = PyTango.DeviceProxy(tango_ds_name)
        else:
          self.tg_device = None

        self.name = name
        self.acquisition_task = None
        self.bpm_result_event = threading.Event()
        self.ccd_ip = ccd_ip
        self.ccd_exposure_time = ccd_exposure_time
        self.orientation = orientation
        self.acquisition_rate = acquisition_rate
        self._new_image_event = gevent.event.Event()
        self.set_image_event_watcher = gevent.get_hub().loop.async()
        self.set_image_event_watcher.start(self._new_image_event.set)
        self._ext_change_event = gevent.event.Event()
        self.calib_x = calib_x
        self.calib_y = calib_y
        self.bm_x = beam_x
        self.bm_y = beam_y
        self.last_x = -1
        self.last_y = -1
        self.last_intensity = -1
        self.last_acq_time = -1
        self.last_fwhm_x = -1
        self.last_fwhm_y = -1
        self.last_max_intensity = -1
        self.last_profile_x = numpy.array([],dtype=numpy.int) 
        self.last_profile_y = numpy.array([],dtype=numpy.int)
        self.live = False 
        self.width = -1
        self.height = -1
        self.do_automatic_roi = True
        self.color_map = pixmaptools.LUT.Palette.TEMP
        self.autoscale = True
        self.lut_method = pixmaptools.LUT.LINEAR
        self.last_image = None
        self.raw_image_buffer = None
        color_palette =  pixmaptools.LUT.Palette(pixmaptools.LUT.Palette.TEMP)
        greyscale_palette = pixmaptools.LUT.Palette(pixmaptools.LUT.Palette.GREYSCALE)
        a = numpy.fromstring(color_palette.getPaletteData(), dtype=numpy.uint8)
        a.shape = (65536, 4)
        # BGR<=>RGB conversion
        r = numpy.array(a.T[2])
        b = numpy.array(a.T[0])
        a.T[0]=r; a.T[2]=b
        color_palette.setPaletteData(a)
        self.palette = { pixmaptools.LUT.Palette.TEMP: color_palette, pixmaptools.LUT.Palette.GREYSCALE: greyscale_palette }
        self.do_bpm = True
        self.bpm_handler = None
        self.bpm_manager = None
        self.latency = 0
        self.bkg_substraction_handler = None      
 
        class ImCallback(self.Lima.Core.CtControl.ImageStatusCallback): 
          def imageStatusChanged(this, status):
            if status.LastImageReady >= 0:
              status.reset()
              if not self.do_bpm:
                self.handle_bpm_result(NoBpmResult())

        self._set_acquisition_rate(self.acquisition_rate)

        if ccd_control is None:
            self.Lima = __import__("Lima", globals(), locals(), ["Core", "Basler"])
            try:
                self.ccd = self.Lima.Basler.Camera(self.ccd_ip, 8000)
            except:
                self.ccd = None
                return 
            _ = self.Lima.Basler.Interface(self.ccd)
            self.ccd_control = self.Lima.Core.CtControl(_) 
            self.ccd_control._i = _
        else:
            self.ccd_control = ccd_control()
        
        BEAM_VIEWERS[id(self)]=self

        try:
          self.ccd_control.hwInterface().setGain(0)
        except AttributeError:
          pass

        #self.saving_ctrl=self.ccd_control.saving()
        #pars=self.saving_ctrl.getParameters() 
        #pars.directory="/tmp"
        #pars.fileFormat=self.saving_ctrl.EDF
        #pars.prefix="image_"
        ##pars.savingMode=self.saving_ctrl.AutoFrame
        #self.saving_ctrl.setParameters(pars)

        self._image = self.ccd_control.image()
        if self.orientation > 0:
            rotation = (self.Lima.Core.Rotation_90, self.Lima.Core.Rotation_180, self.Lima.Core.Rotation_270)[self.orientation-1]
            self._image.setRotation(rotation)
        print "applying rotation on image:", self.orientation*90, "degrees"
        self.ccd_control._imcb = ImCallback()
        self.ccd_control.registerImageStatusCallback(self.ccd_control._imcb)
        self._ext = self.ccd_control.externalOperation()

        self.has_roi = False	
        dim = self._image.getImageDim().getSize()
        self.max_width = dim.getWidth()
        self.max_height = dim.getHeight()
        self.width = self.max_width
        self.height = self.max_height
        print "max image width=", self.max_width, "max image height=",  self.max_height


    def get_name(self):
        return self.name


    def bpm_calculation_is_on(self):
        return self.do_bpm


    def _start_webserver(self, port=None, home_title=None):
        global webserver
        global webserver_queries_handling
        if webserver is None:
            webserver = xbpmds_webserver.webserver(port, home_title)
            webserver_queries_handling = gevent.spawn(handle_webserver_queries)
        return webserver.port 


    def _set_acquisition_parameters(self, parameters):
        self.ccd_exposure_time = float(parameters["exp_t"])

        if bool(int(parameters.get("do_bpm", 1))):
          self.bpm_on()
        else:
          self.bpm_off()

        try:
          acq_rate = float(parameters.get("acq_rate", 0))
        except ValueError:
          acq_rate = 0
        self._set_acquisition_rate(acq_rate)


    def _set_automatic_aoi(self, aoi):
        self.do_automatic_roi = aoi
        try:
          self.bpm_handler.getTask().mRoiAutomatic=aoi
        except:
          pass


    def _get_automatic_aoi(self):
        return self.do_automatic_roi 


    def _start_acquisition(self, nb_frames, latency=0):
        a = self.ccd_control.acquisition()
        try:
          a.setAcqNbFrames(nb_frames)
          a.setAcqExpoTime(self.ccd_exposure_time)
          a.setLatencyTime(latency)
        except:
          logging.exception("Could not set acquisition parameters")
          return None

        t0 = time.time()
        while True:
          try:
            self.ccd_control.prepareAcq()
          except:
            if time.time() - t0 > 1:
              raise
            #time.sleep(0.01)
          else:
            break

        self.ccd_control.startAcq()


    def take_background(self):
       if not self.check_ready():
         raise RuntimeError, "Acquisition has not finished (or Live mode is on)"
       self.get_position()
       self._set_background()


    def _set_background(self):
        if self.bkg_substraction_handler is not None:
          self._ext.delOp("bkg")
        im = self.ccd_control.ReadImage()
        self.bkg_substraction_handler = self._ext.addOp(self.Lima.Core.BACKGROUNDSUBSTRACTION, "bkg", 0)
        self.bkg_substraction_handler.setBackgroundImage(im)
        self._ext_change_event.set()


    def _has_background(self):
        return self.bkg_substraction_handler is not None


    def reset_background(self):
        if self.bkg_substraction_handler is not None:
          self._ext.delOp("bkg")
          self.bkg_substraction_handler = None      
          self._ext_change_event.set()
    
   
    def get_intensity(self, x, y):
        if self.raw_image_buffer is not None:
          try:
            i = int(self.raw_image_buffer[y][x])
            return i
          except:
            pass
        return -1

 
    def _push_image(self):
        #print "!!!!! NEW IMAGE", self
        im = self.ccd_control.ReadImage()
        #t0=time.time()

        binTask = self.Lima.Core.Processlib.Tasks.Binning()
        binTask.setProcessingInPlace(False)
        binTask.mXFactor = 2
        binTask.mYFactor = 2
        im = binTask.process(im)

        self.raw_image_buffer = im.buffer.copy()
   
        jpegFile = cStringIO.StringIO()
        height, width = im.buffer.shape
        #I  = Image.fromarray(im.buffer, "L")
        #I.putpalette(palette())
        #I.convert('RGB')
        #print ">>>>>>>>>>>>>>>>> image size w=", width, "h=", height
        if self.autoscale:
          img_buffer = pixmaptools.LUT.transform_autoscale(im.buffer, self.palette[self.color_map], self.lut_method)[0]
        else:
          img_buffer = pixmaptools.LUT.transform(im.buffer, self.palette[self.color_map], self.lut_method, 0, 4*4096)[0] #4x because of binning
        img_buffer.shape = (height, width, 4)
        I = Image.fromarray(img_buffer, "RGBX").convert("RGB")
        I.save(jpegFile, "jpeg", quality=95)
        #Image.fromarray(img_buffer, "RGBA").save(jpegFile, "png")
        raw_jpeg_data = jpegFile.getvalue()	
        self.jpegData = base64.b64encode(raw_jpeg_data)
        #print "b64 encoded data size=",len(self.jpegData)
        #print "time to reduce size, convert to temperature, jpeg encode and base64 encode:", time.time()-t0
        profile_x_data_list = self.last_profile_x.tolist()
        profile_y_data_list = self.last_profile_y.tolist()

        lima_roi = self.ccd_control.image().getRoi()
        roi_top_left = lima_roi.getTopLeft()
        roi_size = lima_roi.getSize()

        #msg = msgpack.packb({ "timestamp": self.last_acq_time, 
        msg = { "timestamp": self.last_acq_time,
                        "X": self.last_x, 
                        "Y": self.last_y, 
                        "I": self.last_intensity,
                     "maxI": self.last_max_intensity,
                 "jpegData": self.jpegData,
                      "roi": [roi_top_left.x, roi_top_left.y, roi_size.getWidth(), roi_size.getHeight()],
                   "fwhm_x": self.last_fwhm_x,
                   "fwhm_y": self.last_fwhm_y,
                "profile_x": zip(range(len(profile_x_data_list)), profile_x_data_list),
                "profile_y": zip(range(len(profile_y_data_list)), profile_y_data_list) }
    
        self.last_image = msg
            
        self.set_image_event_watcher.send()


    def acquire_live(self, live, from_web=False):
        if live:
          if self.check_ready():
              self.live = True

              if self.do_bpm:
                self.bpm_on()
              else:
                self.bpm_off()

              if self.tg_device:
               self.tg_device.command_inout_asynch("UpdateState")

              self._start_acquisition(0, self.latency)
        else:
          if self.live:
             self.live = False

             if self.tg_device:
               self.tg_device.command_inout_asynch("UpdateState")

             self.ccd_control.stopAcq()

        if not from_web:
          # inform clients
          self._ext_change_event.set()


    def handle_bpm_result(self, result, timestamp=None):
        try:
          return self._handle_bpm_result(result, timestamp)
        finally:
          self.bpm_result_event.set()
          self._push_image()


    def validate_number(self, x, fallback_value=-1, min_value=0, max_value=None):
        if x is None:
          return fallback_value
        if not numpy.isfinite(x):
          return fallback_value
        if numpy.isnan(x):
          return fallback_value
        if min_value is not None and x < min_value:
          return fallback_value
        if max_value is not None and x > max_value:
          return fallback_value
        return x


    def _handle_bpm_result(self, result, timestamp=None):
        result_array = numpy.array([-1]*7,dtype=numpy.double)
        self.last_result_array = result_array

        if timestamp is None:
          self.last_acq_time = time.time()
        else:
          self.last_acq_time = timestamp

        # deal with a single BPM calculation
        if not self.bpm_manager or result.errorCode != self.bpm_manager.OK: 
           result.beam_center_x = -1
           result.beam_center_y = -1
           result.beam_intensity = -1
           result.beam_fwhm_x = 0
           result.beam_fwhm_y = 0
           result.max_pixel_value = -1
        else:
           result.beam_center_x  = self.validate_number(result.beam_center_x, max_value=2000)
           result.beam_center_y  = self.validate_number(result.beam_center_y, max_value=2000)
           result.beam_intensity = self.validate_number(result.beam_intensity)
           result.beam_fwhm_x    = self.validate_number(result.beam_fwhm_x, fallback_value=0)
           result.beam_fwhm_y    = self.validate_number(result.beam_fwhm_y, fallback_value=0)
           result.max_pixel_value= self.validate_number(result.max_pixel_value)

        self.last_x = result.beam_center_x
        self.last_y = result.beam_center_y
        self.last_intensity = result.beam_intensity
        self.last_max_intensity = result.max_pixel_value
        self.last_fwhm_x = result.beam_fwhm_x
        self.last_fwhm_y = result.beam_fwhm_y
        try:
       	  self.last_profile_x = result.profile_x.buffer.astype(numpy.int) 
        except:
          self.last_profile_x = numpy.array([],dtype=numpy.int)
        try:
       	  self.last_profile_y = result.profile_y.buffer.astype(numpy.int)
        except:
          self.last_profile_y = numpy.array([],dtype=numpy.int)

        result_array[0]=self.last_acq_time
        result_array[1]=self.last_intensity
        result_array[2]=self.last_x
        result_array[3]=self.last_y
        result_array[4]=self.last_fwhm_x
        result_array[5]=self.last_fwhm_y
        result_array[6]=result.frameNumber
        return result_array


    def get_position(self, from_web=False):
        # do a 1 frame acquisition and return beam position
        old_do_bpm = self.do_bpm
        try:
          if not from_web:
            self.bpm_on()
          self.bpm_result_event.clear() 

          if not self.live:
            self._start_acquisition(1, 0) 
      
          self.bpm_result_event.wait(self.ccd_exposure_time+1)
          if not self.bpm_result_event.is_set():
              return numpy.array([-1]*7,dtype=numpy.double)
          else:
              result_array = self.last_result_array
              result_array[2]*=self.calib_x
              result_array[3]*=self.calib_y
              result_array[4]*=self.calib_x
              result_array[5]*=self.calib_y
              return result_array
        finally:
          if not from_web and not old_do_bpm:
             self.bpm_off()


    def _set_roi(self, x, y, w, h):
        self.reset_background()

        image = self.ccd_control.image()
        print "setting roi", x, y, w, h
        if w == self.max_width and h == self.max_height:
          self.has_roi = False
          image.resetRoi()
        else:
          self.has_roi = True
          roi = self.Lima.Core.Roi(x, y, w, h)
          image.setRoi(roi)
        self.width = w
        self.height = h


    def bpm_on(self):
        self.do_bpm = True
  
        if self.bpm_handler is None:
          class BpmEv(self.Lima.Core.Processlib.TaskEventCallback):
            def __init__(this, name):
              self.Lima.Core.Processlib.TaskEventCallback.__init__(this)
              this.timestamp = {}
            def started(this, data):
              this.timestamp[data.frameNumber]=time.time()
            def finished(this, data):
              #print data.frameNumber, "BPM took", time.time()-this.timestamp[data.frameNumber]
              self.handle_bpm_result(self.bpm_manager.getResult(data.frameNumber), timestamp=this.timestamp[data.frameNumber])
              del this.timestamp[data.frameNumber]

          self.bpm_handler = self._ext.addOp(self.Lima.Core.BPM, "bpm", 1)
          self.bpm_handler.getTask().mRoiAutomatic = self.do_automatic_roi
          self.bpm_handler.getTask().mEnableBackgroundSubstraction = True
          self.bpm_manager = self.bpm_handler.getManager()
          self.bpm_calculation_done_cb = BpmEv("bpm_result")
          self.bpm_task = self.bpm_handler.getTask().setEventCallback(self.bpm_calculation_done_cb)


    def bpm_off(self):
        self.do_bpm = False
        if self.bpm_handler is None:
          return

        self._ext.delOp("bpm")
        self.bpm_handler = None

  
    def _set_acquisition_rate(self, acq_rate):
        if math.fabs(acq_rate) <= 1E-6 or acq_rate >= (1.0/self.ccd_exposure_time):
          self.latency = 0
        else:
          self.latency = (1.0/acq_rate) - self.ccd_exposure_time
        print "acquisition rate set to", acq_rate, "Hz, latency =", self.latency


    def _get_acquisition_rate(self):
        return float(1/(self.latency + self.ccd_exposure_time))


    def set_ccd_exposure_time(self, exp_time):
        self.ccd_exposure_time = exp_time

        # inform clients
        self._ext_change_event.set()


    def get_ccd_exposure_time(self):
        return self.ccd_exposure_time


    def _fill_result_spectrum(self, result_spectrum, i=0):
        result_spectrum[0][i]=self.last_acq_time
        result_spectrum[1][i]=self.last_x*self.calib_x
        result_spectrum[2][i]=self.last_y*self.calib_y
        result_spectrum[3][i]=self.last_intensity
        result_spectrum[4][i]=self.last_fwhm_x*self.calib_x
        result_spectrum[5][i]=self.last_fwhm_y*self.calib_y
        result_spectrum[6][i]=self.last_max_intensity
 

    def _do_acquire_positions(self, t):
        old_do_bpm = self.do_bpm
        
        try:
            self.bpm_on()

            if int(t*self._get_acquisition_rate()) < 2:
                result_spectrum =  numpy.array([[0] for _ in range(7)],dtype=numpy.double)

                self.bpm_result_event.clear()
                self._start_acquisition(1, 0)

                # can't call getResult because callbacks are not executed yet!
                #self.bpm_manager.getResult(1+self.latency+self.ccd_exposure_time, 0)
                self.bpm_result_event.wait(self.ccd_exposure_time+self.latency+1)
                if self.bpm_result_event.is_set():
                    self._fill_result_spectrum(result_spectrum)
            else:
                tmp = numpy.array([[0]*2 for _ in range(7)],dtype=numpy.double)

                # have to estimate nb of frames, so a minimum of 2 frames will be collected
                # we store the results in a temporary arrays
                t0 = time.time()
                self._start_acquisition(2, 0)
                self.bpm_manager.getResult(t/2.0, 0)
                self._fill_result_spectrum(tmp, 0)
                self.bpm_manager.getResult(t/2.0, 1)
                self._fill_result_spectrum(tmp, 1)

                time_for_a_frame = self.ccd_control.ReadImage(1).timestamp - self.ccd_control.ReadImage(0).timestamp

                real_nb_frames = int((t - (time.time() - t0)) / time_for_a_frame) - 1

                nb_frames = max(2, real_nb_frames + 2)

                result_spectrum =  numpy.array([[0]*nb_frames for _ in range(7)],dtype=numpy.double)
                # copy values from temporary array to the result spectrum
                for i in range(2):
                  for j in range(7):
                    result_spectrum[j][i]=tmp[j][i]
  
                if real_nb_frames > 0:
                    self._start_acquisition(real_nb_frames, 0)
             
                i = 2
                while i < nb_frames:
                    self.bpm_manager.getResult(1+self.latency+self.ccd_exposure_time, i-2)
                    self._fill_result_spectrum(result_spectrum, i)
                    i+=1

            return result_spectrum
        finally:
            if not old_do_bpm:
               self.bpm_off()

            if self.tg_device:
               self.tg_device.command_inout_asynch("UpdateState")


    def in_acquisition(self):
        return self.live or (self.acquisition_task is not None and not self.acquisition_task.ready())


    def check_ready(self):
        return not self.live and not self.in_acquisition()


    def acquire_positions(self, t):
        self.acquisition_task = gevent.spawn(self._do_acquire_positions, t)
        # inform all clients of acq. end
        self.acquisition_task.link(lambda _: self._ext_change_event.set())
        # inform all clients of acq. start
        self._ext_change_event.set()


    def get_acquisition_results(self):
        if self.acquisition_task is not None:
            return self.acquisition_task.get()
        else:
            return numpy.array([[-1] for _ in range(7)],dtype=numpy.double) 


    def get_last_results(self):
        return self.last_acq_time, self.last_x*self.calib_x, self.last_y*self.calib_y, self.last_intensity, self.last_fwhm_x*self.calib_x, self.last_fwhm_y*self.calib_y, self.last_max_intensity


    def get_last_profiles(self):
        profile_x = self.last_profile_x
        profile_y = self.last_profile_y
        profile_x *= self.calib_x
        profile_y *= self.calib_y
        return profile_x, profile_y


    def get_calibration(self):
        return self.calib_x, self.calib_y


    def _update_calibration(self, calib_x, calib_y):
        self.calib_x = calib_x
        self.calib_y = calib_y

    def _save_calibration(self, calib_x, calib_y):
        self._update_calibration(calib_x, calib_y)
        self.tg_device.SaveCalibration([calib_x, calib_y])


if __name__ == '__main__':
    bv = BeamViewer(sys.argv[1])

    gevent.run()
