//###########################################################################
//// This file is part of LImA, a Library for Image Acquisition
////
//// Copyright (C) : 2009-2014
//// European Synchrotron Radiation Facility
//// BP 220, Grenoble 38043
//// FRANCE
////
//// This is free software; you can redistribute it and/or modify
//// it under the terms of the GNU General Public License as published by
//// the Free Software Foundation; either version 3 of the License, or
//// (at your option) any later version.
////
//// This software is distributed in the hope that it will be useful,
//// but WITHOUT ANY WARRANTY; without even the implied warranty of
//// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//// GNU General Public License for more details.
////
//// You should have received a copy of the GNU General Public License
//// along with this program; if not, see <http://www.gnu.org/licenses/>.
////###########################################################################

#include "craydl.h"
#include "RayonixHsCamera.h"

using namespace lima;
using namespace lima::RayonixHs;

//---------------------------
// @brief  Ctor
//---------------------------
Camera::Camera()
	: m_rx_detector(new craydl::RxDetector("./RxDetector.conf")),
	  m_status(DETECTOR_STATUS_IDLE),
	  m_acquiring(false),
	  m_cooler(true),
	  m_vac_valve(false)
{
	DEB_CONSTRUCTOR();

	init();

	m_frame_status_cb = new FrameStatusCb(this, m_acquiring);
	//m_frame_status_cb->registerCallbackAcqComplete(&lima::RayonixHs::Camera::acquisitionComplete);
	//m_frame_status_cb->registerCallbackAcqComplete(&acquisitionComplete);
}

//---------------------------
// @brief  Initialize method 
//---------------------------
void Camera::init() {
	DEB_MEMBER_FUNCT();
	craydl::RxReturnStatus status;
	
	setStatus(DETECTOR_STATUS_FAULT, true);
	//Open camera
	status = m_rx_detector->Open();
	if (status.IsError()) {
		m_max_image_size = Size(0, 0);
		THROW_HW_ERROR(Error) << "Fatal error opening camera";
	}
	std::string model, serial, version, junk;
	if (m_rx_detector->GetDetectorID(model, serial).IsError())
		THROW_HW_ERROR(Error) << "Cannot get camera ID";
	if (m_rx_detector->GetDetectorFirmwareID(junk, version).IsError())
		THROW_HW_ERROR(Error) << "Cannot get camera firmware ID";
		
	DEB_ALWAYS() << "Found RayonixHs model " << model
		     << "  - serial   # " << serial
		     << "  - firmware # " << version;

	//Get unbinned frame size from camera
	craydl::DetectorFormat detector_format;
	if (m_rx_detector->GetDetectorFormat(detector_format).IsError()) {			  
		THROW_HW_ERROR(Error) << "Cannot get camera format";
	}
	int fast, slow;
	fast = detector_format.n_pixels_fast();
	slow = detector_format.n_pixels_slow();

	m_max_image_size = Size(fast, slow);
	
	m_exp_time = 1.0;
	m_lat_time = 0.001;
	m_int_time = m_exp_time + m_lat_time;

	m_nb_frames = 1;
	// default frame mode is SINGLE, FAST_TRANFSER is not
	m_frame_mode = SINGLE;
	m_frame_trig_signal_type = OPTO;
	m_sequ_gate_signal_type = OPTO;

	setStatus(DETECTOR_STATUS_IDLE, true);
}

//---------------------------
// @brief  Dtor
//---------------------------
Camera::~Camera() {
	DEB_DESTRUCTOR();

	if (m_rx_detector->EndAcquisition(true).IsError())
		DEB_ERROR() << "Cannot end acquisition";

	if (m_rx_detector->Close().IsError())
		DEB_ERROR() << "Cannot close camera";

	delete m_rx_detector;
	delete m_frame_status_cb;
}

//---------------------------
// @brief  return the hw buffer
//---------------------------
HwBufferCtrlObj* Camera::getBufferCtrlObj() {
	DEB_MEMBER_FUNCT();
	return &m_buffer_ctrl_obj;
}

//---------------------------
// @brief  return the detector type
//---------------------------
void Camera::getDetectorType(std::string &type) {
	DEB_MEMBER_FUNCT();
	type = "RayonixHs";
}

//---------------------------
// @brief  return the current number of acquired (corrected) images
//---------------------------
int Camera::getNbAcquiredFrames() {
	DEB_MEMBER_FUNCT();
	return m_frame_status_cb->frameCountCorrected();
}

//---------------------------
// @brief  return the detector model
//---------------------------
void Camera::getDetectorModel(std::string &model) {
	DEB_MEMBER_FUNCT();
	std::string junk;
	if (m_rx_detector->GetDetectorID(model, junk).IsError())
		DEB_ERROR() << "Cannot read the detector model";
}

//---------------------------
// @brief  return the image type
//---------------------------
void Camera::getImageType(ImageType &img_type) {
	DEB_MEMBER_FUNCT();
	img_type = Bpp16;
}

//---------------------------
// @brief set the image type
//---------------------------
void Camera::setImageType(ImageType img_type) {
	DEB_MEMBER_FUNCT();
	switch (img_type) {
		case Bpp16:
			break;
		default:
		  THROW_HW_ERROR(InvalidValue) << "Image type is not supported: "
					       << DEB_VAR1(img_type);
	}
}


//---------------------------
// @brief  check if the trigger mode is valid
// valid modes depend on the frame mode
//---------------------------
bool Camera::checkTrigMode(TrigMode mode) {
	DEB_MEMBER_FUNCT();
	bool valid_mode;

	switch (mode) {
	case IntTrig:
	case IntTrigMult:
	case ExtTrigSingle:
	case ExtTrigMult:
	case ExtGate:
	case ExtTrigReadout:
		valid_mode = true;
		break;
	default:
		valid_mode = false;
	}
	return valid_mode;
}

//---------------------------
// @brief  set the frame mode SINGLE or FAST_TRANSFER
//---------------------------
void Camera::setFrameMode(FrameMode mode) {
	DEB_MEMBER_FUNCT();
  
	// RayonixHs is switched to FAST_TRANSFER frame mode by setting exposure time to 0 
	// then interval time sets the exposure duration, and latency is 1ms (transfer time)
	if (mode != m_frame_mode) {
		double exp_time;
		if (mode == SINGLE) exp_time = m_exp_time;
		else // FAST_TRANSFER
			exp_time = 0;
	  
		if (m_rx_detector->SetExposureTime(exp_time).IsError()) 
			THROW_HW_ERROR(Error) << "Failed  setting a new exposure time";
	  
	  
	}
	m_frame_mode = mode;
}

//---------------------------
// @brief  return the frame mode
//---------------------------
void Camera::getFrameMode(FrameMode &mode) {
	DEB_MEMBER_FUNCT();
	mode = m_frame_mode;
}

//---------------------------
// @brief  return the trigger mode
//---------------------------
void Camera::getTrigMode(TrigMode &mode) {
	DEB_MEMBER_FUNCT();
	mode = m_trig_mode;
}

//---------------------------
// @brief  set the trigger mode
//---------------------------
void Camera::setTrigMode(TrigMode mode) {
	DEB_MEMBER_FUNCT();
	m_trig_mode = mode;
}
//---------------------------
// @brief  return the pixel size in meter x and y
//---------------------------
void Camera::getPixelSize(double &x, double &y) {
	DEB_MEMBER_FUNCT();
	m_rx_detector->GetPixelSize(x, y);
}

//---------------------------
// @brief return the maximum image size
//---------------------------
void Camera::getMaxImageSize(Size& max_image_size) {
	DEB_MEMBER_FUNCT();
	max_image_size = m_max_image_size;
}

//---------------------------
// @brief  set the number of frames for acquistion
//---------------------------
void Camera::setNbFrames(int nb_frames) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(nb_frames);
	if (nb_frames < 0)
		THROW_HW_ERROR(InvalidValue) << "Invalid nb of frames";

	m_nb_frames = nb_frames;
}

//---------------------------
// @brief  return the number of frames for acquisition
//---------------------------
void Camera::getNbFrames(int& nb_frames) {
	DEB_MEMBER_FUNCT();
	nb_frames = m_nb_frames;
}

//---------------------------
// @brief  set the new exposure time in second
//---------------------------
void Camera::setExpTime(double exp_time) {
	DEB_MEMBER_FUNCT();
	if (exp_time < 0)
		THROW_HW_ERROR(InvalidValue) << "Invalid exposure time";
	if (m_frame_mode == SINGLE) {
		if (m_rx_detector->SetExposureTime(exp_time).IsError())
			THROW_HW_FATAL(Error) << "Failed setting exposure!";
		m_exp_time = exp_time;
	}
	else { // FAST_TRANSFER
		if (m_rx_detector->SetIntervalTime(exp_time).IsError())
			THROW_HW_FATAL(Error) << "Failed setting interval time for exposure in FAST_TRANSFER mode";
		m_int_time = exp_time;
	}
}

//---------------------------
// @brief  returnt the exposure time in second
//---------------------------
void Camera::getExpTime(double& exp_time) {
	DEB_MEMBER_FUNCT();

	if (m_frame_mode == SINGLE) {
		m_exp_time = m_rx_detector->ExposureTime();
		exp_time = m_exp_time;
	}
	else { // FAST_TRANSFER
		m_int_time = m_rx_detector->IntervalTime();
		exp_time = m_int_time;
	}
}

//---------------------------
// @brief  set the latency (interval) time in second
//---------------------------
void Camera::setLatTime(double lat_time) {
	DEB_MEMBER_FUNCT();

	if (lat_time < 0)
		THROW_HW_ERROR(InvalidValue) << "Invalid latency time";

	// Ok, RayonixHs manages an interval time not a latency:
	// in SINGLE frame mode latency = interval - exposure
	// in FAST_TRANSFER latency = 1ms and interval is the exposure (exposure must be 0 )
	if (m_frame_mode == SINGLE){
		if (m_rx_detector->SetIntervalTime(lat_time+m_exp_time).IsError())
			THROW_HW_FATAL(Error) << "Cannot set latency time";
		
	}
	else {
		// FAST_TRANSFER: latency is fixed, 1ms of frame transfer
		DEB_WARNING() << "Cannot set latency in FAST_TRANSFER frame mode: fixed to 1ms!";
	}
	m_lat_time = lat_time;
}

//---------------------------
// @brief  return the latency (interval) time in second
//---------------------------
void Camera::getLatTime(double& lat_time) {
	DEB_MEMBER_FUNCT();

	if (m_frame_mode == SINGLE){
		lat_time = m_lat_time;
	}
	else { // FAST_TRANSFER: latency is fixed, 1ms of frame transfer
		lat_time = 1e-3;
	}
	//m_lat_time = m_rx_detector->IntervalTime();
	lat_time = m_lat_time;
}

//---------------------------
// @brief  set the binning factors
//---------------------------
void Camera::setBin(const Bin& bin) {
	DEB_MEMBER_FUNCT();

	if (m_rx_detector->SetBinning(bin.getX(), bin.getY()).IsError())
		THROW_HW_ERROR(Error) << "Cannot set a new binning!";
}

//---------------------------
// @brief  return the binning factors
//---------------------------
void Camera::getBin(Bin& bin) {
	DEB_MEMBER_FUNCT();

	int binFast, binSlow;
	if (m_rx_detector->GetBinning(binFast, binSlow).IsError()) {
		THROW_HW_ERROR(Error) << "Cannot read binning!";
		return;
	}
	bin = Bin(binFast, binSlow);
}

//---------------------------
// @brief  check if the binning factors are valid
// if invalid return the current binning factors
//---------------------------
void Camera::checkBin(Bin& bin) {
	DEB_MEMBER_FUNCT();

	if (!m_rx_detector->CheckBinning(bin.getX(), bin.getY())) {
		DEB_TRACE() << "Invalid binning.  Setting bin to current detector binning.";
		getBin(bin);
	}
}

//---------------------------
// @brief  do nothing, not supported
//---------------------------
void Camera::setFrameDim(const FrameDim& frame_dim) {
	DEB_MEMBER_FUNCT();

	//Rayonix detector library handles this automatically
}

//---------------------------
// @brief  return the frame dimension (size x,y and pixel depth and sign)
//---------------------------
void Camera::getFrameDim(FrameDim& frame_dim) {
	DEB_MEMBER_FUNCT();

	int fast, slow, depth;
	m_rx_detector->GetFrameSize(fast, slow, depth);
	frame_dim.setSize(Size(fast, slow));
	frame_dim.setImageType(Bpp16);
}

//---------------------------
// @brief  stop the running acq. and reinitialize the detector 
//---------------------------
void Camera::reset() {
	DEB_MEMBER_FUNCT();

	stopAcq();
	init();
}

//---------------------------
// @brief  return detector acquisition status 
// from IDLE to ACQUISITION or FAULT
//---------------------------
void Camera::getStatus(DetectorStatus &status) {
	DEB_MEMBER_FUNCT();

	status = m_status;
}

//---------------------------
// @brief  private: set the new acquisition status
//---------------------------
void Camera::setStatus(DetectorStatus status,bool force)
{
	DEB_MEMBER_FUNCT();

	if(force || m_status != DETECTOR_STATUS_FAULT)
		m_status = status;
}

//---------------------------
// @brief  prepare the new acquisition
//---------------------------
void Camera::prepareAcq() {
	DEB_MEMBER_FUNCT();

	if (m_rx_detector->SetAcquisitionUserCB(static_cast<craydl::VirtualFrameCallback *> (m_frame_status_cb)).IsError()) {
		THROW_HW_ERROR(Error) << "SetAcquisitionUserCB() failed";
	}
	// 0 means first frame id equal 0 (InternalFrameID)
	if (m_rx_detector->SetupAcquisitionSequence(m_nb_frames, 0).IsError()) 
		THROW_HW_ERROR(Error) << "SetupAcquisitionSequence() failed!";

	craydl::FrameTriggerType_t rx_frame_trig_type;
	craydl::DigitalIOSignalType_t rx_trig_signal_type;
	craydl::SequenceGate_t rx_sequence_gate;
	int trigger;

	switch (m_trig_mode) {
	case IntTrig:
		rx_frame_trig_type	 = craydl::FrameTriggerTypeNone;
		rx_sequence_gate	 = craydl::SequenceGateModeNone;
		rx_trig_signal_type = craydl::DigitalIOSignalTypeNone;
		trigger = 0;
		break;
	case IntTrigMult:
		rx_frame_trig_type	 = craydl::FrameTriggerTypeFrame;
		rx_sequence_gate	 = craydl::SequenceGateModeNone;
		rx_trig_signal_type = craydl::DigitalIOSignalTypeSoftware;
		trigger = 0;
		break;

	case ExtTrigSingle:
		rx_frame_trig_type	 = craydl::FrameTriggerTypeNone;
		rx_sequence_gate	 = craydl::SequenceGateModeStart;
		rx_trig_signal_type = (craydl::DigitalIOSignalType_t) m_sequ_gate_signal_type;
		trigger = 1; // the Gate input
		break;

	case ExtTrigMult:
		rx_frame_trig_type	 = craydl::FrameTriggerTypeFrame;
		rx_sequence_gate	 = craydl::SequenceGateModeNone;
		rx_trig_signal_type = (craydl::DigitalIOSignalType_t) m_frame_trig_signal_type;
		trigger = 0; // Frame input
		break;

	case ExtGate:
		rx_frame_trig_type	 = craydl::FrameTriggerTypeBulb;
		rx_sequence_gate	 = craydl::SequenceGateModeNone;
		rx_trig_signal_type = (craydl::DigitalIOSignalType_t) m_frame_trig_signal_type;
		trigger = 0; // Frame input
		break;

	case ExtTrigReadout:
		// TODO: only in FAST_TRANSFER frame mode
		break;

	}
	// apply trigger mode (soft, trig, gate ...)
	m_rx_detector->SetFrameTriggerMode(craydl::FrameTriggerType(rx_frame_trig_type));
	m_rx_detector->SetSequenceGate(craydl::SequenceGateMode(rx_sequence_gate));
	// then set trig signal type and input (trig is input 0 and gate is input 1) 
	if (trigger==1) {
		if (m_rx_detector->SetSequenceGateSignalType(craydl::DigitalIOSignalType(rx_trig_signal_type)).IsError())
			THROW_HW_FATAL(Error) << "SetSequenceGateSignalType() failed";
	}
	else {
		if (m_rx_detector->SetFrameTriggerSignalType(craydl::DigitalIOSignalType(rx_trig_signal_type)).IsError())
			THROW_HW_FATAL(Error) << "SetFrameTriggerSignalType() failed";
	}


	m_expected_frame_nb = 0;
	// acq will be started in the first call of startAcq()
	if (m_trig_mode == IntTrigMult) m_int_trig_mult_started = false;

	// reset the callbacks frame counters
	m_frame_status_cb->resetFrameCounts();
}

//---------------------------
// @brief  start the new acquisition
//---------------------------
void Camera::startAcq() {
	DEB_MEMBER_FUNCT();

	m_buffer_ctrl_obj.getBuffer().setStartTimestamp(Timestamp::now());

	if (m_trig_mode == IntTrigMult && !m_int_trig_mult_started) {
		if (m_rx_detector->StartAcquisition(craydl::FrameAcquisitionTypeLight).IsError())
			THROW_HW_ERROR(Error) << "Cannot start the acquisition in IntTrigMult mode";
		m_int_trig_mult_started = true;
	}
	if (m_trig_mode == IntTrigMult) {
		if (m_rx_detector->PulseBulb(0.001).IsError())
			THROW_HW_ERROR(Error) << "PulsBulb() failed!";
	}
	else {
		if (m_rx_detector->StartAcquisition(craydl::FrameAcquisitionTypeLight).IsError())
			THROW_HW_ERROR(Error) << "StartAcquisition() failed!";
	}     
	setStatus(DETECTOR_STATUS_INTEGRATING);
}

//---------------------------
// @brief  stop the acquisition
//---------------------------
void Camera::stopAcq() {
	DEB_MEMBER_FUNCT();
	if (m_rx_detector->EndAcquisition(true).IsError())
		DEB_ERROR() << "Camera::stopAcq: Error stopping acquisition!";
	setStatus(DETECTOR_STATUS_IDLE);
}



//---------------------------
// @brief  callback function for each new frame
// called by FrameStatusCB.FrameReady()
//---------------------------
void Camera::frameReady(const craydl::RxFrame *pFrame) {
  
	DEB_MEMBER_FUNCT();
	int frameID = pFrame->InternalFrameID();
	
	StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_obj.getBuffer();
	void *ptr = buffer_mgr.getFrameBufferPtr(frameID);
	FrameDim frame_dim(pFrame->getNFast(), pFrame->getNSlow(), Bpp16);
	memcpy(ptr,pFrame->getBufferAddress(),frame_dim.getMemSize());
	
   
	HwFrameInfoType frame_info;
	frame_info.acq_frame_nb = frameID;
	DEB_TRACE() << "got frame " << DEB_VAR1(frameID);

	AutoMutex aLock(m_mutex);
	if(m_expected_frame_nb == frameID){
		bool continueAcq = buffer_mgr.newFrameReady(frame_info);
		++m_expected_frame_nb;
		
		for(std::map<int,HwFrameInfoType>::iterator i = m_pending_frames.begin();
		    continueAcq && i != m_pending_frames.end();m_pending_frames.erase(i++)){
			if(i->first == m_expected_frame_nb) {
				++m_expected_frame_nb;
				continueAcq = buffer_mgr.newFrameReady(i->second);
			}
			else
				break;
		}
	}
	else
		m_pending_frames[frameID] = frame_info;
   
}

//---------------------------
// @brief  return the frame trigger (input1) signal type
//---------------------------
void Camera::getFrameTriggerSignalType(SignalType & signal_type) {
	DEB_MEMBER_FUNCT();
	signal_type = m_frame_trig_signal_type;
}

//---------------------------
// @brief  set the frame trigger (input1) signal type
// @param  signal_type SOFT type is not allowed
//---------------------------
void Camera::setFrameTriggerSignalType(SignalType signal_type) {
	DEB_MEMBER_FUNCT();
	if (signal_type == SOFTWARE)
		THROW_HW_ERROR(InvalidValue) << "Signal Type SOFT is reserved"
					     << "Please use setTrigMode(IntTrigMult) instead";
	m_frame_trig_signal_type = signal_type;
}

//---------------------------
// @brief return the sequence gate (input2) signal type
//---------------------------
void Camera::getSequenceGateSignalType(SignalType & signal_type) {
	DEB_MEMBER_FUNCT();
	signal_type = m_sequ_gate_signal_type;
}

//---------------------------
// @brief  set the sequence gate (input2) signal type
// @param  signal_type SOFT type is not allowed
//---------------------------
void Camera::setSequenceGateSignalType(SignalType signal_type) {
	DEB_MEMBER_FUNCT();
	if (signal_type == SOFTWARE)
		THROW_HW_ERROR(InvalidValue) << "Signal Type SOFT is reserved"
					     << "Please use setTrigMode(IntTrigMult) instead";
	m_sequ_gate_signal_type = signal_type;
}

//---------------------------
// @brief set the output channel signal type
// @param[in] output CHANNEL_1 or CHANNEL_2
// @param[in] signal_type see SignalType enum
//---------------------------
void Camera::setOutputSignalType(OutputChannel output, SignalType signal_type) {
	DEB_MEMBER_FUNCT();

	craydl::DigitalIOSignalType_t sig_type = (craydl::DigitalIOSignalType_t)signal_type;

	if(m_rx_detector->SetDigitalIOSignalType((int) output, craydl::TriggerDirectionOutput, craydl::DigitalIOSignalType(sig_type)).IsError())
		THROW_HW_ERROR(Error) << "Cannot set output signal type";
}
//---------------------------
// @brief return the output signl type on selected channel
// @param[in] output CHANNEL_1 or CHANNEL_2
// @param[out] signal_type
//---------------------------
void Camera::getOutputSignalType(OutputChannel output, SignalType &signal_type) {
	DEB_MEMBER_FUNCT();

	craydl::DigitalIOSignalType_t sig_type;
	sig_type = m_rx_detector->GetDigitalIOSignalType((int)output, craydl::TriggerDirectionOutput).key();
	signal_type = (SignalType)sig_type;
}

//---------------------------
// @brief set the output ID on the selected channel
// @param[in] output CHANNEL_1 or CHANNEL_2
// @param[in] signal_id see SignalID enum for list
//---------------------------
void Camera::setOutputSignalID(OutputChannel output, SignalID signal_id) {
	DEB_MEMBER_FUNCT();

	craydl::DigitalIOSignalID_t sig_id = (craydl::DigitalIOSignalID_t)signal_id;

	if(m_rx_detector->SetDigitalIOSignalID((int) output, craydl::TriggerDirectionOutput, craydl::DigitalIOSignalID(sig_id)).IsError())
		THROW_HW_ERROR(Error) << "Cannot set output signal ID";
}
//---------------------------
// @brief return the output ID on selected channel
// @param[in] output CHANNEL_1 or CHANNEL_2
// @param[out] signal_id see SignalID enum for list
//---------------------------
void Camera::getOutputSignalID(OutputChannel output, SignalID &signal_id) {
	DEB_MEMBER_FUNCT();

	craydl::DigitalIOSignalID_t sig_id;
	sig_id = m_rx_detector->GetDigitalIOSignalID((int)output, craydl::TriggerDirectionOutput).key();
	signal_id = (SignalID)sig_id;
}

//---------------------------
// @brief set the shutter opening delay in seconds
//---------------------------
void Camera::setShutterOpenDelay(double delay) {
	DEB_MEMBER_FUNCT();

	if (m_rx_detector->SetShutterOpenDelay(delay).IsError())
		THROW_HW_ERROR(Error) << "Cannot set shutter open delay";
}

//---------------------------
// @brief return the shutter opening delay in seconds
//---------------------------
void Camera::getShutterOpenDelay(double &delay) {
	DEB_MEMBER_FUNCT();

	delay = m_rx_detector->ShutterOpenDelay();
}

//---------------------------
// @brief set the shutter closing delay in seconds
//---------------------------
void Camera::setShutterCloseDelay(double delay) {
	DEB_MEMBER_FUNCT();

	if (m_rx_detector->SetShutterCloseDelay(delay).IsError())
		THROW_HW_ERROR(Error) << "Cannot set shutter close delay";
}

//---------------------------
// @brief return the shutter opening delay in seconds
//---------------------------
void Camera::getShutterCloseDelay(double &delay) {
	DEB_MEMBER_FUNCT();

	delay = m_rx_detector->ShutterCloseDelay();
}

//---------------------------
// @brief set manually the shutter open or close
// @param[in] open true = open, false = close
//---------------------------
void Camera::setShutter(bool open) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(open);

	if (m_rx_detector->CommandShutter(open).IsError())
		THROW_HW_ERROR(Error) << "Cannot set manual shutter to :"
				      << DEB_VAR1(open);
}

//---------------------------
// @brief return the shutter status
// @param[out] open true or false
//---------------------------
void Camera::getShutter(bool &open) {
	DEB_MEMBER_FUNCT();

	open = m_rx_detector->Shutter();
}

//---------------------------
// @brief enable/disable the electronic shutter
// @param[in] enable true or false
//---------------------------
void Camera::setElectronicShutterEnabled(bool enable) {
	DEB_MEMBER_FUNCT();

	if (m_rx_detector->EnableElectronicShutter(enable).IsError())
		THROW_HW_ERROR(Error) << "Cannot enable electronic shutter";    
}

//---------------------------
// @brief return if electronic shutter is enable or disable
//---------------------------
void Camera::getElectronicShutterEnabled(bool &enable) {
	DEB_MEMBER_FUNCT();

	enable = m_rx_detector->ElectronicShutterEnabled();
}

//---------------------------
// @brief return the cooler temperature set point
// @param[out] temperature in degre Celcius
//---------------------------
void Camera::getCoolerTemperatureSetpoint(double &temperature) {
	DEB_MEMBER_FUNCT();

	temperature = m_rx_detector->CoolerTemperatureSetpoint();
}

//---------------------------
// @brief set the cooler temperature set point
// @param[in] temperature in degree Celcius
//---------------------------
void Camera::setCoolerTemperatureSetpoint(double temperature) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(temperature);

	if (m_rx_detector->SetCoolerTemperatureSetpoint(temperature).IsError())
		THROW_HW_ERROR(Error) << "Cannot set cooler temperature setpoint!";
}

//---------------------------
// @brief return the sensor temperature set point 
// @param[out] temperature in degree Celcius
//---------------------------
void Camera::getSensorTemperatureSetpoint(double &temperature) {
	DEB_MEMBER_FUNCT();
	
	temperature = m_rx_detector->SensorTemperatureSetpoint();
} 

//---------------------------
// @brief set the sensor temperature set point 
// @param[in] temperature in degree Celcius
//---------------------------
void Camera::setSensorTemperatureSetpoint(double temperature) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(temperature);

	if (m_rx_detector->SetSensorTemperatureSetpoint(temperature).IsError())
		THROW_HW_ERROR(Error) << "Cannot set sensor temperature setpoint!";	
}

//---------------------------
// @brief return the sensor temperature (average) 
// @param[out] temperature in degree Celcius
//---------------------------
void Camera::getSensorTemperature(double &temperature) {
	DEB_MEMBER_FUNCT();
	
	temperature = m_rx_detector->SensorTemperatureAve();
}


//---------------------------
// @brief start or stop the cooling system
// @param[in] start true = start, false = stop
//---------------------------
void Camera::setCooler(bool start) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(start);

	if (m_rx_detector->CommandCooler(start).IsError())
		THROW_HW_ERROR(Error) << "Cannot set cooler to : "
				      << DEB_VAR1(start);
	m_cooler = start;
}

//---------------------------
// @brief return if the cooling system is running or not
// @param[out] started true = started, false = stopped
//---------------------------
void Camera::getCooler(bool& started)
{
	DEB_MEMBER_FUNCT();

	m_cooler = m_rx_detector->SupportedStatusFlagValue(craydl::StatusFlagCoolerEnabled);	
	started = m_cooler;

}

//---------------------------
// @brief close or open the vacuum valve
// @param[in] open true = open, false = close
//---------------------------
void Camera::setVacuumValve(bool open) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(open);

	if (m_rx_detector->CommandVacuumValve(open).IsError())
		THROW_HW_ERROR(Error) << "Cannot set vacuum valve to : "
				      << DEB_VAR1(open);
	m_vac_valve = open;
}

//---------------------------
// @brief close or open the vacuum valve
// @param[in] opened true = open, false = close
//---------------------------
void Camera::getVacuumValve(bool& opened)
{
	DEB_MEMBER_FUNCT();

	m_vac_valve = m_rx_detector->SupportedStatusFlagValue(craydl::StatusFlagVacuumValveEnabled);
	opened = m_vac_valve;
}

//---------------------------
// @brief return if the detector needs a new background
// @param[out] needed: true = yes, false = no
//---------------------------
void Camera::getNewBackgroundNeeded(bool& needed)
{
	DEB_MEMBER_FUNCT();

	needed  = m_rx_detector->NewBackgroundNeeded();
}


//---------------------------
// @brief return if the detector needs a new background
// @param[in] block: true = blocking cmd, false = none-blocking cmd
//---------------------------
void Camera::acquireNewBackground(bool block, int n_background)
{
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR2(block,n_background);

	if (m_rx_detector->AcquireNewBackground(block,n_background).IsError())
		THROW_HW_ERROR(Error) << "Cannot take a new background :"
				      << DEB_VAR2(block, n_background);	
}
