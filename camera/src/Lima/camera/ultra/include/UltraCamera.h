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
/*
 * UltraCamera.h
 * Created on: Feb 04, 2013
 * Author: g.r.mant
 */
#ifndef ULTRACAMERA_H_
#define ULTRACAMERA_H_

#include "HwMaxImageSizeCallback.h"
#include "HwBufferMgr.h"
#include "HwInterface.h"
#include "UltraInterface.h"
#include <ostream>
#include "Debug.h"
#include "UltraNet.h"

using namespace std;

namespace lima {
namespace Ultra {

const int TECPOWERMASK = 0x01l;
const int HEADPOWERMASK = 0x02l;
#define BIASENABLEMASK 0x04l
#define SYNCENABLEMASK 0x80000000l
#define CALENABLEMASK 0x01l
#define EN8PCMASK 0x06l
#define TECOVERTEMPMASK 0x80000000l

//#define HEAD_SILICON 0
//#define HEAD_INGAAS 1
//#define HEAD_MCT 2

enum HEAD_TYPE {SILICON, INGAAS, MCT};

const int maxNumChannels = 16;

const int xPixelSize = 1;
const int yPixelSize = 1;

class BufferCtrlObj;

/*******************************************************************
 * \class Camera
 * \brief object controlling the Ultra camera
 *******************************************************************/
class Camera {
DEB_CLASS_NAMESPC(DebModCamera, "Camera", "Ultra");

public:

	Camera(std::string headname="192.168.1.100", std::string hostname="192.168.1.103",
			int tcpPort=7, int udpPort=5005, int npixels=512);
	~Camera();

	void init();
	void reset();
	void prepareAcq();
	void startAcq();
	void stopAcq();
//	void getStatus(Status& status);
	int getNbHwAcquiredFrames();

	// -- detector info object
	void getImageType(ImageType& type);
	void setImageType(ImageType type);
//
	void getDetectorType(std::string& type);
	void getDetectorModel(std::string& model);
	void getDetectorImageSize(Size& size);
	void getPixelSize(double& sizex, double& sizey);

	// -- Buffer control object
	HwBufferCtrlObj* getBufferCtrlObj();

	//-- Synch control object
	void setTrigMode(TrigMode mode);
	void getTrigMode(TrigMode& mode);

	void setExpTime(double exp_time);
	void getExpTime(double& exp_time);

	void setLatTime(double lat_time);
	void getLatTime(double& lat_time);

	void getExposureTimeRange(double& min_expo, double& max_expo) const;
	void getLatTimeRange(double& min_lat, double& max_lat) const;

	void setNbFrames(int nb_frames);
	void getNbFrames(int& nb_frames);

	bool isAcqRunning() const;

	///////////////////////////
	// -- ultra specific functions
	///////////////////////////

	void getHeadColdTemp(float &value);
	void getHeadHotTemp(float &value);
	void getTecColdTemp(float &value);
	void getTecSupplyVolts(float &value);
	void getAdcPosSupplyVolts(float &value);
	void getAdcNegSupplyVolts(float &value);
	void getVinPosSupplyVolts(float &value);
	void getVinNegSupplyVolts(float &value);
	void getHeadADCVdd(float &value);
	void getHeadVdd(float &value);
	void setHeadVdd(float voltage);
	void getHeadVref(float &value);
	void setHeadVref(float voltage);
	void getHeadVrefc(float &value);
	void setHeadVrefc(float voltage);
	void getHeadVpupref(float &value);
	void setHeadVpupref(float voltage);
	void getHeadVclamp(float &value);
	void setHeadVclamp(float voltage);
	void getHeadVres1(float &value);
	void setHeadVres1(float voltage);
	void getHeadVres2(float &value);
	void setHeadVres2(float voltage);
	void getHeadVTrip(float &value);
	void setHeadVTrip(float voltage);
	void getFpgaXchipReg(unsigned int& reg);
	void setFpgaXchipReg(unsigned int reg);
	void getFpgaPwrReg(unsigned int &reg);
	void setFpgaPwrReg(unsigned int reg);
	void getFpgaSyncReg(unsigned int &reg);
	void setFpgaSyncReg(unsigned int reg);
	void getFpgaAdcReg(unsigned int &reg);
	void setFpgaAdcReg(unsigned int reg);
	void getFrameCount(unsigned int& reg);
	void getFrameErrorCount(unsigned int& reg);
	void getHeadPowerEnabled(bool& state);
	void setHeadPowerEnabled(bool state);
	void getTecPowerEnabled(bool& state);
	void setTecPowerEnabled(bool state);
	void getBiasEnabled(bool& state);
	void setBiasEnabled(bool state);
	void getSyncEnabled(bool& state);
	void setSyncEnabled(bool state);
	void getCalibEnabled(bool& state);
	void setCalibEnabled(bool state);
	void get8pCEnabled(bool& state);
	void set8pCEnabled(bool state);
	void getTecOverTemp(bool& state);
	void saveConfiguration(void);
	void restoreConfiguration(void);
	void getAdcOffset(int channel, float &value);
	void setAdcOffset(int channel, float value);
	void getAdcGain(int channel, float &value);
	void setAdcGain(int channel, float value);
	void getAux1(unsigned int& delay, unsigned int& width);
	void setAux1(unsigned int delay, unsigned int width);
	void getAux2(unsigned int& delay, unsigned int& width);
	void setAux2(unsigned int delay, unsigned int width);
	void getXchipTiming(unsigned int& delay, unsigned int& width, unsigned int& zeroWidth,
			unsigned int& sampleWidth, unsigned int& resetWidth, unsigned int& settlingTime, unsigned int& xClkHalfPeriod,
			unsigned int& readoutMode, unsigned int& shiftDelay);
	void setXchipTiming(unsigned int delay, unsigned int width, unsigned int zeroWidth,
			unsigned int sampleWidth, unsigned int resetWidth, unsigned int settlingTime, unsigned int xClkHalfPeriod,
			unsigned int readoutMode);

private:
	// ultra specific
	UltraNet *m_ultra;
	string m_headname;
	string m_hostname;
	int m_tcpPort;
	int m_udpPort;
	unsigned int m_headType;

	int m_npixels;

	class AcqThread;

	AcqThread *m_acq_thread;
	TrigMode m_trigger_mode;
	double m_exp_time;
	ImageType m_image_type;
	int m_nb_frames; // nos of frames to acquire
	bool m_thread_running;
	bool m_wait_flag;
	bool m_quit;
	int m_acq_frame_nb; // nos of frames acquired
	mutable Cond m_cond;


	// Buffer control object
	SoftBufferCtrlObj m_bufferCtrlObj;

	void readFrame(void *bptr, int frame_nb);
	void getHeadType(unsigned int& headType);
	void getValue(string cmd, unsigned int& value);
	void getValue(string cmd, float& value);
	void getValue(string cmd, unsigned int& value1, unsigned int& value2);
	void setValue(string cmd);
	void adcChanLookup(int headType, int channel, int &adcBoard, int &adcChannel);
};

} // namespace Ultra
} // namespace lima

#endif /* ULTRACAMERA_H_ */
