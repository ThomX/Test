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
#ifndef RAYONIXHS_H
#define RAYONIXHS_H

//#include <ostream>

#include "Debug.h"
#include "HwMaxImageSizeCallback.h"
#include "HwInterface.h"
#include "HwBufferMgr.h"
#include "SizeUtils.h"

#include "craydl.h"
#include "FrameStatusCb.h"
#include "RayonixHsSyncCtrlObj.h"

namespace lima {
namespace RayonixHs {

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

class Camera : public HwMaxImageSizeCallbackGen {
   DEB_CLASS_NAMESPC(DebModCamera, "Camera", "RayonixHs");
   friend class Interface;
   friend class FrameStatusCb;

	public:
                enum ShutterMode {
		  FRAME,
		  MANUAL
		};

		Camera();
		~Camera();

		void getDetectorModel(std::string &model);
		void getDetectorType(std::string &model);
		
		void getStatus(DetectorStatus &status);

		void getMaxImageSize(Size& max_image_size);
	        void getPixelSize(double &x, double &y);

		void reset();

		void prepareAcq();
		void startAcq();
		void stopAcq();

		void getTrigMode(TrigMode &mode);
		void setTrigMode(TrigMode mode);
		bool checkTrigMode(TrigMode mode);

		void getImageType(ImageType &type);
		void setImageType(ImageType type);

		void setNbFrames(int nb_frames);
		void getNbFrames(int& nb_frames);
		int getNbAcquiredFrames();

		void setExpTime(double exp_time);
		void getExpTime(double& exp_time);

		void setLatTime(double lat_time);
		void getLatTime(double& lat_time);

		void setBin(const Bin& bin);
		void getBin(Bin& bin);
		void checkBin(Bin& bin);

		void setFrameDim(const FrameDim& frame_dim);
		void getFrameDim(FrameDim& frame_dim);

		HwBufferCtrlObj* getBufferCtrlObj();

		// Specific Rayonix interface for configuration
		void setFrameMode(FrameMode mode);
		void getFrameMode(FrameMode &mode);
		
		void setFrameTriggerSignalType(SignalType signal_type);
		void getFrameTriggerSignalType(SignalType &signal_type);
		void setSequenceGateSignalType(SignalType signal_type);
		void getSequenceGateSignalType(SignalType &signal_type);
		
		void setOutputSignalType(OutputChannel output, SignalType signal_type);
		void getOutputSignalType(OutputChannel output, SignalType &signal_type);
		void setOutputSignalID(OutputChannel output, SignalID signal_id);
		void getOutputSignalID(OutputChannel output, SignalID &signal_id);

		void setShutterOpenDelay(double delay);
		void getShutterOpenDelay(double &delay);
		void setShutterCloseDelay(double delay);
		void getShutterCloseDelay(double &delay);
		void getShutter(bool &open);
		void setShutter(bool open);
		void setElectronicShutterEnabled(bool enable);
		void getElectronicShutterEnabled(bool &enable);
		
		void getCoolerTemperatureSetpoint(double &temperature);
		void setCoolerTemperatureSetpoint(double temperature);
		void getSensorTemperatureSetpoint(double &temperature);
		void setSensorTemperatureSetpoint(double temperature);
		void getSensorTemperature(double &temperature);

		void setCooler(bool start);
		void getCooler(bool& start);
		void setVacuumValve(bool open);
		void getVacuumValve(bool& open);
		
		void getNewBackgroundNeeded(bool& needed);
		void acquireNewBackground(bool block = false, int n_backgrounds = 0);

	private:
		void init();
		void setStatus(DetectorStatus status, bool force=false);

		SoftBufferCtrlObj m_buffer_ctrl_obj;
		
		FrameDim m_frame_dim;

		double m_exp_time;
		double m_lat_time;
		double m_int_time;

		int m_nb_frames;
		TrigMode m_trig_mode;
		Size m_max_image_size;

		DetectorStatus m_status;

		FrameStatusCb *m_frame_status_cb;
		Mutex m_mutex;
		std::map<int,HwFrameInfoType> m_pending_frames;
		int m_expected_frame_nb;

		craydl::RxDetector *m_rx_detector;
		FrameMode m_frame_mode;
		SignalType m_frame_trig_signal_type;
		SignalType m_sequ_gate_signal_type;
		volatile bool m_int_trig_mult_started;
		volatile bool m_acquiring;
		bool m_cooler, m_vac_valve;

		void frameReady(const craydl::RxFrame *pFrame);
};


//LIBRAYONIXHS_API std::ostream& operator <<(std::ostream& os, Camera& simu);

} //namespace rayonixhs
} // namespace lima

#endif // RAYONIXHS_H
