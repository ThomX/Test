import os
import logging
import subprocess
import TangoBridge
import PyTango
import sys
import threading
import numpy

_control_ref = None
WEBSERVER = None
sys.path.insert(0, "/users/blissadm/server/src/ElettraElectrometer")

class BeamViewer(PyTango.Device_4Impl):
    def __init__(self, *args):
        PyTango.Device_4Impl.__init__(self, *args)
        
        # each time push_event is called, event will be emitted
        # self.set_change_event("txy" , True, False)

        # now do init_device
        self.init_device()     
   
    def init_device(self):
        print "In ", self.get_name(), "::init_device()"
        self.get_device_properties(self.get_device_class())

        self.beam_viewer = TangoBridge.TangoBridge(beam_viewer.BeamViewer, 
                                                   self.ccd_ip,
                                                   self.ccd_exposure_time,
                                                   self.acquisition_rate,
                                                   self.orientation,
                                                   self.calibration_x,
                                                   self.calibration_y,
                                                   self.beam_x,
                                                   self.beam_y,
                                                   _control_ref,
                                                   self.get_name(),
                                                   self.get_name())

        # write port number to database
        tango_db = PyTango.DeviceProxy("sys/database/2")

        try:
          webserver_port = int(tango_db.DbGetDeviceProperty([self.get_name(), "port"])[4])
        except:
          webserver_port = None

        if self.beam_viewer is None:
          self.set_state(PyTango.DevState.FAULT)
          return
        
        # get webpage title
        try:
            device_info = tango_db.DbGetDeviceInfo(self.get_name())
            server_name = device_info[1][3]
        except:
            server_name = None #will use default title

        try:
          display_group = tango_db.DbGetDeviceProperty([self.get_name(), "display_group"])[4]
        except:
          display_group = ' '
        if display_group == ' ':
          display_group = server_name
        
        webserver_port = self.beam_viewer._start_webserver(webserver_port, display_group)
        
        # write port number to database
        tango_db.DbPutDeviceProperty([self.get_name(), "1", "port", "1", str(webserver_port)])

        self.set_state(PyTango.DevState.STANDBY)


    def delete_device(self):
        self.beam_viewer.exit()

    def always_executed_hook(self):
        pass

    def Live(self):  
        self.set_state(PyTango.DevState.MOVING) 
        self.beam_viewer.acquire_live(True)

    def Stop(self):
        self.beam_viewer.acquire_live(False)
        self.restore_state()   
 
    def GetPosition(self):
        if not self.beam_viewer.check_ready():
          raise RuntimeError, "Acquisition has not finished (or Live mode is on)"
        try:
          self.set_state(PyTango.DevState.MOVING)
          return self.beam_viewer.get_position()
        finally:
          self.restore_state()

    def read_tXY(self, attr):
        last_acq_time, last_x, last_y, _, _, _, _ = self.beam_viewer.get_last_results() 
        value = numpy.array([last_acq_time, last_x, last_y], numpy.double)
        attr.set_value(value)

    def read_X(self, attr):
        _, last_x, _, _, _, _, _ = self.beam_viewer.get_last_results()
        attr.set_value(last_x)

    def read_Y(self, attr):
        _, _, last_y, _, _, _, _ = self.beam_viewer.get_last_results()
        attr.set_value(last_y)

    def read_Intensity(self, attr):
        _, _, _, last_intensity, _, _, _ = self.beam_viewer.get_last_results()
        attr.set_value(last_intensity)

    def read_Proj_X(self, attr):
        last_profile_x, _ = self.beam_viewer.get_last_profiles()
        attr.set_value(last_profile_x)

    def read_Proj_Y(self, attr):
        _, last_profile_y = self.beam_viewer.get_last_profiles()
        attr.set_value(last_profile_y)

    def read_MaxIntensity(self, attr):
        _, _, _, _, _, _, last_max_intensity = self.beam_viewer.get_last_results()
        attr.set_value(last_max_intensity)

    def read_Fwhm_X(self, attr):
        _, _, _, _, last_fwhm_x, _, _ = self.beam_viewer.get_last_results()
        attr.set_value(last_fwhm_x)

    def read_Fwhm_Y(self, attr):
        _, _, _, _, _, last_fwhm_y, _ = self.beam_viewer.get_last_results()
        attr.set_value(last_fwhm_y)

    def read_ExposureTime(self, attr):
        attr.set_value(self.beam_viewer.get_ccd_exposure_time())

    def write_ExposureTime(self, attr):
        if not self.beam_viewer.check_ready():
          raise RuntimeError, "Acquisition has not finished (or Live mode is on)"

        self.beam_viewer.set_ccd_exposure_time(attr.get_write_value())
 
    def read_AcquisitionRate(self, attr):
        attr.set_value(self.beam_viewer._get_acquisition_rate())

    def write_AcquisitionRate(self, attr):
        if not self.beam_viewer.check_ready():
          raise RuntimeError, "Acquisition has not finished (or Live mode is on)"

        self.beam_viewer._set_acquisition_rate(attr.get_write_value())

    def read_AutomaticAOI(self, attr):
        attr.set_value(self.beam_viewer._get_automatic_aoi())

    def write_AutomaticAOI(self, attr):
        self.beam_viewer._set_automatic_aoi(attr.get_write_value())

    def restore_state(self):
        if not self.beam_viewer.check_ready():
          self.set_state(PyTango.DevState.MOVING)
        if self.beam_viewer.bpm_calculation_is_on():
          self.set_state(PyTango.DevState.ON)
        else:
          self.set_state(PyTango.DevState.OFF)

    def AcquirePositions(self, t):
        if self.beam_viewer.check_ready():
          self.set_state(PyTango.DevState.MOVING)
          self.beam_viewer.acquire_positions(t)
        else:
          raise RuntimeError, "Acquisition has not finished (or Live mode is on)"

    def TakeBackground(self):
        self.set_state(PyTango.DevState.MOVING)
        try:
          self.beam_viewer.take_background()
        finally:
          self.restore_state()  

    def ResetBackground(self):
        self.set_state(PyTango.DevState.MOVING)
        try:
          self.beam_viewer.reset_background()
        finally:
          self.restore_state()

    def read_AcquisitionSpectrum(self, attr):
        attr.set_value(self.beam_viewer.get_acquisition_results())

    def UpdateState(self):
        self.restore_state()

    def SaveCalibration(self, calib):
        calib_x = calib[0]; calib_y = calib[1]
        tango_db = PyTango.DeviceProxy("sys/database/2")
        tango_db.DbPutDeviceProperty([self.get_name(), "1", "calibration_x", "1", str(calib_x)])
        tango_db.DbPutDeviceProperty([self.get_name(), "1", "calibration_y", "1", str(calib_y)])


    def LockBeamMark(self, bm):  
        x = bm[0]; y=bm[1]
        tango_db = PyTango.DeviceProxy("sys/database/2")
        tango_db.DbPutDeviceProperty([self.get_name(), "1", "beam_x", "1", str(x)])
        tango_db.DbPutDeviceProperty([self.get_name(), "1", "beam_y", "1", str(y)])
        

BPM_COMMANDS = { 'GetPosition': [[PyTango.DevVoid, ""], [PyTango.DevVarDoubleArray, ""]],
        'Live': [[PyTango.DevVoid, ""], [PyTango.DevVoid, ""]],
        'Stop': [[PyTango.DevVoid, ""], [PyTango.DevVoid, ""]],
        'AcquirePositions': [[PyTango.DevDouble, ""], [PyTango.DevVoid, ""]] }

BPM_ATTRIBUTES = {
        'tXY': [[PyTango.DevDouble, PyTango.SPECTRUM, PyTango.READ, 3 ]],
        'X': [[PyTango.DevDouble, PyTango.SCALAR, PyTango.READ ]],
        'Y': [[PyTango.DevDouble, PyTango.SCALAR, PyTango.READ ]],
        'AcquisitionSpectrum': [[PyTango.DevDouble, PyTango.IMAGE, PyTango.READ, 10000000, 7 ]]
    }


class BeamViewerClass(PyTango.DeviceClass):
    #    Class Properties
    class_property_list = { }
    #    Device Properties
    device_property_list = { 'port':
            [PyTango.DevLong,
            "Private do not use",
            []],
            'ccd_ip':
            [PyTango.DevString,
            "Ccd camera IP address",
            [] ],
            'ccd_exposure_time':
            [PyTango.DevDouble,
            "Ccd camera exposure time",
            [0.1] ], 
            "acquisition_rate":
            [PyTango.DevDouble,
            "Acquisition rate (in Hz)", 
            [10] ],
            "orientation":
            [PyTango.DevLong,
            "Orientation factor (0=normal, 1=90 degrees rotation, 2=180 degrees, 3=270 degrees)",
            [0] ],
            "calibration_x":
            [PyTango.DevDouble,
            "Pixel size X",
            [1] ],
            "calibration_y":
            [PyTango.DevDouble,
            "Pixel size Y",
            [1] ],
            "beam_x":
            [PyTango.DevDouble,
            "Beam position X",
            [0] ],
            "beam_y":
            [PyTango.DevDouble,
            "Beam position Y",
            [0] ],
    }
    #    Command definitions
    cmd_list = BPM_COMMANDS.copy().update({
        'TakeBackground': [[PyTango.DevVoid, ""], [PyTango.DevVoid, ""]],
        'ResetBackground': [[PyTango.DevVoid, ""], [PyTango.DevVoid, ""]],
        'UpdateState': [[PyTango.DevVoid, ""], [PyTango.DevVoid, ""]],
        'SaveCalibration': [[PyTango.DevVarDoubleArray, ""], [PyTango.DevVoid, ""]],
        'LockBeamMark': [[PyTango.DevVarDoubleArray, ""], [PyTango.DevVoid, ""]]
    })
    #    Attribute definitions
    attr_list = BPM_ATTRIBUTES.copy().update({
        'AutomaticAOI': [[PyTango.DevBoolean, PyTango.SCALAR, PyTango.READ_WRITE ]],
        'Intensity': [[PyTango.DevDouble, PyTango.SCALAR, PyTango.READ ]],
        'MaxIntensity': [[PyTango.DevDouble, PyTango.SCALAR, PyTango.READ]],
        'Proj_X': [[PyTango.DevLong, PyTango.SPECTRUM, PyTango.READ, 2048 ]],
        'Proj_Y': [[PyTango.DevLong, PyTango.SPECTRUM, PyTango.READ, 2048 ]],
        'Fwhm_X': [[PyTango.DevDouble, PyTango.SCALAR, PyTango.READ]],
        'Fwhm_Y': [[PyTango.DevDouble, PyTango.SCALAR, PyTango.READ]],
        'ExposureTime': [[PyTango.DevDouble, PyTango.SCALAR, PyTango.READ_WRITE]],
        'AcquisitionRate': [[PyTango.DevDouble, PyTango.SCALAR, PyTango.READ_WRITE]],
    })

    def __init__(self, name):
        PyTango.DeviceClass.__init__(self, name)
        self.set_type(name)


class ElectrometerBpm(PyTango.Device_4Impl):
    def __init__(self, *args):
        PyTango.Device_4Impl.__init__(self, *args)

        # each time push_event is called, event will be emitted
        # self.set_change_event("txy" , True, False)

        # now do init_device
        self.init_device()

    def init_device(self):
        print "In ", self.get_name(), "::init_device()"
        self.get_device_properties(self.get_device_class())

        try:
          self.tg_electrometer_device = PyTango.DeviceProxy(self.electrometer_device)
        except:
          self.set_state(PyTango.DevState.FAULT)
        else:
          try:
              self.tg_electrometer_device.ping()
          except:
              self.set_state(PyTango.DevState.FAULT)
          else: 
              self.set_state(PyTango.DevState.ON)
 
    def Stop(self):
        pass

    def Live(self):
        pass

    def AcquirePositions(self):
        pass

    def GetPosition(self):
        pass


class ElectrometerBpmClass(PyTango.DeviceClass):
    #    Class Properties
    class_property_list = { }
    #    Device Properties
    device_property_list = { 'electrometer_device':
            [PyTango.DevString,
            "Electrometer device name",
            []] }
    #    Command definitions
    cmd_list = BPM_COMMANDS.copy()
    #    Attribute definitions
    attr_list = BPM_ATTRIBUTES.copy()


def start_global_webserver(port_number=8066):
    global WEBSERVER
    WEBSERVER=subprocess.Popen([sys.executable, os.path.join(os.path.dirname(__file__), "webserver.py"), "%d" % port_number])
    print "Starting webserver: pid", WEBSERVER.pid


if __name__ == '__main__': 
    # log to stdout instead of stderr, at any level above DEBUG
    logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)

    #hdb = bsddb.btopen(os.path.join(os.path.dirname(__file__), 'bpm_hdb.dat'))
    try:
        sys.argv[0]="xbpmds."
        py = PyTango.Util(sys.argv)
        try:
            import beam_viewer
        except:
            logging.warning("Cannot load beam_viewer module, CCD BPM won't be available.")
        else:
            py.add_TgClass(BeamViewerClass, BeamViewer)
        try:
            import ElettraElectrometerDS
        except:
            logging.warning("Cannot load ElettraElectrometerDS module, Elettra BPM won't be available.")
        else:
            py.add_TgClass(ElettraElectrometerDS.ElectrometerClass, ElettraElectrometerDS.Electrometer)
            py.add_TgClass(ElectrometerBpmClass, ElectrometerBpm)
        U = PyTango.Util.instance()

        start_global_webserver()
 
        U.server_init()
  
        U.server_run()
    except PyTango.DevFailed,e:
        print '-------> Received a DevFailed exception:',e
        sys.exit(-1)
    except Exception,e:
        print '-------> An unforeseen exception occured....',e
        sys.exit(-1)


_control_ref = None
def set_control_ref(control_class_ref): 
    global _control_ref
    _control_ref = control_class_ref

def get_tango_specific_class_n_device():
    if not WEBSERVER:
        start_global_webserver()

    return BeamViewerClass,BeamViewer
