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
#ifndef MPXINTERFACE_H
#define MPXINTERFACE_H

#include "HwInterface.h"
#include "HwReconstructionCtrlObj.h"
#include "EspiaBufferMgr.h"
#include "MaxipixDet.h"
#include "PriamAcq.h"

namespace lima
{

namespace Maxipix
{

/*******************************************************************
 * \class DetInfoCtrlObj
 * \brief Control object providing Maxipix detector info interface
 *******************************************************************/

class DetInfoCtrlObj : public HwDetInfoCtrlObj
{
  DEB_CLASS_NAMESPC(DebModCamera, "DetInfoCtrlObj", "Maxipix");

  public:
    DetInfoCtrlObj(MaxipixDet& det);
    virtual ~DetInfoCtrlObj();

    virtual void getMaxImageSize(Size& size);
    virtual void getDetectorImageSize(Size& size);

    virtual void getDefImageType(ImageType& image_type);
    virtual void getCurrImageType(ImageType& image_type);
    virtual void setCurrImageType(ImageType image_type);

    virtual void getPixelSize(double& x_size, double& y_size);
    virtual void getDetectorType(std::string& type);
    virtual void getDetectorModel(std::string& model);

    virtual void registerMaxImageSizeCallback(HwMaxImageSizeCallback& cb);
    virtual void unregisterMaxImageSizeCallback(HwMaxImageSizeCallback& cb);

  private:
    MaxipixDet& m_det;
};


/*******************************************************************
 * \class BufferCtrlObj
 * \brief Control object providing Maxipix buffering interface
 *******************************************************************/

class BufferCtrlObj : public HwBufferCtrlObj
{
  DEB_CLASS_NAMESPC(DebModCamera, "BufferCtrlObj", "Maxipix");

  public:
    BufferCtrlObj(BufferCtrlMgr& buffer_mgr);
    virtual ~BufferCtrlObj();

    virtual void setFrameDim(const FrameDim& frame_dim);
    virtual void getFrameDim(      FrameDim& frame_dim);

    virtual void setNbBuffers(int  nb_buffers);
    virtual void getNbBuffers(int& nb_buffers);

    virtual void setNbConcatFrames(int  nb_concat_frames);
    virtual void getNbConcatFrames(int& nb_concat_frames);

    virtual void getMaxNbBuffers(int& max_nb_buffers);

    virtual void *getBufferPtr(int buffer_nb, int concat_frame_nb = 0);
    virtual void *getFramePtr(int acq_frame_nb);

    virtual void getStartTimestamp(Timestamp& start_ts);
    virtual void getFrameInfo(int acq_frame_nb, HwFrameInfoType& info);

    virtual void   registerFrameCallback(HwFrameCallback& frame_cb);
    virtual void unregisterFrameCallback(HwFrameCallback& frame_cb);

  private:
    BufferCtrlMgr& m_buffer_mgr;
};


/*******************************************************************
 * \class SyncCtrlObj
 * \brief Control object providing Maxipix synchronization interface
 *******************************************************************/

class SyncCtrlObj : public HwSyncCtrlObj
{
    DEB_CLASS_NAMESPC(DebModCamera, "SyncCtrlObj", "Maxipix");

  public:
    SyncCtrlObj(Espia::Acq& acq, PriamAcq& priam);
    virtual ~SyncCtrlObj();

    virtual bool checkTrigMode(TrigMode trig_mode);
    virtual void setTrigMode(TrigMode  trig_mode);
    virtual void getTrigMode(TrigMode& trig_mode);

    virtual void setExpTime(double  exp_time);
    virtual void getExpTime(double& exp_time);

    virtual void setLatTime(double  lat_time);
    virtual void getLatTime(double& lat_time);

    virtual void setNbHwFrames(int  nb_frames);
    virtual void getNbHwFrames(int& nb_frames);

    virtual void getValidRanges(ValidRangesType& valid_ranges);

  private:
    bool _checkTrigMode(TrigMode trig_modei,bool with_acq_mode = false);
    Espia::Acq& m_acq;
    PriamAcq& m_priam;
};

/*******************************************************************
 * \class ShutterCtrlObj
 * \brief Control object providing shutter interface
 *******************************************************************/

class ShutterCtrlObj : public HwShutterCtrlObj
{
  DEB_CLASS_NAMESPC(DebModCamera, "ShutterCtrlObj", "Maxipix");

public:
	ShutterCtrlObj(PriamAcq& priam);
	virtual ~ShutterCtrlObj();

	virtual bool checkMode(ShutterMode shut_mode) const;
	virtual void getModeList(ShutterModeList&  mode_list) const;
	virtual void setMode(ShutterMode  shut_mode);
	virtual void getMode(ShutterMode& shut_mode) const;

	virtual void setState(bool  shut_open);
	virtual void getState(bool& shut_open) const;

	virtual void setOpenTime (double  shut_open_time);
	virtual void getOpenTime (double& shut_open_time) const;
	virtual void setCloseTime(double  shut_close_time);
	virtual void getCloseTime(double& shut_close_time) const;

 private:
	PriamAcq& m_priam;
};


/*******************************************************************
 * \class ReconstructionCtrlObj
 * \brief Control object providing reconstruction interface
 *******************************************************************/
class ReconstructionCtrlObj : public HwReconstructionCtrlObj 
{
        DEB_CLASS_NAMESPC(DebModCamera, "ReconstructionCtrlObj", "Maxipix");
public:
        ReconstructionCtrlObj(PriamAcq& priam);	
        ~ReconstructionCtrlObj();	
	virtual LinkTask* getReconstructionTask() {return m_reconstruct_task;}	
	void setReconstructionTask(LinkTask* task);
 private :
	LinkTask* m_reconstruct_task;
	PriamAcq& m_priam;
};


/*******************************************************************
 * \class Interface
 * \brief Maxipix hardware interface
 *******************************************************************/
class Interface : public HwInterface
{
	DEB_CLASS_NAMESPC(DebModCamera, "Interface", "Maxipix");

 public:
	Interface(Espia::Acq& acq, BufferCtrlMgr& buffer_mgr, 
		  PriamAcq& priam, MaxipixDet& det);
	virtual ~Interface();

	virtual void getCapList(CapList&) const;

	virtual void reset(ResetLevel reset_level);
	virtual void prepareAcq();
	virtual void startAcq();
	virtual void stopAcq();
	virtual void getStatus(StatusType& status);
	virtual int getNbHwAcquiredFrames();
	void updateValidRanges();
        void setConfigFlag(bool flag);
	
	void setReconstructionTask(LinkTask*);
 private:
	class AcqEndCallback : public Espia::AcqEndCallback
	{
		DEB_CLASS_NAMESPC(DebModCamera, "Interface::AcqEndCallback", 
				  "Maxipix");

	public:
		AcqEndCallback(PriamAcq& priam);
		virtual ~AcqEndCallback();

	protected:
		virtual void acqFinished(const HwFrameInfoType& /*finfo*/);
	private:
		PriamAcq& m_priam;
	};

	Espia::Acq&	m_acq;
	BufferCtrlMgr&	m_buffer_mgr;
	PriamAcq&	m_priam;
	AcqEndCallback  m_acq_end_cb;

	CapList                m_cap_list;
	DetInfoCtrlObj         m_det_info;
	BufferCtrlObj          m_buffer;
	SyncCtrlObj            m_sync;
	ShutterCtrlObj         m_shutter;
	ReconstructionCtrlObj  m_reconstruction;

 	bool           m_prepare_flag;	
        bool           m_config_flag; 
};

} // namespace Maxipix

} // namespace lima

#endif // MPXINTERFACE_H