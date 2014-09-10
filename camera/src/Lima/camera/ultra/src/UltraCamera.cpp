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
//
// UltraCamera.cpp
// Created on: Sep 13, 2013
// Author: g.r.mant

#include <sstream>
#include <iostream>
#include <string>
#include <math.h>
#include <climits>
#include <iomanip>
#include "UltraCamera.h"
#include "Exceptions.h"
#include "Debug.h"

using namespace lima;
using namespace lima::Ultra;
using namespace std;

//---------------------------
//- utility thread
//---------------------------
class Camera::AcqThread: public Thread {
DEB_CLASS_NAMESPC(DebModCamera, "Camera", "AcqThread");
public:
	AcqThread(Camera &aCam);
	virtual ~AcqThread();

protected:
	virtual void threadFunction();

private:
	Camera& m_cam;
};

//---------------------------
// @brief  Ctor
//---------------------------

Camera::Camera(std::string headname, std::string hostname, int tcpPort, int udpPort, int npixels) : m_headname(headname),
		m_hostname(hostname), m_tcpPort(tcpPort), m_udpPort(udpPort), m_npixels(npixels), m_image_type(Bpp16),
		m_nb_frames(0), m_acq_frame_nb(-1), m_bufferCtrlObj() {
	DEB_CONSTRUCTOR();

	DebParams::setModuleFlags(DebParams::AllFlags);
	DebParams::setTypeFlags(DebParams::AllFlags);
	DebParams::setFormatFlags(DebParams::AllFlags);
	m_acq_thread = new AcqThread(*this);
	m_acq_thread->start();
	m_ultra = new UltraNet();
	init();
}

Camera::~Camera() {
	DEB_DESTRUCTOR();
	m_ultra->disconnectFromServer();
	delete m_ultra;
	delete m_acq_thread;
}

void Camera::init() {
	DEB_MEMBER_FUNCT();
	stringstream cmd1, cmd2, cmd3;

	DEB_TRACE() << "Ultra initialising the data port " << DEB_VAR2(m_hostname,m_udpPort);
	m_ultra->initServerDataPort(m_hostname, m_udpPort);
	DEB_TRACE() << "Ultra connecting to " << DEB_VAR2(m_headname, m_tcpPort);
	m_ultra->connectToServer(m_headname, m_tcpPort);
	string reply;
	m_ultra->sendWait("", reply);
	if (reply.compare("!Command Not Recognised\r\n") != 0) {
		THROW_HW_ERROR(Error) << "Camera::init(): Response is not \"!Command Not Recognised\"";
	}
	getHeadType(m_headType);
	DEB_TRACE() << "Ultra responded OK with " << DEB_VAR1(m_headType);
}

void Camera::reset() {
	DEB_MEMBER_FUNCT();
	m_ultra->disconnectFromServer();
	init();
}

void Camera::prepareAcq() {
	DEB_MEMBER_FUNCT();
}

void Camera::startAcq() {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	m_acq_frame_nb = 0;
	StdBufferCbMgr& buffer_mgr = m_bufferCtrlObj.getBuffer();
	buffer_mgr.setStartTimestamp(Timestamp::now());
	AutoMutex aLock(m_cond.mutex());
	m_wait_flag = false;
	m_quit = false;
	m_cond.broadcast();
	// Wait that Acq thread start if it's an external trigger
	while (m_trigger_mode == ExtTrigMult && !m_thread_running)
		m_cond.wait();
}

void Camera::stopAcq() {
	DEB_MEMBER_FUNCT();
	AutoMutex aLock(m_cond.mutex());
	m_wait_flag = true;
	while (m_thread_running)
		m_cond.wait();
}

void Camera::readFrame(void *bptr, int frame_nb) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	int num;
	DEB_TRACE() << "Camera::readFrame() " << DEB_VAR1(frame_nb);
	if (m_image_type == Bpp16) {
		num = m_npixels * sizeof(short);
	} else {
		THROW_HW_ERROR(Error) << "Camera::readFrame(): Unsupported image type";
	}
	m_ultra->getData(bptr, num);
}

int Camera::getNbHwAcquiredFrames() {
	DEB_MEMBER_FUNCT();
	return m_acq_frame_nb;
}

void Camera::AcqThread::threadFunction() {
	DEB_MEMBER_FUNCT();
	AutoMutex aLock(m_cam.m_cond.mutex());
	StdBufferCbMgr& buffer_mgr = m_cam.m_bufferCtrlObj.getBuffer();

	while (!m_cam.m_quit) {
		while (m_cam.m_wait_flag && !m_cam.m_quit) {
			DEB_TRACE() << "Wait";
			m_cam.m_thread_running = false;
			m_cam.m_cond.broadcast();
			m_cam.m_cond.wait();
		}
		DEB_TRACE() << "AcqThread Running";
		m_cam.m_thread_running = true;
		if (m_cam.m_quit)
			return;

		m_cam.m_cond.broadcast();
		aLock.unlock();

		bool continueFlag = true;
		while (continueFlag && (!m_cam.m_nb_frames || m_cam.m_acq_frame_nb < m_cam.m_nb_frames)) {
				void* bptr = buffer_mgr.getFrameBufferPtr(m_cam.m_acq_frame_nb);
				m_cam.readFrame(bptr, m_cam.m_acq_frame_nb);
				HwFrameInfoType frame_info;
				frame_info.acq_frame_nb = m_cam.m_acq_frame_nb;
				continueFlag = buffer_mgr.newFrameReady(frame_info);
				DEB_TRACE() << "acqThread::threadFunction() newframe ready ";
				++m_cam.m_acq_frame_nb;
////			} else {
//				AutoMutex aLock(m_cam.m_cond.mutex());
//				continueFlag = !m_cam.m_wait_flag;
//				if (m_cam.m_wait_flag) {
//					stringstream cmd;
////					cmd << "xstrip timing stop ";
////					m_cam.m_ultra->sendWait(cmd.str());
//				} else {
//					usleep(1000);
//				}
//			}
			DEB_TRACE() << "acquired " << m_cam.m_acq_frame_nb << " frames, required " << m_cam.m_nb_frames << " frames";
		}
		aLock.lock();
		m_cam.m_wait_flag = true;
	}
}

Camera::AcqThread::AcqThread(Camera& cam) :
		m_cam(cam) {
	AutoMutex aLock(m_cam.m_cond.mutex());
	m_cam.m_wait_flag = true;
	m_cam.m_quit = false;
	aLock.unlock();
	pthread_attr_setscope(&m_thread_attr, PTHREAD_SCOPE_PROCESS);
}

Camera::AcqThread::~AcqThread() {
	AutoMutex aLock(m_cam.m_cond.mutex());
	m_cam.m_quit = true;
	m_cam.m_cond.broadcast();
	aLock.unlock();
}

void Camera::getImageType(ImageType& type) {
	DEB_MEMBER_FUNCT();
	type = m_image_type;
}

void Camera::setImageType(ImageType type) {
	DEB_MEMBER_FUNCT();
	m_image_type = type;
}

void Camera::getDetectorType(std::string& type) {
	DEB_MEMBER_FUNCT();
	type = "ultra";
}

void Camera::getDetectorModel(std::string& model) {
	DEB_MEMBER_FUNCT();
	if (m_headType == lima::Ultra::SILICON)
		model = "Silicon";
	else if (m_headType == lima::Ultra::INGAAS)
		model = "INGAAS";
	else
		model = "MCT";
}

void Camera::getDetectorImageSize(Size& size) {
	DEB_MEMBER_FUNCT();
	size = Size(m_npixels, 1);
}

void Camera::getPixelSize(double& sizex, double& sizey) {
	DEB_MEMBER_FUNCT();
	sizex = xPixelSize;
	sizey = yPixelSize;
}

HwBufferCtrlObj* Camera::getBufferCtrlObj() {
	return &m_bufferCtrlObj;
}

void Camera::setTrigMode(TrigMode mode) {
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setTrigMode() " << DEB_VAR1(mode);
	DEB_PARAM() << DEB_VAR1(mode);
	switch (mode) {
	case IntTrig:
	case IntTrigMult:
	case ExtTrigMult:
		m_trigger_mode = mode;
		break;
	case ExtTrigSingle:
	case ExtGate:
	case ExtStartStop:
	case ExtTrigReadout:
	default:
		THROW_HW_ERROR(Error) << "Cannot change the Trigger Mode of the camera, this mode is not managed !";
		break;
	}
}

void Camera::getTrigMode(TrigMode& mode) {
	DEB_MEMBER_FUNCT();
	mode = m_trigger_mode;
	DEB_RETURN() << DEB_VAR1(mode);
}

void Camera::getExpTime(double& exp_time) {
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::getExpTime()";
	exp_time = m_exp_time;
	DEB_RETURN() << DEB_VAR1(exp_time);
}

void Camera::setExpTime(double exp_time) {
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setExpTime() " << DEB_VAR1(exp_time);

	m_exp_time = exp_time;
}

void Camera::setLatTime(double lat_time) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(lat_time);

	if (lat_time != 0.)
		THROW_HW_ERROR(Error) << "Latency not managed";
}

void Camera::getLatTime(double& lat_time) {
	DEB_MEMBER_FUNCT();
	lat_time = 0;
}

void Camera::getExposureTimeRange(double& min_expo, double& max_expo) const {
	DEB_MEMBER_FUNCT();
	min_expo = 0.;
	max_expo = (double)UINT_MAX * 20e-9; //32bits x 20 ns
	DEB_RETURN() << DEB_VAR2(min_expo, max_expo);
}

void Camera::getLatTimeRange(double& min_lat, double& max_lat) const {
	DEB_MEMBER_FUNCT();
	// --- no info on min latency
	min_lat = 0.;
	// --- do not know how to get the max_lat, fix it as the max exposure time
	max_lat = (double) UINT_MAX * 20e-9;
	DEB_RETURN() << DEB_VAR2(min_lat, max_lat);
}

void Camera::setNbFrames(int nb_frames) {
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setNbFrames() " << DEB_VAR1(nb_frames);
	if (m_nb_frames < 0) {
		THROW_HW_ERROR(Error) << "Number of frames to acquire has not been set";
	}
	m_nb_frames = nb_frames;
}

void Camera::getNbFrames(int& nb_frames) {
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::getNbFrames";
	DEB_RETURN() << DEB_VAR1(m_nb_frames);
	nb_frames = m_nb_frames;
}

bool Camera::isAcqRunning() const {
	AutoMutex aLock(m_cond.mutex());
	return m_thread_running;
}

/////////////////////////
// ultra specific stuff now
/////////////////////////

void Camera::getHeadColdTemp(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("coldtemp", value);
}

void Camera::getHeadHotTemp(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("hottemp", value);
}

void Camera::getTecColdTemp(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("tectemp", value);
}

void Camera::getTecSupplyVolts(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("tecsup", value);
}

void Camera::getAdcPosSupplyVolts(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("psupvadc", value);
}

void Camera::getAdcNegSupplyVolts(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("psunvadc", value);
}

void Camera::getVinPosSupplyVolts(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("psupvin", value);
}

void Camera::getVinNegSupplyVolts(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("psunvin", value);
}

void Camera::getHeadADCVdd(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("headvccadc", value);
}

void Camera::getHeadVdd(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("headvcc", value);
}

void Camera::setHeadVdd(float voltage) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "headvcc " << voltage << "V";
	setValue(cmd.str());
}

void Camera::getHeadVref(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("headvref", value);
}

void Camera::setHeadVref(float voltage) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "headvref " << voltage << "V";
	setValue(cmd.str());
}

void Camera::getHeadVrefc(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("headvrefc", value);
}

void Camera::setHeadVrefc(float voltage) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "headvrefc " << voltage << "V";
	setValue(cmd.str());
}

void Camera::getHeadVpupref(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("headvpupref", value);
}

void Camera::setHeadVpupref(float voltage) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "headvpupref " << voltage << "V";
	setValue(cmd.str());
}

void Camera::getHeadVclamp(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("headvclamp", value);
}

void Camera::setHeadVclamp(float voltage) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "headvclamp " << voltage << "V";
	setValue(cmd.str());
}

void Camera::getHeadVres1(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("headvres1", value);
}

void Camera::setHeadVres1(float voltage) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "headvres1 " << voltage << "V";
	setValue(cmd.str());
}

void Camera::getHeadVres2(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("headvres2", value);
}

void Camera::setHeadVres2(float voltage) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "headvres2 " << voltage << "V";
	setValue(cmd.str());
}

void Camera::getHeadVTrip(float &value) {
	DEB_MEMBER_FUNCT();
	getValue("headtrip", value);
}

void Camera::setHeadVTrip(float voltage) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "headtrip " << voltage << "V";
	setValue(cmd.str());
}

void Camera::getFpgaXchipReg(unsigned int& reg) {
	DEB_MEMBER_FUNCT();
	getValue("fpgaxchip", reg);
}

void Camera::setFpgaXchipReg(unsigned int reg) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "fpgaxchip " << hex << reg << "";
	setValue(cmd.str());
}

void Camera::getFpgaPwrReg(unsigned int &reg) {
	DEB_MEMBER_FUNCT();
	getValue("fpgapwr", reg);
}

void Camera::setFpgaPwrReg(unsigned int reg) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "fpgapwr " << hex << reg;
	setValue(cmd.str());
}

void Camera::getFpgaSyncReg(unsigned int &reg) {
	DEB_MEMBER_FUNCT();
	getValue("fpgasync", reg);
}

void Camera::setFpgaSyncReg(unsigned int reg) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "fpgasync " << hex << reg;
	setValue(cmd.str());
}

void Camera::getFpgaAdcReg(unsigned int &reg) {
	DEB_MEMBER_FUNCT();
	getValue("fpgaadc", reg);
}

void Camera::setFpgaAdcReg(unsigned int reg) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "fpgaadc " << hex << reg << "";
	setValue(cmd.str());
}

void Camera::getFrameCount(unsigned int& reg) {
	DEB_MEMBER_FUNCT();
	getValue("fpgaframe", reg);
}

void Camera::getFrameErrorCount(unsigned int& reg) {
	DEB_MEMBER_FUNCT();
	getValue("fpgaerror", reg);
}

void Camera::getTecPowerEnabled(bool& state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaPwrReg(reg);
	state = (reg & TECPOWERMASK) ? true : false;
}

void Camera::setTecPowerEnabled(bool state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaPwrReg(reg);
	reg = (state) ? (reg |  TECPOWERMASK) : (reg & ~TECPOWERMASK);
	setFpgaPwrReg(reg);
}

void Camera::getHeadPowerEnabled(bool& state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaPwrReg(reg);
	state = (reg & HEADPOWERMASK) ? true : false;
}

void Camera::setHeadPowerEnabled(bool state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaPwrReg(reg);
	reg = (state) ? (reg | HEADPOWERMASK) : (reg & ~HEADPOWERMASK);
	setFpgaPwrReg(reg);
}

void Camera::getBiasEnabled(bool& state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaPwrReg(reg);
	state = (reg & BIASENABLEMASK) ? true: false;
}

void Camera::setBiasEnabled(bool state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaPwrReg(reg);
	reg =  (state) ? (reg | BIASENABLEMASK) : (reg & ~BIASENABLEMASK);
	setFpgaPwrReg(reg);
}

void Camera::getSyncEnabled(bool& state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaSyncReg(reg);
	state = (reg & SYNCENABLEMASK) ? true : false;
}

void Camera::setSyncEnabled(bool state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaSyncReg(reg);
	reg = (state) ? (reg | SYNCENABLEMASK) : (reg & ~SYNCENABLEMASK);
	setFpgaSyncReg(reg);
}

void Camera::getCalibEnabled(bool& state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaXchipReg(reg);
	state = (reg & CALENABLEMASK) ? true : false;
}

void Camera::setCalibEnabled(bool state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaXchipReg(reg);
	reg = (state) ? (reg | CALENABLEMASK) : (reg & ~CALENABLEMASK);
	setFpgaXchipReg(reg);
}

void Camera::get8pCEnabled(bool& state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaXchipReg(reg);
	state = (reg & EN8PCMASK) ? true : false;
}

void Camera::set8pCEnabled(bool state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaXchipReg(reg);
	reg = (state) ? (reg | EN8PCMASK) : (reg & ~EN8PCMASK);
}

void Camera::getTecOverTemp(bool& state) {
	DEB_MEMBER_FUNCT();
	unsigned int reg;
	getFpgaPwrReg(reg);
	state = (reg & TECOVERTEMPMASK) ? true : false;
}

void Camera::getAdcOffset(int channel, float &value) {
	DEB_MEMBER_FUNCT();
	int adcBoard, adcChannel;
	stringstream cmd;

	if (channel >= maxNumChannels) {
		THROW_HW_ERROR(Error) << "Invalid arguement channel value is outside of range";
	}
	adcChanLookup(m_headType, channel, adcBoard, adcChannel);
	cmd << "adc" << adcBoard << "off" << adcChannel;
	getValue(cmd.str(), value);
}

void Camera::setAdcOffset(int channel, float value) {
	DEB_MEMBER_FUNCT();
	int adcBoard, adcChannel;
	stringstream cmd;

	if (channel >= maxNumChannels) {
		THROW_HW_ERROR(Error) << "Invalid arguement channel value is outside of range";
	}
	adcChanLookup(m_headType, channel, adcBoard, adcChannel);
	cmd << "adc" << adcBoard << "off" << adcChannel << " " << value << "V";
	setValue(cmd.str());
}

void Camera::getAdcGain(int channel, float &value) {
	DEB_MEMBER_FUNCT();
	int adcBoard, adcChannel;
	stringstream cmd;

	if (channel >= maxNumChannels) {
		THROW_HW_ERROR(Error) << "Invalid arguement channel value is outside of range";
	}
	adcChanLookup(m_headType, channel, adcBoard, adcChannel);
	cmd << "adc" << adcBoard << "ref" << adcChannel;
	getValue(cmd.str(), value);
}

void Camera::setAdcGain(int channel, float value) {
	DEB_MEMBER_FUNCT();
	int adcBoard, adcChannel;
	stringstream cmd;

	if (channel >= maxNumChannels) {
		THROW_HW_ERROR(Error) << "Invalid arguement channel value is outside of range";
	}
	adcChanLookup(m_headType, channel, adcBoard, adcChannel);
	cmd << "adc" << adcBoard << "ref" << adcChannel << " " << value << "V";
	setValue(cmd.str());
}

void Camera::getAux1(unsigned int& delay, unsigned int& width) {
	DEB_MEMBER_FUNCT();
	getValue("fpgaaux1", delay, width);
}

void Camera::setAux1(unsigned int delay, unsigned int width) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "fpgaaux1 " << delay << " " << width;
	setValue(cmd.str());
}

void Camera::getAux2(unsigned int& delay, unsigned int& width) {
	DEB_MEMBER_FUNCT();
	return getValue("fpgaaux2", delay, width);
}

void Camera::setAux2(unsigned int delay, unsigned int width) {
	DEB_MEMBER_FUNCT();
	stringstream cmd;
	cmd << "fpgaaux2 " << delay << " " << width;
	setValue(cmd.str());
}

void Camera::getXchipTiming(unsigned int& delay, unsigned int& width, unsigned int& zeroWidth,
		unsigned int& sampleWidth, unsigned int& resetWidth, unsigned int& settlingTime, unsigned int& xClkHalfPeriod,
		unsigned int& readoutMode, unsigned int& shiftDelay) {
	DEB_MEMBER_FUNCT();
	unsigned int intS1Delay, intS2Delay, intDelay;
	unsigned int intS1Width, intS2Width;
	unsigned int intShiftWidth;

	getValue("fpgarst", intDelay, resetWidth);
	getValue("fpgas1", intS1Delay, intS1Width);
	getValue("fpgas2", intS2Delay, intS2Width);
	getValue("fpgaxclk", xClkHalfPeriod, settlingTime);
	getValue("fpgashift", shiftDelay, intShiftWidth);
	if (m_headType == lima::Ultra::INGAAS) {
		zeroWidth = intS2Width;
		delay = intDelay + intS2Width;
		sampleWidth = intS1Width;
		width = (intS1Width + intS1Delay) - (intS2Width + intS2Delay);
		readoutMode = 0;
	} else {
		zeroWidth = intS1Width;
		delay = intDelay + intS1Width;
		sampleWidth = intS2Width;
		width = (intS2Width + intS2Delay) - (intS1Width + intS1Delay);
		readoutMode =  (shiftDelay == (delay + width)) ? 1 : 0;
	}
}

void Camera::setXchipTiming(unsigned int delay, unsigned int width, unsigned int zeroWidth,
		unsigned int sampleWidth, unsigned int resetWidth, unsigned int settlingTime, unsigned int xClkHalfPeriod,
		unsigned int readoutMode) {
	DEB_MEMBER_FUNCT();
	unsigned int s1Delay, s2Delay, s1Width, s2Width, startDelay, shiftDelay;

	if (delay > zeroWidth)
		delay = delay - zeroWidth;
	else
		delay = 1;

	startDelay = delay + zeroWidth + width - sampleWidth;

	if (readoutMode == 1)
		shiftDelay = startDelay + sampleWidth;
	else
		shiftDelay = delay;

	if (m_headType == lima::Ultra::INGAAS) {
		s2Width = zeroWidth;
		s1Width = sampleWidth;
		s2Delay = delay;
		s1Delay = startDelay;
	} else {
		s1Width = zeroWidth;
		s2Width = sampleWidth;
		s1Delay = delay;
		s2Delay = startDelay;
	}

	// set reset width automatically
	//ResetWidth = ZeroWidth + width + 10;
	stringstream cmd1,cmd2,cmd3,cmd4,cmd5;
	cmd1 << "fpgarst " << delay << " " << resetWidth;
	setValue(cmd1.str());
	cmd2 << "fpgas1 " << s1Delay << " " << s1Width;
	setValue(cmd2.str());
	cmd3 << "fpgas2 " << s2Delay << " " << s2Width;
	setValue(cmd3.str());
	cmd4 << "fpgashift " << shiftDelay << " " << 1;
	setValue(cmd4.str());
	if (settlingTime > (xClkHalfPeriod - 2)) {
		settlingTime = xClkHalfPeriod - 2;
		cmd5 << "fpgaxclk " << xClkHalfPeriod << " " << settlingTime;
		setValue(cmd5.str());
	}
}

void Camera::saveConfiguration(void) {
	DEB_MEMBER_FUNCT();
	return setValue("state");
}

void Camera::restoreConfiguration(void) {
	DEB_MEMBER_FUNCT();
	return setValue("state");
}

void Camera::getHeadType(unsigned int& headType) {
	DEB_MEMBER_FUNCT();
	getValue("eeprom 0x1ff", headType);
}

void Camera::getValue(string cmd, float& value) {
	DEB_MEMBER_FUNCT();
	string reply;

	DEB_TRACE() << "Camera::getValue() sending command read " <<  cmd;
	string command = "read " + cmd;
	m_ultra->sendWait(command, reply);
	DEB_TRACE() << "Camera::getValue() got a reply " <<  reply;
	if ((reply[0] == '<') || (reply[0] == '>')) {
		if (sscanf(&reply[1], "%f", &value) == 1) {
			return;
		}
	} else {
		if (sscanf(reply.c_str(), "%f", &value) == 1) {
			DEB_TRACE() << "Camera::getValue() received float value " << value;
			return;
		}
	}
	THROW_HW_ERROR(Error) << "read failed";
}

void Camera::getValue(string cmd, unsigned int& value1, unsigned int& value2) {
	DEB_MEMBER_FUNCT();
	string reply;

	DEB_TRACE() << "Camera::getValue() sending command read " <<  cmd;
	string command = "read " + cmd;
	m_ultra->sendWait(command, reply);
	if (sscanf(reply.c_str(), "%u %u", &value1, &value2) != 2) {
		THROW_HW_ERROR(Error) << "Camera::getValue(): sscanf failed to read";
	}
}

void Camera::getValue(string cmd, unsigned int& value) {
	DEB_MEMBER_FUNCT();
	string reply;

	DEB_TRACE() << "Camera::getValue() sending command read " <<  cmd;
	string command = "read " + cmd;
	m_ultra->sendWait(command, reply);
//	stringstream ss(reply);
//	ss >> value;
//	if (ss.fail() == true) {
	if (sscanf(reply.c_str(), "%x", &value) != 1) {
		THROW_HW_ERROR(Error) << "Camera::getValue(): sscanf failed to read";
	}
}

void Camera::setValue(string cmd) {
	DEB_MEMBER_FUNCT();
	string reply;

	DEB_TRACE() << "Camera::setValue() sending command set " <<  cmd;
	string command = "set " + cmd;
	m_ultra->sendWait(command, reply);
	if (reply.compare("ACK\r\n") != 0) {
		THROW_HW_ERROR(Error) << "Camera::setValue(): bad acknowledgement";
	}
}

void Camera::adcChanLookup(int headType, int channel, int &adcBoard, int &adcChannel) {
	DEB_MEMBER_FUNCT();
	int SiBrdLookupArray[maxNumChannels] = { 1, 0, 1, 0, 1, 0, 1, 0, 2, 3, 2, 3, 2, 3, 2, 3 };
	int SiChanLookupArray[maxNumChannels] = { 3, 2, 0, 1, 1, 0, 2, 3, 3, 2, 0, 1, 1, 0, 2, 3 };

	if (channel >= maxNumChannels) {
		THROW_HW_ERROR(Error) << "Invalid arguement channel value is outside of range";
	}
	if (headType == lima::Ultra::MCT) {
		adcBoard = (channel / 4); //Temp need to correct when MCT commissioned
		adcChannel = (channel % 4);
	} else {
		adcBoard = SiBrdLookupArray[channel];
		adcChannel = SiChanLookupArray[channel];
	}
	return;
}
