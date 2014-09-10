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
import os.path
import gc
import types, time
import thread

from Lima.Core import *
from Lima import Espia

from MpxCommon import *

import MpxDacs
import MpxDetConfig
import MpxChipConfig


class MpxAcq:
    DEB_CLASS(DebModApplication, "MpxAcq")
    
    @DEB_MEMBER_FUNCT
    def __init__(self, espia_dev_nb, config_path = None, config_name = None, reconstruction=False):
    
        self.__cam_inited = False
	
	self.init(espia_dev_nb, config_path, config_name, reconstruction)

    @DEB_MEMBER_FUNCT
    def init(self, espia_dev_nb, config_path, config_name, reconstruction):	
	self.__edev= Espia.Dev(espia_dev_nb)
	self.__edev.resetLink()
	self.__eser= Espia.SerialLine(self.__edev)
	self.__eacq= Espia.Acq(self.__edev)

	self.__ebuf= Espia.BufferMgr(self.__eacq)
	self.__mbuf= BufferCtrlMgr(self.__ebuf)

	self.__pser= Maxipix.PriamSerial(self.__eser)
	self.__pacq= Maxipix.PriamAcq(self.__pser)
	self.__mdet= Maxipix.MaxipixDet()

	self.__hwInt= Maxipix.Interface(self.__eacq, self.__mbuf, 
					self.__pacq, self.__mdet)
	self.__reconstruct= None
	self.__reconstructType = Maxipix.MaxipixReconstruction.RAW	
	
	self.cfgPath= None
	self.mpxCfg= None
	self.mpxDacs= None
	self.priamPorts= None

        self.mpxFillModes = {'RAW':Maxipix.MaxipixReconstruction.RAW,
                             'ZERO':Maxipix.MaxipixReconstruction.ZERO,
                             'DISPATCH':Maxipix.MaxipixReconstruction.DISPATCH,
                             'MEAN':Maxipix.MaxipixReconstruction.MEAN,
                             }
       
       
        if config_path is not None and config_name is not None:
            self.setPath(config_path)
            self.loadConfig(config_name, reconstruction)

        self.__cam_inited = True
        	
    @DEB_MEMBER_FUNCT
    def __del__(self):
        if self.__cam_inited:
            del self.__hwInt; gc.collect()
            if self.__reconstruct is not None:
                del self.__reconstruct
            del self.__mdet; gc.collect()
            del self.__pacq; gc.collect()
            del self.__pser; gc.collect()
            del self.__mbuf; gc.collect()
            del self.__ebuf; gc.collect()
            del self.__eacq; gc.collect()
            del self.__eser; gc.collect()
            del self.__edev; gc.collect()


    def getInterface(self):
        if self.__cam_inited:
            return self.__hwInt
	else:
	    raise MpxError("init() method must be called first")

    def getPriamAcq(self):
        if self.__cam_inited:
	    return self.__pacq
	else:
	    raise MpxError("init() method must be called first")

    def getPriamSerial(self):
        if self.__cam_inited:
           return self.__pser
        else:
	    raise MpxError("init() method must be called first")


    def getEspiaAcq(self):
        if self.__cam_inited:
            return self.__eacq
	else:
	    raise MpxError("init() method must be called first")

    def getEspiaSerial(self):
        if self.__cam_inited:
            return self.__eser
	else:
	    raise MpxError("init() method must be called first")

    def getEspiaDev(self):
        if self.__cam_inited:
            return self.__edev
	else:
	    raise MpxError("init() method must be called first")

    @DEB_MEMBER_FUNCT
    def setFillMode(self, fillMode):
        
        if self.__cam_inited:
            if self.__reconstruct:
                if fillMode not in self.mpxFillModes.values():
                    raise MpxError("invalid reconstruction fill mode %d"%fillMode)
                else:
                    self.__reconstructType = fillMode
                    self.__reconstruct.setType(fillMode)		 
	else:
	    raise MpxError("init() method must be called first")

    @DEB_MEMBER_FUNCT
    def getFillMode(self):
        if self.__cam_inited:
            return self.__reconstructType
	else:
	    raise MpxError("init() method must be called first")


    @DEB_MEMBER_FUNCT
    def setPath(self, path):
	spath= os.path.normpath(path)
        if not os.path.isdir(spath):
            raise MpxError("<%s> is not a directory"%path)
        if not os.access(spath, os.R_OK):
            raise MpxError("no read permission on <%s>"%path)
        self.cfgPath= spath

    @DEB_MEMBER_FUNCT
    def loadConfig(self,name, reconstruction=True) :
	thread.start_new_thread(MpxAcq._loadConfig,(self,self.__hwInt,name,reconstruction))

    def _loadConfig(cnt,hwInterface, name, reconstruction):
	try:
		hwInterface.setConfigFlag(True)	
		cnt.loadDetConfig(name, reconstruction)
		cnt.loadChipConfig(name)
        	# Need to inform afterward the hwInterface about new ranges
        	# which are calculated once the configs have been loaded.
        	# By callback the CtAcquisition will be refreshed too.
        	hwInterface.updateValidRanges()
	finally:
		hwInterface.setConfigFlag(False)
        
        print "\n\nEnd of configuration, Maxipix is ready !"

    @DEB_MEMBER_FUNCT
    def loadDetConfig(self, name, reconstruction):
	detConfig= MpxDetConfig.MpxDetConfig()
	detConfig.setPath(self.cfgPath)
	print "Loading Detector Config <%s> ..."%name
	detConfig.loadConfig(name)

	self.cfgName= detConfig.getName()
	self.cfgFilename= detConfig.getFilename()

	self.mpxCfg= detConfig.getMpxCfg()
        
	self.priamPorts= detConfig.getPriamPorts()
	self.mpxDacs= detConfig.getDacs()
        self.mpxDacs.setPriamPars(self.__pacq, self.priamPorts)
        
	print "Setting PRIAM configuration ..."
	self.__pacq.setup(self.mpxCfg["version"], self.mpxCfg["polarity"],
			  self.mpxCfg["frequency"], self.mpxDacs.getFsrString(1))
        
	self.__pacq.setParalellReadout(self.priamPorts)
	self.__pacq.setImageMode(Maxipix.PriamAcq.NORMAL)
	self.__pacq.setGateMode(Maxipix.PriamAcq.INACTIVE)
	self.__pacq.setShutterMode(Maxipix.PriamAcq.SEQUENCE)
	self.__pacq.setIntervalTime(0.)
	self.__pacq.setShutterTime(0.)

        # Ask Dacs obj to apply the new FSR registers (DACS values)
	self.mpxDacs.applyChipDacs(0)

        # Reconstruction can be not apply if requested
        self.setReconstructionActive(reconstruction)

    @DEB_MEMBER_FUNCT
    def setReconstructionActive(self,active):

        #apply read config detector size
        self.__mdet.setChipsRotation(self.mpxCfg["rotations"])
	self.__mdet.setVersion(self.mpxCfg["version"])	
        xgap = self.mpxCfg["xgap"];  ygap = self.mpxCfg["ygap"]
        xchip = self.mpxCfg["xchip"]; ychip = self.mpxCfg["ychip"]
        self.__mdet.setNbChip(xchip, ychip)        
        self.__mdet.setPixelGap(xgap, ygap)
        # get the default reconstruction task (object) set from the read config parameters
        # but first inform the hwInt to no reconstruction
        if self.__reconstruct is not None:
            del self.__reconstruct
        self.__hwInt.setReconstructionTask(None)            
        self.__reconstruct = self.__mdet.getReconstructionTask()

        
        d_model = self.__mdet.getDetectorModel()
        
        # now decide to active or not a reconstruction
        # accord to either the config file and/or the "active" flag
        if active and self.__reconstruct is not None:
            print "Image reconstruction is switched ON, model:" , d_model
            self.__hwInt.setReconstructionTask(self.__reconstruct)
        else:
            if active and self.__reconstruct is None:
                print "Image reconstruction is switched OFF (active=true, config=off), model: ", d_model
            else:
                print "Image reconstruction is switched OFF (active=false), model: ", d_model            
                
            # flatten detector if it's 2x2
            if xchip == 2 and ychip == 2:
                xchip = 4; ychip = 1
                print " \t\t--> 2x2 flatten out to 4x1"
            # remove the gap
            xgap = ygap = 0
                
        # Update now det image size and this will call maxImageSizeChanged callback to update CtImage
        self.__mdet.setNbChip(xchip, ychip)        
        self.__mdet.setPixelGap(xgap, ygap)

    @DEB_MEMBER_FUNCT
    def loadChipConfig(self, name):
	self.chipCfg= MpxChipConfig.MpxPixelConfig(self.mpxCfg["version"], 
						self.mpxCfg["nchip"])
	self.chipCfg.setPath(self.cfgPath)
	self.chipCfg.loadConfig(name)

	self.applyPixelConfig(0)

    @DEB_MEMBER_FUNCT
    def applyPixelConfig(self, chipid):
	if chipid==0:
	    for idx in range(self.mpxCfg["nchip"]):
	        scfg= self.chipCfg.getMpxString(idx+1)
	        print "Loading Chip Config #%d ..."%(idx+1)
	        self.__pacq.setChipCfg(self.priamPorts[idx], scfg)
	else:
	    port= self.__getPriamPort(chipid)
	    scfg= self.chipCfg.getMpxString(chipid)
	    print "Loading Chip Config #%d ..."%(chipid)
	    self.__pacq.setChipCfg(port, scfg)

       # After chip(s) configuration a chip pixel value is needed and
        # can only be done by reading the chips, this can be done with
        # a dummy acquisition
        print "Resetting chip(s) pixels ..."
        exptime = self.__pacq.getExposureTime()
        nbframes = self.__pacq.getNbFrames()
        self.__pacq.setExposureTime(0.01)
        self.__pacq.setNbFrames(1)
        self.__pacq.startAcq()
        time.sleep(0.1)
        if self.__pacq.getStatus() != DetIdle:
            self.__pacq.stopAcq()
            raise MpxError("Cannot reset chip(s) after config.")
	self.__pacq.stopAcq()
        self.__pacq.setExposureTime(exptime)
        self.__pacq.setNbFrames(nbframes)

	     
    def __getPriamPort(self, chipid):
	if chipid <= 0 or chipid > self.mpxCfg["nchip"]:
            raise MpxError("<%d> is not a valid chipID"%chipid)
	return self.priamPorts[chipid-1]
	
