############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2011
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################
#=============================================================================
#
# file :        RoperScientific.py
#
# description : Python source for the Roper Scientific and its commands.
#                The class is derived from Device. It represents the
#                CORBA servant object which will be accessed from the
#                network. All commands which can be executed on the
#                Pilatus are implemented in this file.
#
# project :     TANGO Device Server
#
# copyleft :    European Synchrotron Radiation Facility
#               BP 220, Grenoble 38043
#               FRANCE
#
#=============================================================================
#         (c) - Bliss - ESRF
#=============================================================================
#
import PyTango
from Lima import Core
from Lima import RoperScientific as RoperScientificAcq
from LimaCCDs import CallableReadEnum,CallableWriteEnum


class RoperScientific(PyTango.Device_4Impl):

    Core.DEB_CLASS(Core.DebModApplication, 'LimaCCDs')


#------------------------------------------------------------------
#    Device constructor
#------------------------------------------------------------------
    def __init__(self,*args) :
        PyTango.Device_4Impl.__init__(self,*args)

        self.init_device()

#------------------------------------------------------------------
#    Device destructor
#------------------------------------------------------------------
    def delete_device(self):
        pass

#------------------------------------------------------------------
#    Device initialization
#------------------------------------------------------------------
    @Core.DEB_MEMBER_FUNCT
    def init_device(self):
        self.set_state(PyTango.DevState.ON)
        self.get_device_properties(self.get_device_class())

    @Core.DEB_MEMBER_FUNCT
    def getAttrStringValueList(self, attr_name):
        valueList=[]
        dict_name = '_' + self.__class__.__name__ + '__' + ''.join([x.title() for x in attr_name.split('_')])
        d = getattr(self,dict_name,None)
        if d:
            valueList = d.keys()

        return valueList

    def __getattr__(self,name) :
        if name.startswith('read_') or name.startswith('write_') :
            split_name = name.split('_')[1:]
            attr_name = ''.join([x.title() for x in split_name])
            dict_name = '_' + self.__class__.__name__ + '__' + attr_name
            d = getattr(self,dict_name,None)
            attr_name = self.__Attribute2FunctionBase.get('_'.join(split_name),attr_name)
            if d:
                if name.startswith('read_') :
                    functionName = 'get' + attr_name
                    function2Call = getattr(_RoperScientificAcq,functionName)
                    callable_obj = CallableReadEnum(d,function2Call)
                else:
                    functionName = 'set' + attr_name
                    function2Call = getattr(_RoperScientificAcq,function2Call)
                    callable_obj = CallableWriteEnum(d,function2Call)
                self.__dict__[name] = callable_obj
                return callable_obj
        raise AttributeError('RoperScientific has no attribute %s' % name)


#------------------------------------------------------------------
#    Read Temperature attribute
#------------------------------------------------------------------

    def read_Temperature(self, attr):
        temperature = _RoperScientificCam.getTemperature()
        attr.set_value(temperature)

#------------------------------------------------------------------
#    Read TemperatureSetPoint attribute
#------------------------------------------------------------------

    def read_TemperatureSetPoint(self, attr):
        temperature = _RoperScientificCam.getTemperatureSetPoint()
        attr.set_value(temperature)

#------------------------------------------------------------------
#    Write TemperatureSetPoint attribute
#------------------------------------------------------------------

    def write_TemperatureSetPoint(self, attr):
        data = attr.get_write_value()
        temperature = float(data)
        _RoperScientificCam.setTemperatureSetPoint(temperature)

#------------------------------------------------------------------
#    Read Gain attribute
#------------------------------------------------------------------

    def read_Gain(self, attr):
        gain = _RoperScientificCam.getGain()
        attr.set_value(gain)

#------------------------------------------------------------------
#    Write Gain attribute
#------------------------------------------------------------------

    def write_Gain(self, attr):
        data = attr.get_write_value()
        gain = int(data)
        _RoperScientificCam.setTemperatureSetPoint(gain)

#------------------------------------------------------------------
#    Read InternalAcqMode attribute
#------------------------------------------------------------------

    def read_InternalAcqMode(self, attr):
        int_acq_mode = _RoperScientificCam.getInternalAcqMode()
        attr.set_value(int_acq_mode)

#------------------------------------------------------------------
#    Write InternalAcqMode attribute
#------------------------------------------------------------------

    def write_InternalAcqMode(self, attr):
        data = attr.get_write_value()
        int_acq_mode = str(data)
        if int_acq_mode.upper() != "STANDARD" and int_acq_mode.upper() != "CONTINUOUS" and int_acq_mode.upper() != "FOCUS":
            PyTango.Except.throw_exception('WrongData',\
                                           'Wrong value InternalAcqMode: %s'%(int_acq_mode.upper()),\
                                           'LimaCCD Class')
            
        _RoperScientificCam.setInternalAcqMode(int_acq_mode)


#------------------------------------------------------------------
#    Read UseFullFrame attribute
#------------------------------------------------------------------

    def read_UseFullFrame(self, attr):
        use_full_frame = _RoperScientificCam.getUseFullFrame()
        attr.set_value(use_full_frame)

#------------------------------------------------------------------
#    Write UseFullFrame attribute
#------------------------------------------------------------------

    def write_UseFullFrame(self, attr):
        data = attr.get_write_value()
        use_full_frame = int(data)
        _RoperScientificCam.setUseFullFrame(use_full_frame)



        
class RoperScientificClass(PyTango.DeviceClass):

    class_property_list = {}

    device_property_list = {
        'cam_num':
        [PyTango.DevString,
         "Camera number",[]],
        }

    cmd_list = {
        'getAttrStringValueList':
        [[PyTango.DevString, "Attribute name"],
         [PyTango.DevVarStringArray, "Authorized String value list"]],
        }

    attr_list = {
        'Temperature':
            [[PyTango.DevFloat,
            PyTango.SCALAR,
            PyTango.READ]],
        'TemperatureSetPoint':
            [[PyTango.DevFloat,
            PyTango.SCALAR,
            PyTango.READ_WRITE]],
        'Gain':
            [[PyTango.DevLong,
            PyTango.SCALAR,
            PyTango.READ_WRITE]],
        'InternalAcqMode':
            [[PyTango.DevString,
            PyTango.SCALAR,
            PyTango.READ_WRITE]],
        'UseFullFrame':
            [[PyTango.DevLong,
            PyTango.SCALAR,
            PyTango.READ_WRITE]],
        }

    def __init__(self,name) :
        PyTango.DeviceClass.__init__(self,name)
        self.set_type(name)

#----------------------------------------------------------------------------
# Plugins
#----------------------------------------------------------------------------
_RoperScientificCam = None
_RoperScientificInterface = None

def get_control(cam_num = "0",**keys) :
    global _RoperScientificCam
    global _RoperScientificInterface
    my_cam_num = int(cam_num)
    if _RoperScientificCam is None:
	_RoperScientificCam = RoperScientificAcq.Camera(my_cam_num)
	_RoperScientificInterface = RoperScientificAcq.Interface(_RoperScientificCam)
    return Core.CtControl(_RoperScientificInterface)

def get_tango_specific_class_n_device():
    return RoperScientificClass,RoperScientific
