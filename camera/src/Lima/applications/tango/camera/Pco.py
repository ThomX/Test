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
# file :        Pco.py
#
# description : Python source for the Pco and its commands.
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
import pdb
from Lima import Core
from Lima import Pco as PcoAcq
from LimaCCDs import CallableReadEnum,CallableWriteEnum
from AttrHelper import get_attr_4u, get_attr_string_value_list,_getDictKey, _getDictValue


class Pco(PyTango.Device_4Impl):

    Core.DEB_CLASS(Core.DebModApplication, 'LimaCCDs')


#------------------------------------------------------------------
#    Device constructor
#------------------------------------------------------------------
    def __init__(self,*args) :
        PyTango.Device_4Impl.__init__(self,*args)

        self._Pco__Pixelrate = { "LOW": "95333333", "HIGH":"286000000" }    
        self._Pco__Rollingshutter = { "only for EDGE": "-1", "GLOBAL": "0", "ROLLING":"1" }    
        
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
        

    

#==================================================================
#
#    Pco read/write attribute methods
#
#==================================================================
    def __getattr__(self,name) :
        return get_attr_4u(self, name, _PcoAcq)

#------------------------------------------------------------------
#    lastError attribute R
#------------------------------------------------------------------
    def read_lastError(self, attr):
        val  = _PcoCam.talk("lasterror")
        attr.set_value(val)

#------------------------------------------------------------------
#    camInfo attribute R
#------------------------------------------------------------------
    def read_camInfo(self, attr):
        val  = _PcoCam.talk("camInfo")
        attr.set_value(val)

#------------------------------------------------------------------
#    camType attribute R
#------------------------------------------------------------------
    def read_camType(self, attr):
        val  = _PcoCam.talk("camType")
        attr.set_value(val)

#------------------------------------------------------------------
#    clXferPar attribute R
#------------------------------------------------------------------
    def read_clXferPar(self, attr):
        val  = _PcoCam.talk("clTransferParam")
        attr.set_value(val)

#------------------------------------------------------------------
#    cocRunTime attribute R
#------------------------------------------------------------------
    def read_cocRunTime(self, attr):
        val  = _PcoCam.talk("cocRunTime")
        attr.set_value(val)

#------------------------------------------------------------------
#    framerate attribute R
#------------------------------------------------------------------
    def read_frameRate(self, attr):
        val  = _PcoCam.talk("frameRate")
        attr.set_value(val)

#------------------------------------------------------------------
#    maxNbImages attribute R
#------------------------------------------------------------------
    def read_maxNbImages(self, attr):
        val  = _PcoCam.talk("maxNbImages")
        attr.set_value(val)

#------------------------------------------------------------------
#    info attribute R
#------------------------------------------------------------------
    def read_info(self, attr):
        val= _PcoCam.talk("")
        attr.set_value(val)

#------------------------------------------------------------------
#    version attribute R
#------------------------------------------------------------------
    def read_version(self, attr):
        val= _PcoCam.talk("timestamp")
        attr.set_value(val)

#------------------------------------------------------------------
#    pixelRate attribute RW
#------------------------------------------------------------------
    def read_pixelRate(self, attr):
        val  = _PcoCam.talk("pixelRate")
        key= _getDictKey(self._Pco__Pixelrate, val)
        attr.set_value(key)
        #print "--- read_pixelRate>",val, key

    def write_pixelRate(self, attr):
        data = attr.get_write_value()
        key = data
        value= _getDictValue(self._Pco__Pixelrate, key)
        cmd = '%s %s' % ('pixelRate', value)
        val  = _PcoCam.talk(cmd)
        #print "---- write_pixelRate>", cmd, key, value
        
#------------------------------------------------------------------
#    rollingShutter attribute RW
#------------------------------------------------------------------
    def read_rollingShutter(self, attr):
        val  = _PcoCam.talk("rollingShutter")
        key= _getDictKey(self._Pco__Rollingshutter, val)
        attr.set_value(key)
        #print "---- read_rollingShutter>", val, key

    def write_rollingShutter(self, attr):
        data = attr.get_write_value()
        key = data
        value= _getDictValue(self._Pco__Rollingshutter, key)
        cmd = '%s %s' % ('rollingShutter', value)
        val  = _PcoCam.talk(cmd)
        #print "---- write_rollingShutter>", cmd, key, value
        

#==================================================================
#
#    Pco command methods
#
#==================================================================
    @Core.DEB_MEMBER_FUNCT
    def getAttrStringValueList(self, attr_name):
        return get_attr_string_value_list(self, attr_name)

    @Core.DEB_MEMBER_FUNCT
    def talk(self, argin):
        val= _PcoCam.talk(argin)
        return val

#==================================================================
#
#    PcoClass class definition
#
#==================================================================
class PcoClass(PyTango.DeviceClass):

    #    Class Properties
    class_property_list = {}

    #    Device Properties

    device_property_list = {
        'debug_control':
        [PyTango.DevString,
           "general debug",[]],
        'debug_module':
        [PyTango.DevString,
           "debug module flags",[]],
        'debug_format':
        [PyTango.DevString,
           "debug format flags",[]],
        'debug_type':
        [PyTango.DevString,
           "debug flags flags",[]]
        }


    #    Command definitions
    cmd_list = {
        'getAttrStringValueList':
        [[PyTango.DevString, "Attribute name"],
         [PyTango.DevVarStringArray, "Authorized String value list"]],
        'talk':
        [[PyTango.DevString, "str argin"],
         [PyTango.DevString, "str argout"]],
        }

    #    Attribute definitions
    attr_list = {
         'rollingShutter':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ_WRITE]],
         'lastError':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ]],
         'camInfo':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ]],
         'camType':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ]],
         'clXferPar':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ]],
         'cocRunTime':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ]],
         'frameRate':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ]],
         'pixelRate':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ_WRITE]],
         'maxNbImages':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ]],
         'info':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ]],
         'version':	  
         [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ]],
        }

#------------------------------------------------------------------
#    PcoClass Constructor
#------------------------------------------------------------------
    def __init__(self,name) :
        PyTango.DeviceClass.__init__(self,name)
        self.set_type(name)

#----------------------------------------------------------------------------
# Plugins
#----------------------------------------------------------------------------
_PcoCam = None
_PcoInterface = None

def get_control(debug_control = "0", debug_module = "0", debug_type="0",
                debug_format = "0", **keys) :
    global _PcoCam
    global _PcoInterface


    debControl = int(debug_control,0)
    debModule = int(debug_module,0)
    debType = int(debug_type,0)
    debFormat = int(debug_format,0)

    


    if debControl:
        Core.DebParams.setModuleFlags(debModule)
        Core.DebParams.setTypeFlags(debType)
    else:
        Core.DebParams.setTypeFlags(0)
        Core.DebParams.setModuleFlags(0)

    #Core.DebParams.setFormatFlags(0x31)
    Core.DebParams.setFormatFlags(debFormat)

    if _PcoCam is None:
        _PcoCam = PcoAcq.Camera("")
        _PcoInterface = PcoAcq.Interface(_PcoCam)
    return Core.CtControl(_PcoInterface)

def get_tango_specific_class_n_device():
    return PcoClass,Pco

def close_interface() :
    global _PcoCam
    _PcoCam = None
