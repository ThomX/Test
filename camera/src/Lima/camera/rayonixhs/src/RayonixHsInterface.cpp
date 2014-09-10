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
#include "RayonixHsInterface.h"
#include "RayonixHsCamera.h"
#include "RayonixHsDetInfoCtrlObj.h"
#include "RayonixHsBinCtrlObj.h"
#include "RayonixHsSyncCtrlObj.h"
#include "RayonixHsShutterCtrlObj.h"

using namespace lima;
using namespace lima::RayonixHs;

//-----------------------------------------------------
// @brief Ctor
//-----------------------------------------------------
Interface::Interface(Camera *cam)
	: m_cam(cam) {

	DEB_CONSTRUCTOR();

	m_det_info = new DetInfoCtrlObj(cam);
	m_buffer = cam->getBufferCtrlObj();
	m_sync = new SyncCtrlObj(cam);
	m_bin = new BinCtrlObj(cam);
	m_shutter = new ShutterCtrlObj(cam);
}

//-----------------------------------------------------
// @brief Dtor
//-----------------------------------------------------
Interface::~Interface() {
	DEB_DESTRUCTOR();

	delete m_det_info;
	delete m_sync;
	delete m_bin;
	delete m_shutter;
}

//-----------------------------------------------------
// @brief return the supported capability list
//-----------------------------------------------------
void Interface::getCapList(CapList &cap_list) const {
	cap_list.push_back(HwCap(m_sync));
	cap_list.push_back(HwCap(m_det_info));
	cap_list.push_back(HwCap(m_buffer));
	cap_list.push_back(HwCap(m_bin));
	cap_list.push_back(HwCap(m_shutter));
}

//-----------------------------------------------------
// @brief reset the interface, stop the acqisition
//-----------------------------------------------------
void Interface::reset(ResetLevel reset_level) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(reset_level);

	m_cam->stopAcq();
	if (reset_level == HardReset){
	  DEB_TRACE() << "Performing camera hard reset"; 
	  m_cam->reset();
	}
}

//-----------------------------------------------------
// @brief prepare the camera for next acquisition
//-----------------------------------------------------
void Interface::prepareAcq() {
	DEB_MEMBER_FUNCT();
	m_cam->prepareAcq();
}

//-----------------------------------------------------
// @brief start the camera acquisition
//-----------------------------------------------------
void Interface::startAcq() {
	DEB_MEMBER_FUNCT();
	m_cam->startAcq();
}

//-----------------------------------------------------
// @brief stop the camera acquisition
//-----------------------------------------------------
void Interface::stopAcq() {
	DEB_MEMBER_FUNCT();
	m_cam->stopAcq();
}

//-----------------------------------------------------
// @brief return the status of detector/acquisition
//-----------------------------------------------------
void Interface::getStatus(StatusType& status) {
        DEB_MEMBER_FUNCT();
	DetectorStatus det_status;
	
	m_cam->getStatus(det_status);

	switch (det_status) {
	case RayonixHs::DETECTOR_STATUS_INTEGRATING:
		status.acq = AcqRunning;
		status.det = DetExposure;
		break;
	case DETECTOR_STATUS_IDLE:
		status.acq = AcqReady;
		status.det = DetIdle;
		break;
	case DETECTOR_STATUS_READOUT:
		status.acq = AcqRunning;
		status.det = DetReadout;
		break;
	case DETECTOR_STATUS_LATENCY:
		status.det = DetLatency;
		break;
	case DETECTOR_STATUS_CONFIG:
		status.acq = AcqConfig;
		break;
	default:
		status.acq = AcqFault;
		status.det = DetFault;
		break;
	}
	DEB_RETURN() << DEB_VAR1(status);
}

//-----------------------------------------------------
// @brief return the hw number of acquired frames
//-----------------------------------------------------
int Interface::getNbAcquiredFrames() {
	DEB_MEMBER_FUNCT();

        int aNbAcquiredFrames = 0;
	aNbAcquiredFrames = m_cam->getNbAcquiredFrames();
	
	DEB_RETURN() << DEB_VAR1(aNbAcquiredFrames);
	return aNbAcquiredFrames;
}

int Interface::getNbHwAcquiredFrames() {
        DEB_MEMBER_FUNCT();    
	return getNbAcquiredFrames();
}

/////////////////////////////////////////////////////////////
// HERE we just a mapping of setter/getter methods between
//  Camera and Interface. Redondant but useful for client
// which does not want to export the Camera but only the Interface
// object.
/////////////////////////////////////////////////////////////

//---------------------------
// @brief  set the frame mode SINGLE or FAST_TRANSFER
//---------------------------
void Interface::setFrameMode(FrameMode mode){
        DEB_MEMBER_FUNCT();    
	m_cam->setFrameMode(mode);
}

//---------------------------
// @brief  return the frame mode
//---------------------------
void Interface::getFrameMode(FrameMode &mode){
        DEB_MEMBER_FUNCT();    
	m_cam->getFrameMode(mode);
}

//---------------------------
// @brief  set the frame trigger (input1) signal type
// @param  signal_type SOFT type is not allowed
//---------------------------
void Interface::setFrameTriggerSignalType(SignalType signal_type) {
        DEB_MEMBER_FUNCT();    
	m_cam->setFrameTriggerSignalType(signal_type);
}

//---------------------------
// @brief  return the frame trigger (input1) signal type
//---------------------------
void Interface::getFrameTriggerSignalType(SignalType &signal_type) {

        DEB_MEMBER_FUNCT();    
	m_cam->getFrameTriggerSignalType(signal_type);
}

//---------------------------
// @brief  set the sequence gate (input2) signal type
// @param  signal_type SOFT type is not allowed
//---------------------------
void Interface::setSequenceGateSignalType(SignalType signal_type) {
        DEB_MEMBER_FUNCT();    
	m_cam->setSequenceGateSignalType(signal_type);
}

//---------------------------
// @brief  return the sequence gate (input2) signal type
//---------------------------
void Interface::getSequenceGateSignalType(SignalType &signal_type) {
        DEB_MEMBER_FUNCT();    

	m_cam->getSequenceGateSignalType(signal_type);
}


//---------------------------
// @brief set the output channel signal type
// @param[in] output CHANNEL_1 or CHANNEL_2
// @param[in] signal_type see SignalType enum
//---------------------------
void Interface::setOutputSignalType(OutputChannel output, SignalType signal_type) {
	DEB_MEMBER_FUNCT();

	m_cam->setOutputSignalType(output, signal_type);
}
//---------------------------
// @brief return the output signl type on selected channel
// @param[in] output CHANNEL_1 or CHANNEL_2
// @param[out] signal_type
//---------------------------
void Interface::getOutputSignalType(OutputChannel output, SignalType &signal_type) {
	DEB_MEMBER_FUNCT();

	m_cam->getOutputSignalType(output, signal_type);
}

//---------------------------
// @brief set the output ID on the selected channel
// @param[in] output CHANNEL_1 or CHANNEL_2
// @param[in] signal_id see SignalID enum for list
//---------------------------
void Interface::setOutputSignalID(OutputChannel output, SignalID signal_id) {
	DEB_MEMBER_FUNCT();

	m_cam->setOutputSignalID(output, signal_id);
}
//---------------------------
// @brief return the output ID on selected channel
// @param[in] output CHANNEL_1 or CHANNEL_2
// @param[out] signal_id see SignalID enum for list
//---------------------------
void Interface::getOutputSignalID(OutputChannel output, SignalID &signal_id) {
	DEB_MEMBER_FUNCT();

	m_cam->getOutputSignalID(output, signal_id);
}

//---------------------------
// @brief enable/disable the electronic shutter
// @param[in] enable true or false
//---------------------------
void Interface::setElectronicShutterEnabled(bool enable) {
	DEB_MEMBER_FUNCT();

	m_cam->setElectronicShutterEnabled(enable);
}

//---------------------------
// @brief return if electronic shutter is enable or disable
//---------------------------
void Interface::getElectronicShutterEnabled(bool &enable) {
	DEB_MEMBER_FUNCT();

	m_cam->getElectronicShutterEnabled(enable);
}

//---------------------------
// @brief return the cooler temperature set point
// @param[out] temperature in degre Celcius
//---------------------------
void Interface::getCoolerTemperatureSetpoint(double &temperature) {
	DEB_MEMBER_FUNCT();

	m_cam->getCoolerTemperatureSetpoint(temperature);
}

//---------------------------
// @brief set the cooler temperature set point
// @param[in] temperature in degree Celcius
//---------------------------
void Interface::setCoolerTemperatureSetpoint(double temperature) {
	DEB_MEMBER_FUNCT();

	m_cam->setCoolerTemperatureSetpoint(temperature);
}

//---------------------------
// @brief return the sensor temperature set point 
// @param[out] temperature in degree Celcius
//---------------------------
void Interface::getSensorTemperatureSetpoint(double &temperature) {
	DEB_MEMBER_FUNCT();
	
	m_cam->getSensorTemperatureSetpoint(temperature);
} 

//---------------------------
// @brief return the sensor temperature
// @param[out] temperature in degree Celcius
//---------------------------
void Interface::getSensorTemperature(double &temperature) {
	DEB_MEMBER_FUNCT();
	
	m_cam->getSensorTemperature(temperature);
} 

//---------------------------
// @brief set the sensor temperature set point 
// @param[in] temperature in degree Celcius
//---------------------------
void Interface::setSensorTemperatureSetpoint(double temperature) {
	DEB_MEMBER_FUNCT();

	m_cam->setSensorTemperatureSetpoint(temperature);
}

//---------------------------
// @brief start or stop the cooling system
// @param[in] enable true = start, false = stop
//---------------------------
void Interface::setCooler(bool enable) {
	DEB_MEMBER_FUNCT();

	m_cam->setCooler(enable);
}

//---------------------------
// @brief return if the cooling system is running or not
// @param[out] started true = started, false = stopped
//---------------------------
void Interface::getCooler(bool &started) {
	DEB_MEMBER_FUNCT();
       
	m_cam->getCooler(started);

}

//---------------------------
// @brief close or open the vacuum valve
// @param[in] enable true = open, false = close
//---------------------------
void Interface::setVacuumValve(bool open) {
	DEB_MEMBER_FUNCT();

	m_cam->setVacuumValve(open);
}

//---------------------------
// @brief close or open the vacuum valve
// @param[in] opened true = open, false = close
//---------------------------
void Interface::getVacuumValve(bool& opened)
{
	DEB_MEMBER_FUNCT();

	m_cam->getVacuumValve(opened);
}
//---------------------------
// @brief return if the detector needs a new background
// @param[out] needed: true = yes, false = no
//---------------------------
void Interface::getNewBackgroundNeeded(bool& needed)
{
        DEB_MEMBER_FUNCT();

        m_cam->getNewBackgroundNeeded(needed);
}


//---------------------------
// @brief return if the detector needs a new background
// @param[in] block: true = blocking cmd, false = none-blocking cmd
//---------------------------
void Interface::acquireNewBackground(bool block, int n_background)
{
        DEB_MEMBER_FUNCT();

	m_cam->acquireNewBackground(block, n_background);
}


