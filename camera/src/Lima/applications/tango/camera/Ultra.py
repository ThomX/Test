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
# file : Ultra.py
#
# description : Python source for the Ultra and its commands.
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
from Lima import Ultra as UltraAcq
# import some useful helpers to create direct mapping between tango attributes
# and Lima interfaces.
from AttrHelper import get_attr_4u, get_attr_string_value_list

#------------------------------------------------------------------
#------------------------------------------------------------------
# class Ultra
#------------------------------------------------------------------
#------------------------------------------------------------------

class Ultra(PyTango.Device_4Impl):

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
#    Ultra command methods
#-----------------------------------------------------------------------------

    @Core.DEB_MEMBER_FUNCT
    def SaveConfiguration(self):
       _UltraCamera.SaveConfiguration()

    @Core.DEB_MEMBER_FUNCT
    def RestoreConfiguration(self):
       _UltraCamera.RestoreConfiguration()

#==================================================================
#
# Ultra read/write attribute methods
#
#==================================================================


    def __getattr__(self, name) :
        return get_attr_4u(self, name, _UltraInterface)

    def read_headColdTemp(self, attr):
        attr.set_value(_UltraCamera.getHeadColdTemp())

    def read_headHotTemp(self, attr):
        attr.set_value(_UltraCamera.getHeadHotTemp())

    def read_tecColdTemp(self, attr):
        attr.set_value(_UltraCamera.getTecColdTemp())

    def read_tecSupplyVolts(self, attr):
        attr.set_value(_UltraCamera.getTecSupplyVolts())

    def read_adcPosSupplyVolts(self, attr):
        attr.set_value(_UltraCamera.getAdcPosSupplyVolts())

    def read_adcNegSupplyVolts(self, attr):
        attr.set_value(_UltraCamera.getAdcNegSupplyVolts())

    def read_vinPosSupplyVolts(self, attr):
        attr.set_value(_UltraCamera.getVinPosSupplyVolts())

    def read_vinNegSupplyVolts(self, attr):
        attr.set_value(_UltraCamera.getVinNegSupplyVolts())

    def read_headADCVdd(self, attr):
        attr.set_value(_UltraCamera.getHeadADCVdd())

    def read_headVdd(self, attr):
        attr.set_value(_UltraCamera.getHeadVdd())

    def write_headVdd(self, attr):
        _UltraCamera.setHeadVdd(attr.get_write_value())

    def read_headVref(self, attr):
        attr.set_value(_UltraCamera.getHeadVref())

    def write_headVref(self, attr):
        _UltraCamera.setHeadVref(attr.get_write_value())

    def read_headVrefc(self, attr):
        attr.set_value(_UltraCamera.getHeadVrefc())

    def write_headVrefc(self, attr):
        _UltraCamera.setHeadVrefc(attr.get_write_value())

    def read_headVpupref(self, attr):
        attr.set_value(_UltraCamera.getHeadVpupref())

    def write_headVpupref(self, attr):
        _UltraCamera.setHeadVpupref(attr.get_write_value())

    def read_headVclamp(self, attr):
        attr.set_value(_UltraCamera.getHeadVclamp())

    def write_headVclamp(self, attr):
        _UltraCamera.setHeadVclamp(attr.get_write_value())

    def read_headVres1(self, attr):
        attr.set_value(_UltraCamera.getHeadVres1())

    def write_headVres1(self, attr):
        _UltraCamera.setHeadVres1(attr.get_write_value())

    def read_headVres2(self, attr):
        attr.set_value(_UltraCamera.getHeadVres2())

    def write_headVres2(self, attr):
        _UltraCamera.setHeadVres2(attr.get_write_value())

    def read_headVTrip(self, attr):
        attr.set_value(_UltraCamera.getHeadVTrip())

    def write_headVTrip(self, attr):
        _UltraCamera.setHeadVTrip(attr.get_write_value())

    def read_fpgaXchipReg(self, attr):
        attr.set_value(_UltraCamera.getFpgaXchipReg())

    def write_fpgaXchipReg(self, attr):
        _UltraCamera.setFpgaXchipReg(attr.get_write_value())

    def read_fpgaPwrReg(self, attr):
        attr.set_value(_UltraCamera.getFpgaPwrReg())

    def write_fpgaPwrReg(self, attr):
        _UltraCamera.setFpgaPwrReg(attr.get_write_value())

    def read_fpgaSyncReg(self, attr):
        attr.set_value(_UltraCamera.getFpgaSyncReg())

    def write_fpgaSyncReg(self, attr):
        _UltraCamera.setFpgaSyncReg(attr.get_write_value())

    def read_fpgaAdcReg(self, attr):
        attr.set_value(_UltraCamera.getFpgaAdcReg())

    def write_fpgaAdcReg(self, attr):
        _UltraCamera.setFpgaAdcReg(attr.get_write_value())

    def read_frameCount(self, attr):
        attr.set_value(_UltraCamera.getFrameCount())

    def read_frameError(self, attr):
        attr.set_value(_UltraCamera.getFrameError())

    def read_headPowerEnabled(self, attr):
        attr.set_value(_UltraCamera.getHeadPowerEnabled())

    def write_headPowerEnabled(self, attr):
        _UltraCamera.setHeadPowerEnabled(attr.get_write_value())

    def read_tecPowerEnabled(self, attr):
        attr.set_value(_UltraCamera.getTecPowerEnabled())

    def write_tecPowerEnabled(self, attr):
        _UltraCamera.setTecPowerEnabled(attr.get_write_value())

    def read_biasEnabled(self, attr):
        attr.set_value(_UltraCamera.getBiasEnabled())

    def write_biasEnabled(self, attr):
        _UltraCamera.setBiasEnabled(attr.get_write_value())

    def read_syncEnabled(self, attr):
        attr.set_value(_UltraCamera.getSyncEnabled())

    def write_syncEnabled(self, attr):
        _UltraCamera.setSyncEnabled(attr.get_write_value())

    def read_calibEnabled(self, attr):
        attr.set_value(_UltraCamera.getCalibEnabled())

    def write_calibEnabled(self, attr):
        _UltraCamera.setCalibEnabled(attr.get_write_value())

    def read_8pCEnabled(self, attr):
        attr.set_value(_UltraCamera.get8pCEnabled())

    def write_8pCEnabled(self, attr):
        _UltraCamera.set8pCEnabled(attr.get_write_value())

    def read_tecOverTemp(self, attr):
        attr.set_value(_UltraCamera.getTecOverTemp())

    def read_adcOffset(self, attr):
        attr.set_value(_UltraCamera.getAdcOffset())

    def write_adcOffset(self, attr):
        data = attr.get_write_value()
        _UltraCamera.setAdcOffset(*data)

    def read_adcGain(self, attr):
        attr.set_value(_UltraCamera.getAdcGain())

    def write_adcGain(self, attr):
        data = attr.get_write_value()
        _UltraCamera.setAdcGain(*data)

    def read_aux1(self, attr):
        attr.set_value(_UltraCamera.getAux1())

    def write_aux1(self, attr):
        data = attr.get_write_value()
        _UltraCamera.setAux1(*data)

    def read_aux2(self, attr):
        attr.set_value(_UltraCamera.getAux2())

    def write_aux2(self, attr):
        data = attr.get_write_value()
        _UltraCamera.setAux2(*data)

    def read_xchipTiming(self, attr):
        attr.set_value(_UltraCamera.getxchipTiming())

    def write_xchipTiming(self, attr):
        data = attr.get_write_value()
        _UltraCamera.setxchipTiming(*data)


#------------------------------------------------------------------
#------------------------------------------------------------------
# class UltraClass
#------------------------------------------------------------------
#------------------------------------------------------------------

class UltraClass(PyTango.DeviceClass):

    class_property_list = {}

    #    Device Properties
    device_property_list = {
        'headIPaddress':
            [PyTango.DevString,
            "should be 192.168.1.100 by default",
            ["192.168.1.100"] ],
        'hostIPaddress':
            [PyTango.DevString,
            "should be 192.168.1.103 by default",
            ["192.168.1.103"] ],
        'tcpPort':
            [PyTango.DevLong,
            "the tcp echo port 7 by default",
            [7]],
        'udpPort':
            [PyTango.DevLong,
            "5005 by default",
            [5005]],
        'nPixels':
            [PyTango.DevLong,
            "number of detector pixels.",
            [512]],
        }

    cmd_list = {
        'getAttrStringValueList':
            [[PyTango.DevString, "Attribute name"],
            [PyTango.DevVarStringArray, "Authorized String value list"]],
        'SaveConfiguration':
            [[PyTango.DevVoid, ""],
            [PyTango.DevVoid, ""]],
        'RestoreConfiguration':
            [[PyTango.DevVoid, ""],
            [PyTango.DevVoid, ""]],

        }

    attr_list = {
         'headColdTemp':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ]],
         'headHotTemp':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ]],
         'tecColdTemp':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ]],
         'tecSupplyVolts':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ]],
         'adcPosSupplyVolts':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ]],
         'adcNegSupplyVolts':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ]],
         'vinPosSupplyVolts':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ]],
         'vinNegSupplyVolts':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ]],
         'headADCVdd':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ]],
         'headVdd':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'headVref':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'headVrefc':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'headVpupref':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'headVclamp':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'headVres1':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'headVres2':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'headVTrip':
            [[PyTango.DevFloat,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'fpgaXchipReg':
            [[PyTango.DevULong,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'fpgaPwrReg':
            [[PyTango.DevULong,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'fpgaSyncReg':
            [[PyTango.DevULong,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'fpgaAdcReg':
            [[PyTango.DevULong,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'frameCount':
            [[PyTango.DevULong,
              PyTango.SCALAR,
              PyTango.READ]],
         'frameError':
            [[PyTango.DevULong,
              PyTango.SCALAR,
              PyTango.READ]],
         'headPowerEnabled':
            [[PyTango.DevBoolean,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'tecPowerEnabled':
            [[PyTango.DevBoolean,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'biasEnabled':
            [[PyTango.DevBoolean,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'syncEnabled':
            [[PyTango.DevBoolean,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'calibEnabled':
            [[PyTango.DevBoolean,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         '8pCEnabled':
            [[PyTango.DevBoolean,
              PyTango.SCALAR,
              PyTango.READ_WRITE]],
         'tecOverTemp':
            [[PyTango.DevBoolean,
              PyTango.SCALAR,
              PyTango.READ]],
         'adcOffset':
            [[PyTango.DevFloat,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 16]],
         'adcGain':
            [[PyTango.DevFloat,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 16]],
         'aux1':
            [[PyTango.DevULong,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 2]],
         'aux2':
            [[PyTango.DevULong,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 2]],
         'xchipTiming':
            [[PyTango.DevULong,
              PyTango.SPECTRUM,
              PyTango.READ_WRITE, 9]],

      }

    def __init__(self, name) :
        PyTango.DeviceClass.__init__(self, name)
        self.set_type(name)

#------------------------------------------------------------------
#------------------------------------------------------------------
# Plugins
#------------------------------------------------------------------
#------------------------------------------------------------------

_UltraCamera = None
_UltraInterface = None

def get_control(headIPaddress, hostIPaddress, tcpPort, udpPort, nPixels) :
    global _UltraCamera
    global _UltraInterface
#    Core.DebParams.setTypeFlags(Core.DebParams.AllFlags)
    if _UltraInterface is None:
        _UltraCamera = UltraAcq.Camera(headIPaddress, hostIPaddress, int(tcpPort), int(udpPort), int(nPixels))
        _UltraInterface = UltraAcq.Interface(_UltraCamera)
    return Core.CtControl(_UltraInterface)

def get_tango_specific_class_n_device():
    return UltraClass, Ultra
