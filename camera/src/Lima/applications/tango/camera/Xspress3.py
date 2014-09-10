############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2013
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################
#=============================================================================
#
# file : Xspress3.py
#
# description : Python source for the Xspress3 and its commands.
# The class is derived from Device. It represents the
# CORBA servant object which will be accessed from the
# network. All commands which can be executed on the
# Pilatus are implemented in this file.
#
# project : TANGO Device Server
#
# copyleft : European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
#=============================================================================
# (c) - Bliss - ESRF
#=============================================================================
#
import PyTango
import numpy
from Lima import Core
from Lima import Xspress3 as Xspress3Acq
# import some useful helpers to create direct mapping between tango attributes
# and Lima interfaces.
from AttrHelper import get_attr_4u, get_attr_string_value_list

#------------------------------------------------------------------
#------------------------------------------------------------------
# class Xspress3
#------------------------------------------------------------------
#------------------------------------------------------------------

class Xspress3(PyTango.Device_4Impl):

    Core.DEB_CLASS(Core.DebModApplication, 'LimaCCDs')


#------------------------------------------------------------------
# Device constructor
#------------------------------------------------------------------
    def __init__(self, *args) :
        PyTango.Device_4Impl.__init__(self, *args)

        self.init_device()

#------------------------------------------------------------------
# Device destructor
#------------------------------------------------------------------
    def delete_device(self):
        pass

#------------------------------------------------------------------
# Device initialization
#------------------------------------------------------------------
    @Core.DEB_MEMBER_FUNCT
    def init_device(self):
        self.set_state(PyTango.DevState.ON)
        self.get_device_properties(self.get_device_class())
        self.attr_clocks_read = [-1,-1, 0]
        self.attr_scaling_read = [-1.,-1.,-1.,-1.,-1.,-1.,-1.,-1.]
        self.attr_playback_file_read = ""

#------------------------------------------------------------------
# getAttrStringValueList command:
#
# Description: return a list of authorized values if any
# argout: DevVarStringArray
#------------------------------------------------------------------
    @Core.DEB_MEMBER_FUNCT
    def getAttrStringValueList(self, attr_name):
        valueList = []
        dict_name = '_' + self.__class__.__name__ + '__' + ''.join([x.title() for x in attr_name.split('_')])
        d = getattr(self, dict_name, None)
        if d:
            valueList = d.keys()

        return valueList

    def __getDictKey(self, dict, value):
        try:
            ind = dict.values().index(value)
        except ValueError:
            return None
        return dict.keys()[ind]

    def __getDictValue(self, dict, key):
        try:
            value = dict[key]
        except KeyError:
            return None
        return value

#-----------------------------------------------------------------------------
#    Xspress3 command methods
#-----------------------------------------------------------------------------


    @Core.DEB_MEMBER_FUNCT
    def Reset(self):
        _Xspress3Camera.reset()

    @Core.DEB_MEMBER_FUNCT
    def InitBrams(self, argin):
        _Xspress3Camera.initBrams(argin)

    @Core.DEB_MEMBER_FUNCT
    def Pause(self):
        _Xspress3Camera.pause()

    @Core.DEB_MEMBER_FUNCT
    def Restart(self):
        _Xspress3Camera.restart()

    @Core.DEB_MEMBER_FUNCT
    def Arm(self):
        _Xspress3Camera.arm()

    @Core.DEB_MEMBER_FUNCT
    def Clear(self):
        _Xspress3Camera.clear()

    @Core.DEB_MEMBER_FUNCT
    def SaveSettings(self):
       _Xspress3Camera.saveSettings()

    @Core.DEB_MEMBER_FUNCT
    def RestoreSettings(self, argin):
       _Xspress3Camera.restoreSettings(argin)

    @Core.DEB_MEMBER_FUNCT
    def InitRois(self, argin):
       _Xspress3Camera.initRoi(argin)

    @Core.DEB_MEMBER_FUNCT
    def StartScope(self):
       _Xspress3Camera.startScope()

    @Core.DEB_MEMBER_FUNCT
    def ReadHistogram(self, argin):
        data = _Xspress3Camera.readHistogram(*argin)
        __dataflat_cache = numpy.array(data.buffer.ravel())
        if _Xspress3Camera.getUseDtc():
            __dataflat_cache.dtype = numpy.double
        else:
            __dataflat_cache.dtype = numpy.uint32
        data.releaseBuffer()
        return __dataflat_cache
        data.releaseBuffer()
        return __dataflat_cache

    @Core.DEB_MEMBER_FUNCT
    def ReadScalers(self, argin):
        data = _Xspress3Camera.readScalers(*argin)
        __dataflat_cache = numpy.array(data.buffer.ravel())
        if _Xspress3Camera.getUseDtc():
            __dataflat_cache.dtype = numpy.double
        else:
            __dataflat_cache.dtype = numpy.uint32
        data.releaseBuffer()
        return __dataflat_cache

    @Core.DEB_MEMBER_FUNCT
    def FormatRun(self, argin):
        _Xspress3Camera.format(*argin);

    @Core.DEB_MEMBER_FUNCT
    def LoadPlayback(self, argin):
        _Xspress3Camera.loadPlayback(self.attr_playback_file_read, *argin)

#==================================================================
#
# Xspress3 read/write attribute methods
#
#==================================================================


    def __getattr__(self, name) :
        return get_attr_4u(self, name, _Xspress3Interface)

    def read_numChan(self, attr):
        attr.set_value(_Xspress3Camera.getNumChan())

    def read_numCards(self, attr):
        attr.set_value(_Xspress3Camera.getNumCards())

    def read_chansPerCard(self, attr):
        attr.set_value(_Xspress3Camera.getChansPerCard())

    def read_maxNumChan(self, attr):
        attr.set_value(_Xspress3Camera.getMaxNumChan())

    def read_binsPerMca(self, attr):
        attr.set_value(_Xspress3Camera.getBinsPerMca())

    def read_card(self, attr):
        attr.set_value(_Xspress3Camera.getCard())

    def write_card(self, attr):
        card=attr.get_write_value()
        _Xspress3Camera.setCard(card)
        
    def read_windows(self, attr):
        returnList = []
        for i in range(_Xspress3Camera.getNumChan()):
            for j in range(2):
                low, high = _Xspress3Camera.getWindow(i, j)
                returnList.append(low)
                returnList.append(high)
        attr.set_value(returnList)

    def write_windows(self, attr):
        data=attr.get_write_value()
        for i in range(0,len(data),4):
            _Xspress3Camera.setWindow(*data[i:i+4])

    def write_runMode(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setRunMode(*data)

    def read_runMode(self, attr):
        attr.set_value(_Xspress3Camera.getRunMode())

    def read_clocks(self, attr):
        attr.set_value(self.attr_clocks_read)

    def write_clocks(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setupClocks(*data)
        for i in range(len(data)):
            self.attr_clocks_read[i] = data[i]

    def write_goodThreshold(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setGoodThreshold(*data)

    def read_goodThreshold(self, attr):
        attr.set_value([_Xspress3Camera.getGoodThreshold(i) for i in range(_Xspress3Camera.getNumChan())])

    def read_dtcEnergy(self, attr):
        attr.set_value(_Xspress3Camera.getDeadtimeCalculationEnergy())

    def write_dtcEnergy(self, attr):
        energy=attr.get_write_value()
        _Xspress3Camera.setDeadtimeCalculationEnergy(energy)

    def read_dtcParameters(self, attr):
        returnList = []
        for i in range(_Xspress3Camera.getNumChan()):
            p1,p2,p3,p4,p5,p6 = _Xspress3Camera.getDeadtimeCorrectionParameters(i)
            returnList.append(p1)
            returnList.append(p2)
            returnList.append(p3)
            returnList.append(p4)
            returnList.append(p5)
            returnList.append(p6)
            attr.set_value(returnList)

    def write_dtcParameters(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setDeadtimeCorrectionParameters(*data)

    def read_scaling(self, attr):
        attr.set_value([self.attr_scaling_read[i] for i in range(_Xspress3Camera.getNumChan())])

    def write_scaling(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setScaling(*data)
        if data[0] == -1:
            for i in range(_Xspress3Camera.getNumChan()):
                self.attr_scaling_read[i] = data[1]
        else:
            self.attr_scaling_read[int(data[0])] = data[1]

    def read_fanTemperatures(self, attr):
        data = _Xspress3Camera.getFanTemperatures()
        __dataflat_cache = numpy.array(data.buffer.ravel())
        __dataflat_cache.dtype = numpy.double
        data.releaseBuffer()
        attr.set_value(__dataflat_cache)

    def write_setpoint(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setFanSetpoint(data)

    def write_fanController(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setFanController(*data)

    def write_roi(self, attr):
        data=attr.get_write_value()
        num_roi = (len(data)-1)/3
        roi = Xspress3Acq.Camera.Xsp3Roi(num_roi)
        for i in range(num_roi):
            j=i*3+1
            roi.addRegion(i, *data[j:j+3])
        _Xspress3Camera.setRoi(data[0], roi)

    def read_useDtc(self, attr):
        attr.set_value(_Xspress3Camera.getUseDtc())

    def write_useDtc(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setUseDtc(data)

    def write_setTiming(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setTiming(*data);

    def write_adcTempLimit(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setAdcTempLimit(data)

    def write_setPlayback(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setPlayback(data)

    def write_playbackFilename(self, attr):
        self.attr_playback_file_read=attr.get_write_value()

    def read_playbackFilename(self, attr):
        attr.set_value(self.attr_playback_file_read)

    def write_dataSource(self, attr):
        data=attr.get_write_value()
        _Xspress3Camera.setDataSource(*data)

    def read_dataSource(self, attr):
        attr.set_value([_Xspress3Camera.getDataSource(i) for i in range(_Xspress3Camera.getNumChan())])

#------------------------------------------------------------------
#------------------------------------------------------------------
# class Xspress3Class
#------------------------------------------------------------------
#------------------------------------------------------------------

class Xspress3Class(PyTango.DeviceClass):

    class_property_list = {}

    #    Device Properties
    device_property_list = {
        'baseIPaddress':
            [PyTango.DevString,
            "override the base IP address (e.g. 192.168.0.1) \nfrom which all other address`s are calculated or NULL to use the default",
            [] ],
        'baseMacAddress':
            [PyTango.DevString,
            "override the base MAC address (e.g. 02.00.00.00.00),\nfrom which all other card MAC address`s are calculated or NULL to use the default",
            [] ],
        'basePort':
            [PyTango.DevLong,
            "override the base IP port number or 0 to use the default",
            [0]],
        'createScopeModule':
            [PyTango.DevBoolean,
            "true = don`t create a scope data module",
            [False]],
        'nbFrames':
            [PyTango.DevLong,
            "Number of 4096 energy bin spectra timeframes",
            [1]],
        'scopeModName':
            [PyTango.DevString,
            "the scope data module filename or NULL to use the default.",
            ["NULL"] ],
        'nbCards':
            [PyTango.DevLong,
            "the number of xspress3 cards that constitute the xspress3 system, \nbetween 1 and  XSP3_MAX_CARDS",
            [1]],
        'nbChans':
            [PyTango.DevLong,
            "limit the number of channels.",
            [-1]],
        'debug':
            [PyTango.DevLong,
            "debug messages (0 = off, 1=normal, 2=verbose)",
            [0]],
        'noUDP':
            [PyTango.DevBoolean,
            "true = don`t do UDP connection",
            [False]],
        'cardIndex':
            [PyTango.DevLong,
            "Starting card index",
            [] ],
        'directoryName':
            [PyTango.DevString,
            "The directory name to save and restore configurations",
            [] ],
        }

    cmd_list = {
        'getAttrStringValueList':
            [[PyTango.DevString, "Attribute name"],
            [PyTango.DevVarStringArray, "Authorized String value list"]],
        'Reset':
            [[PyTango.DevVoid, ""],
            [PyTango.DevVoid, ""]],
        'InitBrams':
            [[PyTango.DevLong, "channel"],
            [PyTango.DevVoid, ""]],
        'Pause':
            [[PyTango.DevVoid, "none"],
            [PyTango.DevVoid, "none"]],
        'Restart':
            [[PyTango.DevVoid, "none"],
            [PyTango.DevVoid, "none"]],
        'Arm':
            [[PyTango.DevVoid, "none"],
            [PyTango.DevVoid, "none"]],
        'Clear':
            [[PyTango.DevVoid, "none"],
            [PyTango.DevVoid, "none"]],
        'SaveSettings':
            [[PyTango.DevVoid, "none"],
            [PyTango.DevVoid, "none"]],
        'RestoreSettings':
            [[PyTango.DevBoolean, "force restore if major revision of saved file does not match the firmware revision."],
            [PyTango.DevVoid, "none"]],
        'InitRois':
            [[PyTango.DevLong, "channel"],
            [PyTango.DevVoid, ""]],
        'ReadHistogram':
            [[PyTango.DevVarLongArray,"frame, channel"],
            [PyTango.DevVarULongArray,"the histogram data"]],
        'ReadScalers':
            [[PyTango.DevVarLongArray,"frame, channel"],
            [PyTango.DevVarULongArray,"the scaler data"]],
        'StartScope':
            [[PyTango.DevVoid, "none"],
            [PyTango.DevVoid, "none"]],
        'LoadPlayback':
            [[PyTango.DevVarLongArray,"src0,src1,[num_streams, digital]"],
            [PyTango.DevVoid, "none"]],
        'FormatRun':
            [[PyTango.DevVarLongArray,"chan,[nbits_eng,aux1_mode,adc_bits,min_samples,aux2_mode,pileup_reject]"],
            [PyTango.DevVoid, "none"]],

        }

    attr_list = {
         'card':
            [[PyTango.DevLong,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],

         'numChan':
            [[PyTango.DevLong,
              PyTango.SCALAR,
              PyTango.READ]],

         'numCards':
            [[PyTango.DevLong,
              PyTango.SCALAR,
              PyTango.READ]],

         'chansPerCard':
            [[PyTango.DevLong,
              PyTango.SCALAR,
              PyTango.READ]],

         'maxNumChan':
            [[PyTango.DevLong,
              PyTango.SCALAR,
              PyTango.READ]],

         'binsPerMca':
            [[PyTango.DevLong,
              PyTango.SCALAR,
              PyTango.READ]],

         'windows':
            [[PyTango.DevLong,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 32]],

          'runMode':
            [[PyTango.DevBoolean,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 4]],

          'clocks':
            [[PyTango.DevLong,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 3]],

          'goodThreshold':
            [[PyTango.DevLong,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 16]],

          'dtcEnergy':
            [[PyTango.DevDouble,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],

          'dtcParameters':
            [[PyTango.DevDouble,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 48]],

          'scaling':
            [[PyTango.DevDouble,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 8]],

          'fanTemperatures':
            [[PyTango.DevDouble,
              PyTango.SPECTRUM,
              PyTango.READ, 50]],

          'fanController':
            [[PyTango.DevDouble,
              PyTango.SPECTRUM,
              PyTango.WRITE, 2]],

          'setPoint':
            [[PyTango.DevDouble,
              PyTango.SCALAR,
              PyTango.WRITE]],

          'roi':
            [[PyTango.DevLong,
              PyTango.SPECTRUM,
              PyTango.WRITE, 25]],

          'useDtc':
            [[PyTango.DevBoolean,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],

          'setTiming':
            [[PyTango.DevLong,
              PyTango.SPECTRUM,
              PyTango.WRITE, 7]],

          'adcTempLimit':
            [[PyTango.DevLong,
              PyTango.SCALAR,
              PyTango.WRITE]],

          'setPlayback':
            [[PyTango.DevBoolean,
              PyTango.SCALAR,
              PyTango.WRITE]],

          'playbackFilename':
            [[PyTango.DevString,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],

          'dataSource':
            [[PyTango.DevLong,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE,8]],

      }

    def __init__(self, name) :
        PyTango.DeviceClass.__init__(self, name)
        self.set_type(name)


#------------------------------------------------------------------
#------------------------------------------------------------------
# Plugins
#------------------------------------------------------------------
#------------------------------------------------------------------

_Xspress3Camera = None
_Xspress3Interface = None

def get_control(nbCards=1, nbFrames=1, baseIPaddress="", basePort=0, baseMACaddress="", nbChans=1, createScopeModule=0, scopeModName="", debug=1, cardIndex=0, noUDP=0, directoryName="", **keys) :
    global _Xspress3Camera
    global _Xspress3Interface
#    Core.DebParams.setTypeFlags(Core.DebParams.AllFlags)
    if _Xspress3Interface is None:
        _Xspress3Camera = Xspress3Acq.Camera(int(nbCards), int(nbFrames), baseIPaddress, int(basePort), baseMACaddress, int(nbChans),
                                          bool(int(createScopeModule)), scopeModName, int(debug), int(cardIndex), bool(int(noUDP)), directoryName)
        _Xspress3Interface = Xspress3Acq.Interface(_Xspress3Camera)
    return Core.CtControl(_Xspress3Interface)

def get_tango_specific_class_n_device():
    return Xspress3Class, Xspress3
