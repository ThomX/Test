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
# file :        imXPAD.py
#
# description : Python source for the imXPAD and its commands.
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
import os
from Lima import Core
from Lima import imXpad as XpadAcq
from LimaCCDs import CallableReadEnum,CallableWriteEnum


class imXPAD(PyTango.Device_4Impl):

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

    @Core.DEB_MEMBER_FUNCT
    def loadConfig(self,config_prefix) :
        config_path = self.config_path
        _imXPADCam.loadConfigGFromFile(os.path.join(config_path,'%s.cfg' % config_prefix))
        _imXPADCam.loadConfigLFromFileToSRAM(os.path.join(config_path,'%s.cfl' % config_prefix))
        _imXPADCam.loadConfigLSRAMToDetector()
        
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
                    function2Call = getattr(_XpadAcq,functionName)
                    callable_obj = CallableReadEnum(d,function2Call)
                else:
                    functionName = 'set' + attr_name
                    function2Call = getattr(_XpadAcq,function2Call)
                    callable_obj = CallableWriteEnum(d,function2Call)
                self.__dict__[name] = callable_obj
                return callable_obj
        raise AttributeError('Xpad has no attribute %s' % name)


class imXPADClass(PyTango.DeviceClass):

    class_property_list = {}

    device_property_list = {
        'cam_ip_address' :
        [PyTango.DevString,
         "Camera ip address",[]],
        'port' :
        [PyTango.DevLong,
         'ip port',[]],
        'model' :
        [PyTango.DevString,
         "Camera model",[]],
        'usb_device_id' :
        [PyTango.DevLong,
         'USB device id',[]],
        'config_path' :
        [PyTango.DevString,
         "Config path",[]]
        }

    cmd_list = {
        'getAttrStringValueList':
        [[PyTango.DevString, "Attribute name"],
         [PyTango.DevVarStringArray, "Authorized String value list"]],
        'loadConfig':
        [[PyTango.DevString, "Config file prefix"],
         [PyTango.DevVoid,""]]
        }

    attr_list = {
        }

    def __init__(self,name) :
        PyTango.DeviceClass.__init__(self,name)
        self.set_type(name)

#----------------------------------------------------------------------------
# Plugins
#----------------------------------------------------------------------------
_imXPADCam = None
_imXPADInterface = None

def get_control(cam_ip_address = "0",port=3456,model="XPAD_S70",**keys) :
    print cam_ip_address,port,model
    global _imXPADCam
    global _imXPADInterface
    port = int(port)
    if _imXPADCam is None:
	_imXPADCam = XpadAcq.Camera(cam_ip_address,port,model)
        _imXPADCam.setImageType(Core.Bpp32)
        _imXPADCam.init()
	_imXPADInterface = XpadAcq.Interface(_imXPADCam)
    return Core.CtControl(_imXPADInterface)

def get_tango_specific_class_n_device():
    return imXPADClass,imXPAD
