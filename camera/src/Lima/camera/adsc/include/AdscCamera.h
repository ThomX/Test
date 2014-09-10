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
#ifndef ADSCCAMERA_H
#define ADSCCAMERA_H

#include "HwBufferMgr.h"
#include "AdscApi.h"
#include "ThreadUtils.h"
#include "SizeUtils.h"
#include "Debug.h"

#include "DetconFcns.h"
#include "DetconPar.h"
#include "DetconState.h"

#include <ostream>

namespace lima
{
namespace Adsc
{

class  Camera
{
	DEB_CLASS_NAMESPC(DebModCamera, "Camera", "Adsc");
public:
	enum Status
	{
		Ready, Exposure, Readout, Latency,
	};

	Camera();
	~Camera();

	
	void    startAcq();
	void    stopAcq();

	void    setNbFrames(int  nb_frames);
	void    getNbFrames(int& nb_frames);

	void    setExpTime(double  exp_time);
	void    getExpTime(double& exp_time);

	void    setLatTime(double  lat_time);
	void    getLatTime(double& lat_time);

	void    setBin(const Bin& bin);
	void    getBin(Bin& bin);
	void    checkBin(Bin& bin);

	void    setFrameDim(const FrameDim& frame_dim);
	void    getFrameDim(FrameDim& frame_dim);
	
	Status  getStatus();
	int     getNbAcquiredFrames();

	void    getMaxImageSize(Size& max_image_size);
	void    reset();
	//ADSC specific stuff
	void 	setHeaderParameters(const std::string& header);	
	void	setStoredImageDark(bool value);
	bool	getStoredImageDark(void);
	void    setImageKind(int image_kind);
	int	    getImageKind(void);	
	void    setLastImage(int last_image);
	int	    getLastImage(void);	

	void                setFileName(const std::string& name);
	const std::string&  getFileName(void);
	void                setImagePath(const std::string& path);
	const std::string&  getImagePath(void);

private:
	class AdscThread : public CmdThread
	{
    DEB_CLASS_NAMESPC(DebModCamera, "AdscThread", "Adsc");
    public:
      enum
      { // Status
    	  Ready = MaxThreadStatus, Exposure, Readout, Latency,
      };
  
      enum 
      { // Cmd 
    	  StartAcq = MaxThreadCmd, StopAcq,
      };
      
      AdscThread(Camera& adsc);
  
      virtual void 	start();
      
      int 			getNbAcquiredFrames();
      bool 			m_force_stop;
  
    protected:
      virtual void 	init();
      virtual void 	execCmd(int cmd);
    private:
      void 			execStartAcq();
      Camera* 		m_adsc;
      int 			m_acq_frame_nb;
	};


	friend class AdscThread;

	void init();

	AdscApi 		m_adsc_api;
    mutable Cond    m_cond;
	double		  	m_exp_time;
	double		  	m_lat_time;
	int		      	m_nb_frames;
	int		      	m_flp_kind;
	int		      	m_bin_used;
	int		      	m_adc_used;
	int		      	m_output_raws;
	int		      	m_no_transform;
	int		      	m_use_stored_dark;
	int		      	m_last_image;
    bool 			m_use_dw;
	std::string		m_filename;
	std::string		m_path;
	
	AdscThread 		m_thread;
};

 std::ostream& operator <<(std::ostream& os, Camera& adsc);

} // namespace Adsc
} // namespace lima

#endif // ADSCCAMERA_H
