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
# file :        Basler.py
#
# description : Python source for the Basler and its commands.
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
from Lima import Basler as BaslerAcq
from AttrHelper import get_attr_4u, get_attr_string_value_list


class Basler(PyTango.Device_4Impl):

    Core.DEB_CLASS(Core.DebModApplication, 'LimaCCDs')


#------------------------------------------------------------------
#    Device constructor
#------------------------------------------------------------------
    def __init__(self,*args) :
        PyTango.Device_4Impl.__init__(self,*args)

        self.init_device()

        self.__Attribute2FunctionBase = {
                                         }

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

#------------------------------------------------------------------
#    getAttrStringValueList command:
#
#    Description: return a list of authorized values if any
#    argout: DevVarStringArray   
#------------------------------------------------------------------
    @Core.DEB_MEMBER_FUNCT
    def getAttrStringValueList(self, attr_name):
        #use AttrHelper
        return get_attr_string_value_list(self, attr_name)
#==================================================================
#
#    Basler read/write attribute methods
#
#==================================================================
    def __getattr__(self,name) :
        #use AttrHelper
        return get_attr_4u(self,name,_BaslerCam)


#==================================================================
#
#    BaslerClass class definition
#
#==================================================================
class BaslerClass(PyTango.DeviceClass):

    class_property_list = {}

    device_property_list = {
        'cam_ip_address':
        [PyTango.DevString,
         "Camera ip address",[]],
        'inter_packet_delay':
        [PyTango.DevLong,
         "Inter Packet Delay",0],
        'frame_transmission_delay':
        [PyTango.DevLong,
         "Frame Transmission Delay",0]
        }

    cmd_list = {
        'getAttrStringValueList':
        [[PyTango.DevString, "Attribute name"],
         [PyTango.DevVarStringArray, "Authorized String value list"]],
        }

    attr_list = {
        }

    def __init__(self,name) :
        PyTango.DeviceClass.__init__(self,name)
        self.set_type(name)

#----------------------------------------------------------------------------
# Plugins
#----------------------------------------------------------------------------
_BaslerCam = None
_BaslerInterface = None

# packet_size = 8000 suppose the eth MTU is set at least to 8192 (Jumbo mode !)
# otherwise frame transfer can failed, the package size must but 
# correspond to the MTU, see README file under Pylon-3.2.2 installation
# directory for for details about network optimization.

def get_control(cam_ip_address = "0", frame_transmission_delay = 0,
                inter_packet_delay = 0, packet_size = 8000,**keys) :
    print "cam_ip_address",cam_ip_address
    global _BaslerCam
    global _BaslerInterface

    if _BaslerCam is None:
	_BaslerCam = BaslerAcq.Camera(cam_ip_address,packet_size)
	_BaslerCam.setInterPacketDelay(int(inter_packet_delay))
	_BaslerCam.setFrameTransmissionDelay(int(frame_transmission_delay))
	_BaslerInterface = BaslerAcq.Interface(_BaslerCam)
    return Core.CtControl(_BaslerInterface)

def get_tango_specific_class_n_device():
    return BaslerClass,Basler
