############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2014
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
# file :        Maxipix.py
#
# description : Python source for the Maxipix and its commands. 
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
import sys, types, os, time

from Lima import Core
from Lima.Maxipix.MpxCommon import MpxError
# import some useful helpers to create direct mapping between tango attributes
# and Lima interfaces.
from AttrHelper import get_attr_4u, get_attr_string_value_list
import AttrHelper

class Maxipix(PyTango.Device_4Impl):

    Core.DEB_CLASS(Core.DebModApplication, 'Maxipix')
    

#------------------------------------------------------------------
#    Device constructor
#------------------------------------------------------------------
    def __init__(self,*args) :
        PyTango.Device_4Impl.__init__(self,*args)

	_PriamAcq = _MaxipixAcq.getPriamAcq()
        self.__SignalLevel = {'LOW_FALL': _PriamAcq.LOW_FALL,\
                              'HIGH_RISE': _PriamAcq.HIGH_RISE}
        self.__ReadyLevel = self.__SignalLevel
        self.__GateLevel = self.__SignalLevel
        self.__TriggerLevel = self.__SignalLevel
        self.__ShutterLevel = self.__SignalLevel
        
        self.__ReadyMode =   {'EXPOSURE': _PriamAcq.EXPOSURE,\
                              'EXPOSURE_READOUT': _PriamAcq.EXPOSURE_READOUT}
        self.__GateMode =    {'INACTIVE': _PriamAcq.INACTIVE,\
                              'ACTIVE': _PriamAcq.ACTIVE}
        self.__FillMode =    _MaxipixAcq.mpxFillModes
        
        self.__dacname = "thl"
        
        self.__Attribute2FunctionBase = {'signal_level': 'SignalLevel',
                                         'ready_level': 'ReadyLevel',
                                         'gate_level': 'GateLevel',
                                         'shutter_level': 'ShutterLevel',
                                         'trigger_level': 'TriggerLevel',
                                         'ready_mode': 'ReadyMode',
                                         'gate_mode': 'GateMode',
                                         'fill_mode': 'FillMode'
                                         }

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
        # Load the properties
        self.get_device_properties(self.get_device_class())

        # Apply property to the attributes

        for attr_name in ['fill_mode','ready_mode','ready_level','gate_mode','gate_level','shutter_level','trigger_level'] :       
            self.applyNewPropery(attr_name ,None)
                                         
	
#==================================================================
# 
# Some Utils
#
#==================================================================

    @Core.DEB_MEMBER_FUNCT
    def applyNewPropery(self, prop_name, extra=None):
        if extra is not None: name = self.__OtherAttribute2FunctionBase[prop_name]
        else: name = self.__Attribute2FunctionBase[prop_name]
        key = getattr(self, prop_name)
        if not key: return # property is not set
        
        dict = getattr(self, '_'+self.__class__.__name__+'__'+name)
        func = getattr(_MaxipixAcq, 'set'+name)
        deb.Always('Setting property '+prop_name) 

        val = AttrHelper._getDictValue(dict, key.upper())
        if  val is None:
            deb.Error('Wrong value for property %s :%s' % (prop_name, val))
        else:
            if extra is not None: func(extra,val)
            else: func(val)

    def __getConfigNameList(self):
        spath= os.path.normpath(self.config_path)
        if not os.path.isdir(spath):
            PyTango.Except.throw_exception('WrongData',\
                                           'Invalid path: %s'%(self.config_path),\
                                           'Maxipix Class') 

        else:
            dirList = os.listdir(spath)
            fileDict={}
            fileList=[]
            for file in dirList:
                if file.endswith('.cfg'):
                    filePath = spath+'/'+file
                    fileStat = os.stat(filePath)
                    modifiedTime = fileStat.st_mtime
                    fileDict[modifiedTime]= file.strip('.cfg')
        if fileDict:
            timeList = fileDict.keys();timeList.sort()
            for mTime in timeList:
                fileList.append(fileDict[mTime])
                #fileList.append(time.ctime(mTime))
        return fileList
                     
        
#==================================================================
#
#    Maxipix read/write attribute methods
#
#==================================================================

    def __getattr__(self,name) :
        _PriamAcq = _MaxipixAcq.getPriamAcq()
        if name.count('fill_mode'):
            return get_attr_4u(self, name, _MaxipixAcq)
        else:
            return get_attr_4u(self, name, _PriamAcq)
            

	            
    ## @brief Read the current dac name
    #
    def read_dac_name(self,attr) :
        attr.set_value(self.__dacname)
        
    ## @brief Write dac name
    #
    def write_dac_name(self,attr) :
        data = attr.get_write_value()

        dacs = _MaxipixAcq.mpxDacs
        if data not in dacs.getListKeys():
            PyTango.Except.throw_exception('WrongData',\
                                           'Wrong value %s: %s'%('dac_name',data),\
                                           'Maxipix Class')          
        self.__dacname = data[0]

    ## @brief Read the possible dac names
    #
    def read_dac_possible(self,attr) :

        dacs = _MaxipixAcq.mpxDacs
        data = dacs.getListKeys()
        attr.set_value(data)        

    # Read the chip dac value, named by the dac_name attribute
    # For multichips only a unique DAC is valid for all the chips
    def read_dac_value(self,attr) :
        data = 0
        dacs = _MaxipixAcq.mpxDacs
        data = dacs.getOneDac(0,self.__dacname)
        # if a all the chips don't have the same dac value
        # None is return, typically this is the case for thl
        if data == None: data = -1
        attr.set_value(data)
        
    ## @brief Write a DAC value of the named dac_name attribute
    #
    def write_dac_value(self,attr) :
        data = attr.get_write_value()
        dacs = _MaxipixAcq.mpxDacs
        dacs.setOneDac(0,self.__dacname, data)
        dacs.applyChipDacs(0)
    
    ## @brief Read threshold noise of a maxipix chips
    #
    def read_threshold_noise(self,attr) :
        dac = _MaxipixAcq.mpxDacs
        thlNoises = dac.getThlNoise(0)

        attr.set_value(thlNoises,len(thlNoises))
        
    ## @brief Write threshold noise of a maxipix chips
    #
    def write_threshold_noise(self,attr) :
        data = attr.get_write_value()

        dacs = _MaxipixAcq.mpxDacs
        dacs.setThlNoise(0,data)
        dacs.applyChipDacs(0)


    ## @brief Read the global threshold
    #
    def read_threshold(self,attr) :
        dacs = _MaxipixAcq.mpxDacs
        thl = dacs.getThl()
	if thl is None: thl = -1

        attr.set_value(thl)

    ## @brief Write the global threshold
    #
    def write_threshold(self,attr) :
        data = attr.get_write_value()
        
        dacs = _MaxipixAcq.mpxDacs
        dacs.setThl(data)
        dacs.applyChipDacs(0)

    ## @brief Read the energy step
    #
    # energy step is the coef which link the global threshold with energy
    # threshold
    # 
    def read_energy_calibration(self,attr) :
        dacs = _MaxipixAcq.mpxDacs
        values = dacs .getECalibration()
        
        attr.set_value(values,len(values))
        
    ## @brief Write the energy step
    #
    def write_energy_calibration(self,attr) :
        data = attr.get_write_value()

        dacs  = _MaxipixAcq.mpxDacs
        dacs.setECalibration(data)

    ## @brief Read the energy threshold
    #
    # energy_threshold = energy_step * threshold (global)
    def read_energy_threshold(self,attr) :
        dacs= _MaxipixAcq.mpxDacs
        value = dacs.getEThl()
	if value is None: value = -1
	
        attr.set_value(value)

    ## @brief Write the energy threshold
    #
    def write_energy_threshold(self,attr) :
        data = attr.get_write_value()
        
        dacs = _MaxipixAcq.mpxDacs
        dacs.setEThl(data)
        dacs.applyChipDacs(0)
        
    ## @brief read the config name
    #
    def read_config_name(self,attr) :
        cfg_name = ""
        if self.config_name:
            cfg_name = self.config_name
        attr.set_value(cfg_name)

    ## @brief Write the config name and load it
    #
    def write_config_name(self,attr) :
        data = attr.get_write_value()
        _MaxipixAcq.loadConfig(data)
        self.config_name = data

    ## @brief read the config path
    #
    def read_config_path(self,attr) :
        cfg_path = ""
        if self.config_path:
            cfg_path = self.config_path
        attr.set_value(cfg_path)

    ## @brief Write the config path
    #
    def write_config_path(self,attr) :
        data = attr.get_write_value()
        _MaxipixAcq.setPath(data)
        self.config_path = data

    ## @brief read the board id
    #
    def read_espia_dev_nb(self,attr) :
        espia_dev_nb = 0
        if self.espia_dev_nb:
            espia_dev_nb = self.espia_dev_nb
        attr.set_value(espia_dev_nb)


#==================================================================
#
#    Maxipix command methods
#
#==================================================================
#------------------------------------------------------------------
#    getAttrStringValueList command:
#
#    Description: return a list of authorized values if any
#    argout: DevVarStringArray   
#------------------------------------------------------------------
    @Core.DEB_MEMBER_FUNCT
    def getAttrStringValueList(self, attr_name):
        if attr_name.count('config_name'):
            return self.__getConfigNameList()
        else:
            return get_attr_string_value_list(self, attr_name)

#------------------------------------------------------------------
#    setDebugFlags command:
#
#    Description: Get the current acquired frame number
#    argout: DevVarDoubleArray    
#------------------------------------------------------------------
    @Core.DEB_MEMBER_FUNCT
    def setDebugFlags(self, deb_flags):
        deb_flags &= 0xffffffff
        deb.Param('Setting debug flags: 0x%08x' % deb_flags)
        Core.DebParams.setTypeFlags((deb_flags   >> 16)  & 0xff)
        Core.DebParams.setModuleFlags((deb_flags >>  0)  & 0xffff)

        deb.Trace('FormatFlags: %s' % Core.DebParams.getFormatFlagsNameList())
        deb.Trace('TypeFlags:   %s' % Core.DebParams.getTypeFlagsNameList())
        deb.Trace('ModuleFlags: %s' % Core.DebParams.getModuleFlagsNameList())

#------------------------------------------------------------------
#    getDebugFlags command:
#
#    Description: Get the current acquired frame number
#    argout: DevVarDoubleArray    
#------------------------------------------------------------------
    @Core.DEB_MEMBER_FUNCT
    def getDebugFlags(self):
        deb.Trace('FormatFlags: %s' % Core.DebParams.getFormatFlagsNameList())
        deb.Trace('TypeFlags:   %s' % Core.DebParams.getTypeFlagsNameList())
        deb.Trace('ModuleFlags: %s' % Core.DebParams.getModuleFlagsNameList())
        
        deb_flags = (((Core.DebParams.getTypeFlags()    & 0xff)   << 16) |
                     ((Core.DebParams.getModuleFlags()  & 0xffff) <<  0))
        deb_flags &= 0xffffffff
        deb.Return('Getting debug flags: 0x%08x' % deb_flags)
        return deb_flags

class MaxipixClass(PyTango.DeviceClass):

    class_property_list = {}

    device_property_list = {
        'espia_dev_nb':
        [PyTango.DevShort,
         "Espia board device number",[]],
        'config_path':
        [PyTango.DevString,
         "Path where configuration files are",[]],
        'config_name':
        [PyTango.DevString,
         "The default configuration loaded",[]],
        'reconstruction_active':
        [PyTango.DevBoolean,
         "Set active or inactive the image reconstruction",[]],
        'fill_mode':
        [PyTango.DevString,
         "The default configuration loaded",[]],	 
       'ready_level':
        [PyTango.DevString,
         "The ready output signal level",[]],	  
       'gate_level':
        [PyTango.DevString,
         "The gate output signal level",[]],	  
       'shutter_level':
        [PyTango.DevString,
         "The shutter output signal level",[]],	  
       'trigger_level':
        [PyTango.DevString,
         "The trigger output signal level",[]],	  
       'ready_mode':
        [PyTango.DevString,
         "The ready output signal level",[]],	  
       'gate_mode':
        [PyTango.DevString,
         "The gate output signal level",[]],	  
        }

    cmd_list = {
        'getAttrStringValueList':
        [[PyTango.DevString, "Attribute name"],
         [PyTango.DevVarStringArray, "Authorized String value list"]],
        'getDebugFlags':
        [[PyTango.DevVoid, ""],
         [PyTango.DevULong, "Debug flag in HEX format"]],
        'setDebugFlags':
        [[PyTango.DevULong, "Debug flag in HEX format"],
         [PyTango.DevVoid, ""]],
        }

    attr_list = {
        'threshold_noise':
        [[PyTango.DevLong,
          PyTango.SPECTRUM,
          PyTango.READ_WRITE,5],
         {
             'label':"Threshold (thlow) noise of chips",
             'unit':"N/A",
             'format':"%6d",
             'description':"Threshold (thlow) noise of the chip(s)",
         }],
        'threshold':
        [[PyTango.DevLong,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"Global Threshold ",
             'unit':"N/A",
             'format':"%6d",
             'description':"The global threshold, apply the same offset on all the chips",
         }],
        'energy_calibration':
        [[PyTango.DevDouble,
          PyTango.SPECTRUM,
          PyTango.READ_WRITE,2],
         {
             'label':"Energy calibration",
             'unit':"N/A",
             'format':"%5.2f",
             'description':"[0] = e0thl, [1] = estep: ethl=(thl-e0thl)*estep",
         }],
        'energy_threshold':
        [[PyTango.DevDouble,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"Energy thresholds",
             'unit':"keV",
             'format':"%5.2f",
             'description':"Threshold in energy (keV)",
         }],
        'config_name':
        [[PyTango.DevString,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"Configuration name",
             'unit':"N/A",
             'format':"",
             'description':"root name of the configuration files",
         }],
        'config_path':
        [[PyTango.DevString,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"Configuration directory path",
             'unit':"N/A",
             'format':"",
             'description':"Path of the configuration directory",
         }],
        'fill_mode':	  
        [[PyTango.DevString,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"Fill mode",
             'unit':"enum.",
             'format':"",
             'description':"Between chip filling mode",
         }],	  
        'espia_dev_nb':	  
        [[PyTango.DevShort,
          PyTango.SCALAR,
          PyTango.READ],
         {
             'label':"Espia board number",
             'unit':"number",
             'format':"",
             'description':"The Espia board device number",
         }],	  
        'ready_mode':	  
        [[PyTango.DevString,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"Ready output mode",
             'unit':"enum.",
             'format':"",
             'description':"Mode of the Ready output",
         }],	  
        'ready_level':	  
        [[PyTango.DevString,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"Ready output level",
             'unit':"enum.",
             'format':"",
             'description':"The level logic of the Ready output",
         }],	  
        'shutter_level':	  
        [[PyTango.DevString,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"Shutter output level",
             'unit':"enum.",
             'format':"",
             'description':"The level logic of the  Shutter output",
         }],	  
        'gate_mode':	  
        [[PyTango.DevString,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"The Gate input mode",
             'unit':"enum.",
             'format':"",
             'description':"",
         }],	  
        'gate_level':	  
        [[PyTango.DevString,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"",
             'unit':"",
             'format':"",
             'description':"",
         }],	  
        'trigger_level':	  
        [[PyTango.DevString,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
         {
             'label':"",
             'unit':"",
             'format':"",
             'description':"",
         }],	  
         'dac_possible':	  
         [[PyTango.DevString,
           PyTango.SPECTRUM,
           PyTango.READ,17],
         {
             'label':"",
             'unit':"",
             'format':"",
             'description':"",
          }],	  
        'dac_name':	  
        [[PyTango.DevString,
           PyTango.SCALAR,
           PyTango.READ_WRITE],
         {
             'label':"",
             'unit':"",
             'format':"",
             'description':"",
         }],	  
         'dac_value':	  
         [[PyTango.DevLong,
           PyTango.SCALAR,
           PyTango.READ_WRITE],
         {
             'label':"",
             'unit':"",
             'format':"%xd",
             'description':"",
          }],
        }


    def __init__(self,name) :
        PyTango.DeviceClass.__init__(self,name)
        self.set_type(name)


#----------------------------------------------------------------------------
#                              Plugins
#----------------------------------------------------------------------------
from Lima.Maxipix.MpxAcq import MpxAcq

_MaxipixAcq = None

def get_control(espia_dev_nb = '0',config_path='', config_name='', reconstruction_active='true', **keys) :
    #properties are passed here as string
    global _MaxipixAcq
    global _MaxipixInterface

    if reconstruction_active.lower() == 'true': active = True
    else: active  = False
    if _MaxipixAcq is None:
        _MaxipixAcq = MpxAcq(int(espia_dev_nb), config_path, config_name, active)
        _MaxipixInterface = _MaxipixAcq.getInterface()
        
    return Core.CtControl(_MaxipixInterface)

    
def get_tango_specific_class_n_device():
    return MaxipixClass,Maxipix
