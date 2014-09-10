//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2011
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
namespace PhotonicScience
{
  class Camera
  {
%TypeHeaderCode
#include <PhotonicScienceCamera.h>
%End
  public:
    Camera(const std::string &dllName,const std::string &camera_files_path = "");
    ~Camera();

    void startAcq();
    void stopAcq();
    
    // -- detector info object
    void getImageType(ImageType& type /Out/);
    void setImageType(ImageType type);

    void getDetectorType(std::string& type /Out/);
    void getDetectorModel(std::string& model /Out/);
    void getDetectorImageSize(Size& size /Out/);
    
    //-- Synch control object
    void setTrigMode(TrigMode  mode);
    void getTrigMode(TrigMode& mode /Out/);
    
    void setExpTime(double  exp_time);
    void getExpTime(double& exp_time /Out/);

    void setLatTime(double  lat_time);
    void getLatTime(double& lat_time /Out/);

    void getExposureTimeRange(double& min_expo /Out/, double& max_expo /Out/) const;
    void getLatTimeRange(double& min_lat /Out/, double& max_lat /Out/) const;    

    void setNbFrames(int  nb_frames);
    void getNbFrames(int& nb_frames /Out/);
    void getNbHwAcquiredFrames(int &nb_acq_frames /Out/);

    void checkRoi(const Roi& set_roi, Roi& hw_roi /Out/);
    void setRoi(const Roi& set_roi);
    void getRoi(Roi& hw_roi /Out/);    

    void checkBin(Bin& /In,Out/);
    void setBin(const Bin&);
    void getBin(Bin& /Out/);

    void reset();

    bool isAcqRunning() const;

    // --- Photonics specifics
    // *** Virtual Shutter
    enum ShutterMode {OFF = 2,FAST = 0,SLOW = 1};
    ShutterMode getShutterMode() const;
    void setShutterMode(ShutterMode);
    
    bool isVirtualShutterEnable() const;
    void setVirtualShutterEnable(bool);
    
    void setShutterParameters(int,int,int);
    // *** Camera temperature
    void setOperatingTemp(int temp);
    int readCurrentTemp() const;
    // *** intensifier control
    void setIntensifierGain(int gain);
  };
};
