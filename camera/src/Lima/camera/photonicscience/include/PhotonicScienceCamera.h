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
#ifndef PHOTONICSCIENCECAMERA_H
#define PHOTONICSCIENCECAMERA_H

#include "Debug.h"
#include "SizeUtils.h"
#include "HwBufferMgr.h"
#include "PhotonicScienceCompatibility.h"

namespace lima
{
  namespace PhotonicScience
  {
    //DLL typedef
    typedef int (*Init)(char*);
    typedef char* (*get_camera_identifier)();
    typedef bool (*ReadMaxImageWidth)(int*);
    typedef bool (*ReadMaxImageHeight)(int*);
  
    typedef void (*enable_noise_reduction)(bool);
    typedef void (*enable_sharpening)(bool);

    typedef bool (*set_shutter_parameters)(int,int,int);
    typedef void (*set_virtual_shutter_mode)(int); 
    typedef bool (*enable_virtual_shuttering)(bool);

    typedef bool (*Set_subarea_and_binning)(int,int,int,int,int,int);
    typedef bool (*WriteExposure)(int);
    typedef bool (*Set_trigger_mode)(int);
    typedef bool (*set_required_operating_temp)(int);
    typedef int (*read_current_temp)();
    
    typedef bool (*Is_intensifier_fitted)();
    typedef bool (*WriteIntensifierGain)(unsigned short);

    typedef bool (*Snap_and_return)();
    typedef bool (*abort_snap)();
    typedef bool (*Get_snap_status)();
    typedef bool (*grab_exposed_image)();

    typedef unsigned short* (*Get_image_pointer)();
    typedef int (*Return_width)();
    typedef int (*Return_height)();

    typedef void (*Free)();

    /*******************************************************************
     * \class Camera
     * \brief object controlling the photonicscience camera via Pylon driver
     *******************************************************************/
    class LIBPHOTONICSCIENCE_API Camera
    {
      DEB_CLASS_NAMESPC(DebModCamera, "Camera", "PhotonicScience");
      friend class Interface;
    public:
      Camera(const std::string &dllName,
	     const std::string &camera_files_path = "");
      ~Camera();

      void startAcq();
      void stopAcq();
    
      // -- detector info object
      void getImageType(ImageType& type);
      void setImageType(ImageType type);

      void getDetectorType(std::string& type);
      void getDetectorModel(std::string& model);
      void getDetectorImageSize(Size& size);
    
      // -- Buffer control object
      HwBufferCtrlObj* getBufferObj();
    
      //-- Synch control object
      void setTrigMode(TrigMode  mode);
      void getTrigMode(TrigMode& mode);
    
      void setExpTime(double  exp_time);
      void getExpTime(double& exp_time);

      void setLatTime(double  lat_time);
      void getLatTime(double& lat_time);

      void getExposureTimeRange(double& min_expo, double& max_expo) const;
      void getLatTimeRange(double& min_lat, double& max_lat) const;    

      void setNbFrames(int  nb_frames);
      void getNbFrames(int& nb_frames);
      void getNbHwAcquiredFrames(int &nb_acq_frames);

      void checkRoi(const Roi& set_roi, Roi& hw_roi);
      void setRoi(const Roi& set_roi);
      void getRoi(Roi& hw_roi);    

      void checkBin(Bin&);
      void setBin(const Bin&);
      void getBin(Bin&);

      void reset(void);

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
    private:
      class _AcqThread;
      friend class _AcqThread;

      //Function pointer of the library
      Init 				m_Init;
      get_camera_identifier 		m_get_camera_identifier;
      ReadMaxImageWidth 		m_ReadMaxImageWidth;
      ReadMaxImageHeight 		m_ReadMaxImageHeight;
  
      enable_noise_reduction 		m_enable_noise_reduction;
      enable_sharpening 		m_enable_sharpening;

      set_shutter_parameters 		m_set_shutter_parameters;
      set_virtual_shutter_mode 		m_set_virtual_shutter_mode;
      enable_virtual_shuttering 	m_enable_virtual_shuttering;

      Set_subarea_and_binning 		m_Set_subarea_and_binning;
      WriteExposure 			m_WriteExposure;
      Set_trigger_mode 			m_Set_trigger_mode;
      set_required_operating_temp 	m_set_required_operating_temp;
      read_current_temp 		m_read_current_temp;

      Is_intensifier_fitted		m_Is_intensifier_fitted;
      WriteIntensifierGain		m_WriteIntensifierGain;

      Snap_and_return 			m_Snap_and_return;
      abort_snap 			m_abort_snap;
      Get_snap_status 			m_Get_snap_status;
      grab_exposed_image 		m_grab_exposed_image;

      Get_image_pointer 		m_Get_image_pointer;
      Return_width 			m_Return_width;
      Return_height 			m_Return_height;

      Free 				m_Free;

      // Variables
      HINSTANCE 			m_hDLL;
      _AcqThread*			m_acq_thread;
      TrigMode				m_trigger_mode;
      double				m_exp_time;
      int				m_image_number;
      int				m_nb_frames;
      bool				m_thread_running;
      bool				m_wait_flag;
      bool				m_quit;
      ShutterMode			m_shutter_mode;
      bool				m_shutter_enable;
      SoftBufferCtrlObj			m_buffer_ctrl_obj;
      mutable Cond			m_cond;
    };
  } // namespace PhotonicScience
} // namespace lima


#endif // PHOTONICSCIENCECAMERA_H
