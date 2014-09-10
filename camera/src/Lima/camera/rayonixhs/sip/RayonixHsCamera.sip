//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2014
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################

namespace RayonixHs {
%TypeHeaderCode
#include <RayonixHsCamera.h>
using namespace lima;
%End

  enum DetectorStatus {
   DETECTOR_STATUS_IDLE,
   DETECTOR_STATUS_INTEGRATING,
   DETECTOR_STATUS_READOUT,
   DETECTOR_STATUS_LATENCY,
   DETECTOR_STATUS_CONFIG,
   DETECTOR_STATUS_FAULT
  };

  enum FrameMode {
    SINGLE,
    FAST_TRANSFER
  };  

  enum SignalType {
    UNKNOWN, 
    NONE,  
    EXTENDED, 
    OPTO,  
    OPTO_INVERTED,  
    CMOS, 
    CMOS_PULLDOWN,  
    CMOS_PULLUP,  
    CMOS_PULLDOWN_INVERTED,  
    CMOS_PULLUP_INVERTED,  
    SOFTWARE   
  };
  enum SignalID {
    ID_UNKNOWN,
    ID_NONE,
    ID_EXTENDED,
    ID_SHUTTER,
    ID_INTEGRATE,
    ID_FRAME,
    ID_LINE,
    ID_SHUTTER_OPENING,
    ID_SHUTTER_CLOSING,
    ID_SHUTTER_ACTIVE,
    ID_TRIGGER_RISE_WAIT,
    ID_TRIGGER_RISE_ACK,
    ID_TRIGGER_FALL_WAIT,
    ID_TRIGGER_FALL_ACK,
    ID_TRIGGER_2_RISE_WAIT,
    ID_TRIGGER_2_RISE_ACK,
    ID_INPUT_FRAME,
    ID_INPUT_GATE
  };

  enum OutputChannel {
    CHANNEL_1,
    CHANNEL_2
  };

  class Camera 
{
%TypeHeaderCode
#include <RayonixHsCamera.h>
using namespace lima;
%End
  public:
    Camera();
    ~Camera();

    void getDetectorModel(std::string &model /Out/);
    void getDetectorType(std::string &model /Out/);
		
    void getMaxImageSize(Size& max_image_size /Out/);
    void getPixelSize(double &x /Out/, double &y /Out/);

    void reset();
    void prepareAcq();
    void startAcq();
    void stopAcq();
    void getStatus(RayonixHs::DetectorStatus &status /Out/);

    void getTrigMode(TrigMode &mode /Out/);
    void setTrigMode(TrigMode mode);
    bool checkTrigMode(TrigMode mode);

    void getImageType(ImageType &type /Out/);
    void setImageType(ImageType type);

    void setNbFrames(int nb_frames);
    void getNbFrames(int& nb_frames /Out/);
    int getNbAcquiredFrames();

    void setExpTime(double exp_time);
    void getExpTime(double& exp_time /Out/);

    void setLatTime(double lat_time);
    void getLatTime(double& lat_time /Out/);

    void setBin(const Bin& bin);
    void getBin(Bin& bin /Out/);
    void checkBin(Bin& bin /In,Out/);

    void setFrameDim(const FrameDim& frame_dim);
    void getFrameDim(FrameDim& frame_dim /Out/);

    void getNewBackgroundNeeded(bool& needed /Out/);
    void acquireNewBackground(bool block = false, int n_backgrounds = 0);
  };


  //LIBRAYONIXHS_API std::ostream& operator <<(std::ostream& os, Camera& simu);

}; //namespace rayonixhs
