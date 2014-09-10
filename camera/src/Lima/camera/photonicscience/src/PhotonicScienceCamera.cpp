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
//############################################################################
#include <direct.h>
#include "PhotonicScienceCamera.h"

using namespace lima;
using namespace lima::PhotonicScience;

//---------------------------
//- utility thread
//---------------------------

class Camera::_AcqThread : public Thread
{
    DEB_CLASS_NAMESPC(DebModCamera, "Camera", "_AcqThread");
    public:
        _AcqThread(Camera &aCam);
    virtual ~_AcqThread();
    
    protected:
        virtual void threadFunction();
    
    private:
        Camera&    m_cam;
};


//---------------------------
//- Ctor
//---------------------------
Camera::Camera(const std::string &dllName,
	       const std::string &camera_files_path_par) :
  
  m_acq_thread(NULL),
  m_trigger_mode(IntTrig),
  m_exp_time(1.),
  m_image_number(0),
  m_nb_frames(1),
  m_thread_running(false),
  m_wait_flag(true),
  m_quit(false),
  m_shutter_mode(SLOW),
  m_shutter_enable(false)
{
  DEB_CONSTRUCTOR();
  m_acq_thread = new _AcqThread(*this);
  m_acq_thread->start();
  while(m_thread_running)
    m_cond.wait();

  //Convert string to WideChar
  int len = MultiByteToWideChar(CP_ACP,0,dllName.c_str(), dllName.length() + 1,0,0);
  wchar_t* dllName_wchar = new wchar_t[len];
  MultiByteToWideChar(CP_ACP, 0, dllName.c_str(), dllName.length() + 1,dllName_wchar,len);
  
  m_hDLL = LoadLibrary(dllName_wchar);
  delete [] dllName_wchar;

  if(!m_hDLL)
    THROW_HW_ERROR(Error) << "Can't load library: " << dllName;

  //Part
  m_Init = (Init)GetProcAddress(m_hDLL,"PSL_VHR_Init"); 
  m_get_camera_identifier = (get_camera_identifier)GetProcAddress(m_hDLL,"PSL_VHR_get_camera_identifier");
  m_ReadMaxImageWidth = (ReadMaxImageWidth)GetProcAddress(m_hDLL,"PSL_VHR_ReadMaxImageWidth");
  m_ReadMaxImageHeight = (ReadMaxImageHeight)GetProcAddress(m_hDLL,"PSL_VHR_ReadMaxImageHeight");
  
  m_enable_noise_reduction = (enable_noise_reduction)GetProcAddress(m_hDLL,"PSL_VHR_enable_noise_reduction");
  m_enable_sharpening = (enable_sharpening)GetProcAddress(m_hDLL,"PSL_VHR_enable_sharpening");

  m_set_shutter_parameters = (set_shutter_parameters)GetProcAddress(m_hDLL,"PSL_VHR_set_shutter_parameters");
  m_set_virtual_shutter_mode = (set_virtual_shutter_mode)GetProcAddress(m_hDLL,"PSL_VHR_set_virtual_shutter_mode"); 
  m_enable_virtual_shuttering = (enable_virtual_shuttering)GetProcAddress(m_hDLL,"PSL_VHR_enable_virtual_shuttering");

  m_Set_subarea_and_binning = (Set_subarea_and_binning)GetProcAddress(m_hDLL,"PSL_VHR_Set_subarea_and_binning");
  m_WriteExposure = (WriteExposure)GetProcAddress(m_hDLL,"PSL_VHR_WriteExposure");
  m_Set_trigger_mode = (Set_trigger_mode)GetProcAddress(m_hDLL,"PSL_VHR_Set_trigger_mode");
  m_set_required_operating_temp = (set_required_operating_temp)GetProcAddress(m_hDLL,"PSL_VHR_set_required_operating_temp");
  m_read_current_temp = (read_current_temp)GetProcAddress(m_hDLL,"PSL_VHR_read_current_temp");

  m_Is_intensifier_fitted = (Is_intensifier_fitted)GetProcAddress(m_hDLL,"PSL_VHR_Is_intensifier_fitted");
  m_WriteIntensifierGain = (WriteIntensifierGain)GetProcAddress(m_hDLL,"PSL_VHR_WriteIntensifierGain");

  m_Snap_and_return = (Snap_and_return)GetProcAddress(m_hDLL,"PSL_VHR_Snap_and_return");
  m_abort_snap = (abort_snap)GetProcAddress(m_hDLL,"PSL_VHR_abort_snap");
  m_Get_snap_status = (Get_snap_status)GetProcAddress(m_hDLL,"PSL_VHR_Get_snap_status");
  m_grab_exposed_image = (grab_exposed_image)GetProcAddress(m_hDLL,"PSL_VHR_grab_exposed_image");

  m_Get_image_pointer = (Get_image_pointer)GetProcAddress(m_hDLL,"PSL_VHR_Get_image_pointer");
  m_Return_width = (Return_width)GetProcAddress(m_hDLL,"PSL_VHR_Return_width");
  m_Return_height = (Return_height)GetProcAddress(m_hDLL,"PSL_VHR_Return_height");

  m_Free = (Free)GetProcAddress(m_hDLL,"PSL_VHR_Free");

  std::string camera_files_path;
  if(camera_files_path_par.empty())
    {
      char pwdBuffer[1024];
      _getcwd(pwdBuffer,sizeof(pwdBuffer));
      size_t slashPos = dllName.rfind("\\");
      if(slashPos != std::string::npos)
	{
	  camera_files_path = pwdBuffer;
	  camera_files_path += '\\';
	  camera_files_path += dllName.substr(0,slashPos);
	  camera_files_path += '\\';
	  camera_files_path += "PSL_camera_files";
	}
    }
  else
    camera_files_path = camera_files_path_par;
  int initStatus = -1;
  if(m_Init)
    initStatus = m_Init((char*)camera_files_path.c_str());

  if(initStatus)
    THROW_HW_ERROR(Error) << "Camera init failed: " << DEB_VAR2(dllName,camera_files_path);

  int maxWidth,maxHeight;
  m_ReadMaxImageWidth(&maxWidth);
  m_ReadMaxImageHeight(&maxHeight);
  m_Set_subarea_and_binning(0,0,maxWidth - 1,maxHeight - 1,1,1);
}

//---------------------------
//- Dtor
//---------------------------
Camera::~Camera()
{
  DEB_DESTRUCTOR();
  if(m_Free) m_Free();

  FreeLibrary(m_hDLL);
}
//---------------------------
//- Camera::start()
//---------------------------
void Camera::startAcq()
{
  DEB_MEMBER_FUNCT();

  m_image_number=0;
  
  StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_obj.getBuffer();
  buffer_mgr.setStartTimestamp(Timestamp::now());

  AutoMutex aLock(m_cond.mutex());
  m_wait_flag = false;
  m_cond.broadcast();
  // Wait that Acq thread start if it's an external trigger 
  while(m_trigger_mode == ExtTrigMult && !m_thread_running)
    m_cond.wait();
}
//---------------------------
//- Camera::stopAcq()
//---------------------------
void Camera::stopAcq()
{
  AutoMutex aLock(m_cond.mutex());
  m_wait_flag = true;
  m_abort_snap();
  while(m_thread_running)
    m_cond.wait();
}
//---------------------------
//- Camera::_AcqThread::threadFunction()
//---------------------------
void Camera::_AcqThread::threadFunction()
{
  DEB_MEMBER_FUNCT();
  AutoMutex aLock(m_cam.m_cond.mutex());
  StdBufferCbMgr& buffer_mgr = m_cam.m_buffer_ctrl_obj.getBuffer();

  while(!m_cam.m_quit)
    {
      while(m_cam.m_wait_flag && !m_cam.m_quit)
	{
	  DEB_TRACE() << "Wait";
	  m_cam.m_thread_running = false;
	  m_cam.m_cond.broadcast();
	  m_cam.m_cond.wait();
	}
      DEB_TRACE() << "Run";
      m_cam.m_thread_running = true;
      if(m_cam.m_quit) return;
    
      m_cam.m_cond.broadcast();
      aLock.unlock();
      
      bool continueFlag = true;
      while(continueFlag && 
	    (!m_cam.m_nb_frames || m_cam.m_image_number < m_cam.m_nb_frames))
	{
	  if(m_cam.m_trigger_mode != ExtTrigMult)
	    m_cam.m_Snap_and_return();
	  while(continueFlag)
	    {
	      bool finishedFlag = m_cam.m_Get_snap_status();
	      if(finishedFlag)
		{
		  aLock.lock();
		  m_cam.m_grab_exposed_image();
		  unsigned short* aSrcPt = m_cam.m_Get_image_pointer();
		  void* aDstPt = buffer_mgr.getFrameBufferPtr(m_cam.m_image_number);
		  const FrameDim& fDim = buffer_mgr.getFrameDim();
		  memcpy(aDstPt,aSrcPt,fDim.getMemSize());
		  aLock.unlock();
		  HwFrameInfoType frame_info;
		  frame_info.acq_frame_nb = m_cam.m_image_number;
		  continueFlag = buffer_mgr.newFrameReady(frame_info);
		  ++m_cam.m_image_number;
		  break;
		}
	      else
		{
		  AutoMutex aLock(m_cam.m_cond.mutex());
		  continueFlag = !m_cam.m_wait_flag;
		}
	    }
	}
      aLock.lock();
      m_cam.m_wait_flag = true;
      /** Camera don 't finished well when Ext trigger is used
	  Force an other soft acquisition at the end.
	  This is really bad but....
	  @todo ask for a fix
      */
      if(m_cam.m_trigger_mode == ExtTrigMult)
	{
	  m_cam.m_Set_trigger_mode(1);
	  m_cam.setExpTime(1e-6);
	  m_cam.m_Snap_and_return();
	}
    }
}

//-----------------------------------------------------
//
//-----------------------------------------------------
Camera::_AcqThread::_AcqThread(Camera &aCam) :
  m_cam(aCam)
{
  pthread_attr_setscope(&m_thread_attr,PTHREAD_SCOPE_PROCESS);
}
//-----------------------------------------------------
//
//-----------------------------------------------------

Camera::_AcqThread::~_AcqThread()
{
  AutoMutex aLock(m_cam.m_cond.mutex());
  m_cam.m_quit = true;
  m_cam.m_cond.broadcast();
  aLock.unlock();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getDetectorImageSize(Size& size)
{
  DEB_MEMBER_FUNCT();

  int maxWidth,maxHeight;
  m_ReadMaxImageWidth(&maxWidth);
  m_ReadMaxImageHeight(&maxHeight);
  size = Size(maxWidth - 1,maxHeight - 1);

  DEB_RETURN() << DEB_VAR1(size);
}


//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getImageType(ImageType& type)
{
  DEB_MEMBER_FUNCT();
  type = Bpp16;        
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setImageType(ImageType type)
{
  DEB_MEMBER_FUNCT();
  if(type != Bpp16)
    THROW_HW_ERROR(Error) << "Cannot change the format of the camera !";
}
//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getDetectorType(std::string& type)
{
  DEB_MEMBER_FUNCT();
  type = "PhotonicScience";
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getDetectorModel(std::string& type)
{
  DEB_MEMBER_FUNCT();
  type = m_get_camera_identifier();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
HwBufferCtrlObj* Camera::getBufferObj()
{
  return &m_buffer_ctrl_obj;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setTrigMode(TrigMode mode)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(mode);
  
  int tmode;
  switch(mode)
    {
    case ExtTrigMult: 	tmode = 2;break;
    default: 		tmode = 1;break;
    }

  if(!m_Set_trigger_mode(tmode))
    THROW_HW_ERROR(Error) << "Can't set triggerMode:" << DEB_VAR1(mode);
  m_trigger_mode = mode;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getTrigMode(TrigMode& mode)
{
  DEB_MEMBER_FUNCT();
  mode = m_trigger_mode;
  DEB_RETURN() << DEB_VAR1(mode);
}


//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setExpTime(double exp_time)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(exp_time);
    
  if(!m_WriteExposure(int(exp_time * 1000.)))
    THROW_HW_ERROR(Error) << "Can't set exposure to : " << DEB_VAR1(exp_time);

  m_exp_time = exp_time;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getExpTime(double& exp_time)
{
  DEB_MEMBER_FUNCT();
  exp_time = m_exp_time;            
  DEB_RETURN() << DEB_VAR1(exp_time);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setLatTime(double lat_time)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(lat_time);

  if(lat_time != 0.)
    THROW_HW_ERROR(Error) << "Latency not managed";
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getLatTime(double& lat_time)
{
  DEB_MEMBER_FUNCT();
  lat_time = 0;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getExposureTimeRange(double& min_expo, double& max_expo) const
{
  DEB_MEMBER_FUNCT();
  min_expo = 1e-9;		// Don't know
  max_expo = 1e9;		// Don't know
  DEB_RETURN() << DEB_VAR2(min_expo, max_expo);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getLatTimeRange(double& min_lat, double& max_lat) const
{   
  DEB_MEMBER_FUNCT();
  max_lat = min_lat= 0;
  DEB_RETURN() << DEB_VAR2(min_lat, max_lat);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setNbFrames(int nb_frames)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(nb_frames);
  m_nb_frames = nb_frames;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getNbFrames(int& nb_frames)
{
  DEB_MEMBER_FUNCT();
  nb_frames = m_nb_frames;
  DEB_RETURN() << DEB_VAR1(nb_frames);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getNbHwAcquiredFrames(int &nb_acq_frames)
{ 
  DEB_MEMBER_FUNCT();    
  nb_acq_frames = m_image_number;
}

/** @brief accept all roi for the moment
 */
void Camera::checkRoi(const Roi& set_roi, Roi& hw_roi)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(set_roi);
  hw_roi = set_roi;

  DEB_RETURN() << DEB_VAR1(hw_roi);
}

void Camera::setRoi(const Roi& set_roi)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(set_roi);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getRoi(Roi& hw_roi)
{
  DEB_MEMBER_FUNCT();
    
  DEB_RETURN() << DEB_VAR1(hw_roi);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::checkBin(Bin &aBin)
{
  DEB_MEMBER_FUNCT();

  DEB_RETURN() << DEB_VAR1(aBin);
}
//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setBin(const Bin &aBin)
{
  DEB_MEMBER_FUNCT();

  DEB_RETURN() << DEB_VAR1(aBin);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getBin(Bin &aBin)
{
  DEB_MEMBER_FUNCT();
  DEB_RETURN() << DEB_VAR1(aBin);
}

//---------------------------
//- Camera::reset()
//---------------------------
void Camera::reset(void)
{
    
}
bool Camera::isAcqRunning() const
{
  AutoMutex aLock(m_cond.mutex());
  return m_thread_running;
}

Camera::ShutterMode Camera::getShutterMode() const
{
  AutoMutex aLock(m_cond.mutex());
  return m_shutter_mode;
}
void Camera::setShutterMode(ShutterMode mode)
{
  AutoMutex aLock(m_cond.mutex());
  m_set_virtual_shutter_mode(mode);
  m_shutter_mode = mode;
}
bool Camera::isVirtualShutterEnable() const
{
  AutoMutex aLock(m_cond.mutex());
  return m_shutter_enable;
}

void Camera::setVirtualShutterEnable(bool flag)
{
  AutoMutex aLock(m_cond.mutex());
  m_enable_virtual_shuttering(flag);
  m_shutter_enable = flag;
}

void Camera::setShutterParameters(int iShutterStartOffset,int iShutterEndOffset,int iShutterPedestal)
{
  m_set_shutter_parameters(iShutterStartOffset,iShutterEndOffset,iShutterPedestal);
}

void Camera::setOperatingTemp(int temp)
{
  DEB_MEMBER_FUNCT();
  
  if(!m_set_required_operating_temp(temp))
    THROW_HW_ERROR(Error) << "Can't set temperature to value:" << DEB_VAR1(temp);
}

int Camera::readCurrentTemp() const
{
  return m_read_current_temp();
}

void Camera::setIntensifierGain(int gain)
{
  DEB_MEMBER_FUNCT();
  
  if(!m_Is_intensifier_fitted || !m_Is_intensifier_fitted())
    THROW_HW_ERROR(Error) << "Camera don't have Intensifier";

  if(!m_WriteIntensifierGain(gain))
    THROW_HW_ERROR(Error) << "Can't set gain to: " << DEB_VAR1(gain);
}
//---------------------------
